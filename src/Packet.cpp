#include <ptu_kongsberg_oe10/Packet.hpp>
#include <boost/lexical_cast.hpp>
#include <base/Logging.hpp>
#include <iodrivers_base/Driver.hpp>

using namespace std;
using namespace ptu_kongsberg_oe10;
using boost::lexical_cast;

/**
 * Initialize a packet with specified destination and source IDs
 * Sets initial command and data sizes to zero
 */
Packet::Packet(byte to, byte from)
    : from(from)
    , to(to)
    , command_size(0)
    , data_size(0)
{}

/**
 * Set a single-byte command in the packet
 * Used for simple commands that don't require additional parameters
 */
void Packet::setCommand(byte c0)
{
    command_size = 1;
    command[0] = c0;
}

/**
 * Set a two-byte command in the packet
 * Used for more complex commands that require two command bytes
 */
void Packet::setCommand(byte c0, byte c1)
{
    command_size = 2;
    command[0] = c0;
    command[1] = c1;
}

/**
 * Serialize the packet into a byte buffer according to the protocol format:
 * <to:from:length:command:data:checksum>
 * Handles special character escaping and checksum calculation
 */
void Packet::marshal(vector<byte>& buffer) const
{
    int start = buffer.size();
    // Add packet start marker and header fields
    buffer.push_back('<');
    buffer.push_back(to);
    buffer.push_back(':');
    buffer.push_back(from);
    buffer.push_back(':');
    buffer.push_back(command_size + data_size + 1);  // +1 for checksum
    buffer.push_back(':');

    // Add command bytes
    for (int i = 0; i < command_size; ++i)
        buffer.push_back(command[i]);
    buffer.push_back(':');

    // Add data payload
    for (int i = 0; i < data_size; ++i)
        buffer.push_back(data[i]);

    // Calculate and add checksum
    byte checksum = computeChecksum(&buffer[start + 1], &buffer[buffer.size()]);
    buffer.push_back(':');

    // Marshal checksum with special character handling
    byte marshalledChecksum[3];
    Packet::marshalChecksum(checksum, marshalledChecksum);
    buffer.insert(buffer.end(), marshalledChecksum, marshalledChecksum + 3);
    buffer.push_back('>');
}

/**
 * Parse a 3-byte ASCII representation of an angle
 * Handles special cases where the PTU reports angles:
 * - '000' represents 0 degrees
 * - '999' also represents 0 degrees (PTU bug workaround)
 * @return Angle in radians
 */
float Packet::parseAngle(byte const* buffer)
{
    // Handle special cases for zero angle
    if (buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 0)
        return 0;
    if (buffer[0] == '9' && buffer[1] == '9' && buffer[2] == '9')
        return 0;

    // Validate that all bytes are ASCII digits
    for (int i = 0; i < 3; ++i)
    {
        char c = static_cast<char>(buffer[i]);
        if (c < '0' || c > '9')
            throw std::runtime_error("ASCII angle representation not in the 0-9 range (got " + lexical_cast<string>(static_cast<int>(c)) + ")");
    }

    // Convert ASCII digits to angle value and convert to radians
    float angle =
        (static_cast<char>(buffer[0]) - '0') * 100 +
        (static_cast<char>(buffer[1]) - '0') * 10 +
        (static_cast<char>(buffer[2]) - '0') * 1;
    return angle * M_PI / 180;
}

/**
 * Convert an angle to its 3-byte ASCII representation
 * Angles must be between 0 and 360 degrees
 * Each byte will contain an ASCII digit character
 */
void Packet::encodeAngle(byte* buffer, float angle)
{
    int degrees = angle * 180 / M_PI;
    if (degrees < 0 || degrees > 360)
        throw std::range_error("angles must be in [0, 360], got " + lexical_cast<string>(degrees));

    // Split into hundreds, tens, and units digits
    int hundreds = static_cast<int>(degrees / 100);
    int tens     = static_cast<int>(degrees / 10) % 10;
    int units    = static_cast<int>(degrees) % 10;
    buffer[0] = static_cast<byte>('0' + hundreds);
    buffer[1] = static_cast<byte>('0' + tens);
    buffer[2] = static_cast<byte>('0' + units);
}

/**
 * Calculate XOR checksum of a byte range
 * Used for packet validation
 */
byte Packet::computeChecksum(byte const* begin, byte const* end)
{
    byte result = 0;
    for (; begin != end; ++begin)
    {
        result = result ^ *begin;
    }
    return result;
}

/**
 * Create a human-readable representation of packet data
 * Preserves special characters for better debugging
 */
std::string Packet::kongsberg_com(boost::uint8_t const* buffer, int buffer_size)
{
    ostringstream str;
    for (int i = 0; i < buffer_size; ++i)
    {
        if (buffer[i] == '<')
            str << '<';
        else if (buffer[i] == ':')
            str << ':';
        else if (buffer[i] == '>')
            str << '>';
        else
            str << iodrivers_base::Driver::binary_com(buffer + i, 1);
    }
    return str.str();
}

/**
 * Extract a complete packet from a buffer of received data
 * Validates packet format and checksum
 * @return Packet size if valid, 0 if incomplete, -1 if invalid
 */
int Packet::extractPacket(byte const* buffer, int size)
{
    LOG_DEBUG_S << "parsing " << size << " bytes: " << kongsberg_com(buffer, size);

    // Protocol special characters
    byte const BRACKET_OPEN = '<';
    byte const COLON = ':';
    byte const BRACKET_CLOSE = '>';

    // Basic size and format validation
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

    // Validate packet checksum
    byte expectedChecksum = computeChecksum(&buffer[1], &buffer[7 + length]);
    if (!Packet::compareChecksum(expectedChecksum, &buffer[7 + length + 1]))
    {
        LOG_DEBUG_S << "packet failed checksum test";
        return -1;
    }

    return 12 + length;
}

/**
 * Parse a complete packet from a buffer into a Packet structure
 * Optionally validates the packet format first
 */
Packet Packet::parse(byte const* buffer, int size, bool validate)
{
    if (validate)
    {
        if (extractPacket(buffer, size) <= 0)
            throw std::runtime_error("provided buffer does not start with a complete packet");
    }

    Packet result;
    // Parse header fields
    result.to   = buffer[1];
    result.from = buffer[3];
    byte length = buffer[5];

    // Parse command (1 or 2 bytes)
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
        
    // Copy data payload
    result.data_size = length - result.command_size - 1;
    memcpy(result.data, &buffer[7 + result.command_size + 1], result.data_size);
    return result;
}

/**
 * Validate that this packet is a proper response to a command
 * Checks:
 * - Response is ACK or NAK
 * - Source device ID matches
 * - Command echo matches original command
 * Throws runtime_error if validation fails
 */
void Packet::validateResponseFor(Packet const& cmd)
{
    if (command_size != 1 || (command[0] != ACK && command[0] != NAK))
    {
        throw std::runtime_error("expecting a ACK/NAK packet but got " + iodrivers_base::Driver::binary_com(command, command_size));
    }

    if (cmd.to != BROADCAST && from != cmd.to)
    {
        throw std::runtime_error("expected a response from device ID " +
                lexical_cast<string>(cmd.to) + " but got one from " +
                lexical_cast<string>(from));
    }

    if (data_size < cmd.command_size)
        throw std::runtime_error("got a ACK/NAK packet with a smaller-than expected data field");

    // Validate command echo
    for (int i = 0; i < cmd.command_size; ++i)
    {
        if (data[i] != cmd.command[i])
            throw std::runtime_error("expected a ACK/NAK for command " +
                    cmd.getCommandAsString() + " but got it for " +
                    string(reinterpret_cast<char const*>(data), static_cast<string::size_type>(cmd.command_size)));
    }

    // Handle NAK responses with error information
    if (command[0] == NAK)
    {
        throw std::runtime_error("received NAK with the following error bits set: " +
                parseNACKError(data[0]));
    }
}

/**
 * Get a human-readable string representation of the command
 * Special handling for ACK and NAK commands
 */
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

// Error code descriptions for NAK responses
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

/**
 * Convert a NAK error byte into a human-readable error message
 * Each bit in the error byte represents a different error condition
 */
string Packet::parseNACKError(byte errorByte)
{
    string result;
    for (int i = 0; i < 8; ++i)
    {
        if (errorByte & (1 << i))
        {
            if (!result.empty())
                result += ", ";
            result += ERROR_CODES[i];
        }
    }
    return result;
}

/**
 * Marshal a checksum value according to protocol rules
 * Handles special cases where checksum matches field separators
 * @param checksum Raw checksum value
 * @param buffer 3-byte buffer to store encoded checksum
 */
void Packet::marshalChecksum(byte checksum, byte* buffer)
{
    if (checksum == ':' || checksum == '>')
    {
        buffer[0] = '0';
        buffer[1] = '0';
        buffer[2] = checksum;
    }
    else
    {
        buffer[0] = checksum;
        buffer[1] = '0';
        buffer[2] = '0';
    }
}

/**
 * Compare an encoded checksum with expected value
 * Handles special cases where checksum matches field separators
 * @param expected Raw checksum value to compare against
 * @param buffer Buffer containing encoded checksum
 * @return True if checksums match
 */
bool Packet::compareChecksum(byte expected, byte const* buffer)
{
    if (expected == ':' || expected == '>')
        return buffer[0] == '0' && buffer[1] == '0' && buffer[2] == expected;
    else
        return buffer[0] == expected && buffer[1] == '0' && buffer[2] == '0';
}

