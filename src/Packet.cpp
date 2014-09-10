#include <ptu_kongsberg_oe10/Packet.hpp>
#include <boost/lexical_cast.hpp>
#include <base/Logging.hpp>

using namespace std;
using namespace ptu_kongsberg_oe10;
using boost::lexical_cast;

Packet::Packet(byte to, byte from)
    : from(from)
    , to(to)
    , command_size(0)
    , data_size(0)
{}


void Packet::setCommand(byte c0)
{
    command_size = 1;
    command[0] = c0;
}

void Packet::setCommand(byte c0, byte c1)
{
    command_size = 2;
    command[0] = c0;
    command[1] = c1;
}

void Packet::marshal(vector<byte>& buffer) const
{
    int start = buffer.size();
    buffer.push_back('<');
    buffer.push_back(to);
    buffer.push_back(':');
    buffer.push_back(from);
    buffer.push_back(':');
    buffer.push_back(command_size + data_size + 1);
    buffer.push_back(':');
    for (int i = 0; i < command_size; ++i)
        buffer.push_back(command[i]);
    buffer.push_back(':');
    for (int i = 0; i < data_size; ++i)
        buffer.push_back(data[i]);
    byte checksum = computeChecksum(&buffer[start + 1], &buffer[buffer.size()]);
    buffer.push_back(':');
    buffer.push_back(checksum);
    buffer.push_back(':');
    byte checksumInd = computeChecksumInd(checksum);
    buffer.push_back(checksumInd);
    buffer.push_back('>');
}

float Packet::parseAngle(byte const* buffer)
{
    int scale[3] = { 100, 10, 1 };
    float angle = 0;
    for (int i = 0; i < 3; ++i)
    {
        char c = static_cast<char>(buffer[i]);
        if (c < '0' || c > '9')
            throw std::runtime_error("ASCII angle representation not in the 0-9 range");
        angle += (c - '0') * scale[i];
    }
    return angle * M_PI / 180;
}

byte Packet::computeChecksum(byte const* begin, byte const* end)
{
    byte result = 0;
    for (; begin != end; ++begin)
    {
        result = result ^ *begin;
    }
    return result;
}

byte Packet::computeChecksumInd(byte checksum)
{
    if (checksum == 0x3C)
        return '0';
    if (checksum == 0x3E)
        return '1';
    return 'G';
}

int Packet::extractPacket(byte const* buffer, int size)
{
    byte const BRACKET_OPEN = '<';
    byte const COLON = ':';
    byte const BRACKET_CLOSE = '>';

    if (size < 14)
        return 0;
    if (size && buffer[0] != BRACKET_OPEN)
        return -1;
    if (buffer[1] == 0)
        return -1;
    if (buffer[2] != COLON)
        return -1;
    if (buffer[3] == 0)
        return -1;
    if (buffer[4] != COLON)
        return -1;
    byte length = buffer[5];
    if (length >= 99)
        throw std::logic_error("I don't know how to handle packets whose length is 99 or more, the protocol spec is very unclear");
    if (buffer[6] != COLON)
        return -1;
    if (size < 12 + length)
        return 0;
    if (buffer[7 + length] != COLON)
        return -1;
    if (buffer[7 + length + 2] != COLON)
        return -1;
    if (buffer[7 + length + 4] != BRACKET_CLOSE)
        return -1;
    byte checksum    = buffer[7 + length + 1];
    if (checksum != computeChecksum(&buffer[1], &buffer[7 + length]))
    {
        LOG_DEBUG_S << "packet failed checksum test";
        return -1;
    }
    byte checksumInd = computeChecksumInd(checksum);
    if (checksumInd != buffer[7 + length + 3])
    {
        LOG_DEBUG_S << "packet failed checksumInd test";
        return -1;
    }

    return 12 + length;
}

Packet Packet::parse(byte const* buffer, int size, bool validate)
{
    if (validate)
    {
        if (extractPacket(buffer, size) <= 0)
            throw std::runtime_error("provided buffer does not start with a complete packet");
    }

    Packet result;
    result.to   = buffer[1];
    result.from = buffer[3];
    byte length = buffer[5];
    result.command[0] = buffer[7];
    if (buffer[8] == ':')
    {
        result.command_size = 1;
    }
    else
    {
        result.command[1] = buffer[8];
        result.command_size = 2;
    }
        
    result.data_size = length - result.command_size - 1;
    memcpy(result.data, &buffer[7 + result.command_size + 1], result.data_size);
    return result;
}

void Packet::validateResponseFor(Packet const& cmd)
{
    if (command_size != 1 || command[0] != ACK || command[0] != NAK)
    {
        throw std::runtime_error("expecting a ACK/NAK packet but got " +
                string(reinterpret_cast<char const*>(command), command_size));
    }

    if (from != cmd.to)
    {
        throw std::runtime_error("expected a response from device ID " +
                lexical_cast<string>(cmd.to) + " but got one from " +
                lexical_cast<string>(from));
    }

    if (data_size < cmd.command_size)
        throw std::runtime_error("got a ACK/NAK packet with a smaller-than expected data field");

    for (int i = 0; i < cmd.command_size; ++i)
    {
        if (data[i] != cmd.command[i])
            throw std::runtime_error("expected a ACK/NAK for command " +
                    cmd.getCommandAsString() + " but got it for " +
                    string(reinterpret_cast<char const*>(cmd.data), static_cast<string::size_type>(cmd.command_size)));
    }

    if (command[0] == NAK)
    {
        throw std::runtime_error("received NAK with the following error bits set: " +
                parseNACKError(data[0]));
    }
}

string Packet::getCommandAsString() const
{
    if (command_size == 1)
    {
        if (command[0] == ACK)
            return "ACK";
        else if (command[0] == NAK)
            return "NAK";
    }
    return string(reinterpret_cast<char const*>(command), static_cast<string::size_type>(command_size));
}

char const* ERROR_CODES[] =
{
    "device under control of another controller",
    "at focus end stop",
    "at zoom end stop",
    "command not available for this device",
    "command not recognized",
    "device timed out",
    "undefined",
    "undefined"
};

string Packet::parseNACKError(byte errorByte)
{
    string result;
    for (int i = 0; i < 8; ++i)
    {
        if (errorByte & (1 << i))
        {
            if (i != 0)
                result += ", ";
            result += ERROR_CODES[i];
        }
    }
    return result;
}

