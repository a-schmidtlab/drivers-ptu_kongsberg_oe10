#include <base/Logging.hpp>
#include <ptu_kongsberg_oe10/Driver.hpp>
#include <stdexcept>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace ptu_kongsberg_oe10;
using boost::lexical_cast;

Driver::Driver()
    : iodrivers_base::Driver(Packet::MAX_PACKET_SIZE)
{
    setReadTimeout(base::Time::fromSeconds(2));
    setWriteTimeout(base::Time::fromSeconds(2));
}

Status Driver::getStatus(int device_id)
{
    Packet packet(device_id);
    packet.setCommand('S', 'T');
    writePacket(packet);
    Packet response = readResponse(packet, 9);
    
    Status status;
    byte b0 = response.data[0];
    byte b1 = response.data[1];
    byte b2 = response.data[2];
    status.camera.enabled         = (b0 & 0x01) != 0;
    status.camera.focus           = (b0 & 0x02) != 0;
    status.camera.zoom            = (b0 & 0x04) != 0;
    status.ptu.pan                = (b0 & 0x08) != 0;
    status.ptu.tilt               = (b0 & 0x10) != 0;
    status.camera.auto_focus      = (b0 & 0x20) != 0;
    status.camera.manual_exposure = (b0 & 0x40) != 0;
    status.camera.stills          = (b0 & 0x80) != 0;
    status.camera.wipers          = (b1 & 0x01) != 0;
    status.camera.washer          = (b1 & 0x02) != 0;
    status.camera.lamp_control    = (b1 & 0x04) != 0;
    status.camera.flash           = (b1 & 0x08) != 0;
    status.camera.flash_charged   = (b1 & 0x10) != 0;

    status.temperature = base::Temperature::fromCelsius((b2 & 0xF) * 5 - 5);
    status.humidity    = static_cast<int>(b2 >> 8) * 100 / 16;

    float pan  = Packet::parseAngle(response.data + 3);
    float tilt = Packet::parseAngle(response.data + 6);
    status.pan = base::Angle::fromRad(pan);
    status.tilt = base::Angle::fromRad(tilt);
    return status;
}

Packet Driver::readResponse(Packet const& cmd, int expectedSize)
{
    Packet response = readPacket();
    response.validateResponseFor(cmd);
    if (response.data_size != (expectedSize + cmd.command_size))
    {
        throw std::runtime_error("expected response with " +
                lexical_cast<string>(expectedSize) +
                " bytes of data, but got " +
                lexical_cast<string>(response.data_size));
    }
    memmove(response.data,
            response.data + cmd.command_size,
            response.data_size - cmd.command_size);
    response.data_size -= cmd.command_size;
    return response;
}

Packet Driver::readPacket()
{
    byte buffer[Packet::MAX_PACKET_SIZE];
    int packetSize = iodrivers_base::Driver::readPacket(buffer, Packet::MAX_PACKET_SIZE);
    return Packet::parse(buffer, packetSize, false);
}

void Driver::writePacket(Packet const& packet)
{
    writeBuffer.clear();
    packet.marshal(writeBuffer);
    LOG_DEBUG_S << "writing " << writeBuffer.size() << " bytes: " << Packet::kongsberg_com(&writeBuffer[0], writeBuffer.size());
    iodrivers_base::Driver::writePacket(&writeBuffer[0], writeBuffer.size());
}

int Driver::extractPacket(boost::uint8_t const* buffer, size_t size) const
{
    return Packet::extractPacket(buffer, size);
}

