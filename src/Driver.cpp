#include <base/Logging.hpp>
#include <ptu_kongsberg_oe10/Driver.hpp>
#include <stdexcept>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace ptu_kongsberg_oe10;
using boost::lexical_cast;

/**
 * Constructor initializes the driver with maximum packet size and default timeouts
 * Sets both read and write timeouts to 2 seconds for reliable communication
 */
Driver::Driver()
    : iodrivers_base::Driver(Packet::MAX_PACKET_SIZE)
{
    setReadTimeout(base::Time::fromSeconds(2));
    setWriteTimeout(base::Time::fromSeconds(2));
}

/**
 * Configures whether the device should use end stops for safety
 * End stops prevent the PTU from moving beyond its physical limits
 */
void Driver::useEndStops(int device_id, bool enable)
{
    // Create and configure the command packet
    Packet packet(device_id);
    packet.setCommand('E', 'S');
    packet.data_size = 1;
    packet.data[0] = enable ? 0x31 : 0x30;  // 0x31 = '1' (enable), 0x30 = '0' (disable)
    writePacket(packet);

    // Read and validate response
    Packet response = readResponse(packet, 1);
    if (response.data[2] != packet.data[0])
        throw std::runtime_error("boolean in the reply for use end stops command mismatches the sent command");
}

// Pan end stop configuration methods
void Driver::setPanPositiveEndStop(int device_id)
{
    setEndStop(device_id, 'C', 'W');  // CW = Clockwise limit
}

void Driver::setPanNegativeEndStop(int device_id)
{
    setEndStop(device_id, 'A', 'W');  // AW = Anti-clockwise limit
}

// Tilt end stop configuration methods
void Driver::setTiltPositiveEndStop(int device_id)
{
    setEndStop(device_id, 'U', 'T');  // UT = Up tilt limit
}

void Driver::setTiltNegativeEndStop(int device_id)
{
    setEndStop(device_id, 'D', 'T');  // DT = Down tilt limit
}

/**
 * Generic method to set an end stop position
 * Used internally by the specific end stop setting methods
 */
void Driver::setEndStop(int device_id, char cmd0, char cmd1)
{
    Packet packet(device_id);
    packet.setCommand(cmd0, cmd1);
    writePacket(packet);
    readResponse(packet, 0);  // Expect empty response
}

/**
 * Retrieves comprehensive status information from the device
 * Includes camera capabilities, PTU capabilities, temperature,
 * humidity, and current positions
 */
Status Driver::getStatus(int device_id)
{
    // Request status information
    Packet packet(device_id);
    packet.setCommand('S', 'T');
    writePacket(packet);
    Packet response = readResponse(packet, 9);
    
    Status status;
    // Parse capability flags from first three bytes
    byte b0 = response.data[0];
    byte b1 = response.data[1];
    byte b2 = response.data[2];

    // Parse camera capabilities from first byte
    status.camera.enabled         = (b0 & 0x01) != 0;
    status.camera.focus           = (b0 & 0x02) != 0;
    status.camera.zoom            = (b0 & 0x04) != 0;
    status.ptu.pan                = (b0 & 0x08) != 0;
    status.ptu.tilt               = (b0 & 0x10) != 0;
    status.camera.auto_focus      = (b0 & 0x20) != 0;
    status.camera.manual_exposure = (b0 & 0x40) != 0;
    status.camera.stills          = (b0 & 0x80) != 0;

    // Parse additional camera capabilities from second byte
    status.camera.wipers          = (b1 & 0x01) != 0;
    status.camera.washer          = (b1 & 0x02) != 0;
    status.camera.lamp_control    = (b1 & 0x04) != 0;
    status.camera.flash           = (b1 & 0x08) != 0;
    status.camera.flash_charged   = (b1 & 0x10) != 0;

    // Parse environmental data from third byte
    status.temperature = base::Temperature::fromCelsius((b2 & 0xF) * 5 - 5);
    status.humidity    = static_cast<int>(b2 >> 8) * 100 / 16;

    // Parse current positions
    float pan  = Packet::parseAngle(response.data + 3);
    float tilt = Packet::parseAngle(response.data + 6);
    status.pan = pan;
    status.tilt = tilt;
    return status;
}

/**
 * Sends an asynchronous request for pan/tilt status
 * Use readPanTiltStatus to get the response
 */
void Driver::requestPanTiltStatus(int device_id)
{
    Packet packet(device_id);
    packet.setCommand('A', 'S');
    writePacket(packet);
}

/**
 * Reads and parses the response to a pan/tilt status request
 * Returns current speeds, positions, and end stop usage
 */
PanTiltStatus Driver::readPanTiltStatus(int device_id)
{
    Packet packet(device_id);
    packet.setCommand('A', 'S');
    Packet response = readResponse(packet, 10);

    PanTiltStatus status;
    status.time = base::Time::now();
    // Parse speeds (0x64 = 100, so dividing gives percentage)
    status.pan_speed  = static_cast<float>(response.data[0]) / 0x64;
    status.tilt_speed = static_cast<float>(response.data[1]) / 0x64;
    // Parse current positions
    status.pan  = Packet::parseAngle(response.data + 2);
    status.tilt = Packet::parseAngle(response.data + 5);
    // Parse end stop usage (0x31 = '1' means enabled)
    status.uses_pan_stop  = (response.data[8] == 0x31);
    status.uses_tilt_stop = (response.data[9] == 0x31);
    return status;
}

/**
 * Synchronous method to get pan/tilt status
 * Combines requestPanTiltStatus and readPanTiltStatus
 */
PanTiltStatus Driver::getPanTiltStatus(int device_id)
{
    requestPanTiltStatus(device_id);
    return readPanTiltStatus(device_id);
}

// Position control methods
void Driver::setPanPosition(int device_id, float pan)
{
    return setPosition(device_id, 'P', pan);
}

void Driver::setTiltPosition(int device_id, float tilt)
{
    return setPosition(device_id, 'T', tilt);
}

// Simple tilt movement controls
double Driver::tiltUp(int device_id)
{
    return simpleMovement(device_id, 'T', 'U');
}

double Driver::tiltDown(int device_id)
{
    return simpleMovement(device_id, 'T', 'D');
}

double Driver::tiltStop(int device_id)
{
    return simpleMovement(device_id, 'T', 'S');
}

/**
 * Generic method for simple movement commands
 * Returns the current position after initiating movement
 */
double Driver::simpleMovement(int device_id, char cmd0, char cmd1)
{
    Packet packet(device_id);
    packet.setCommand(cmd0, cmd1);
    writePacket(packet);
    Packet response = readResponse(packet, 3);
    return Packet::parseAngle(response.data);
}

/**
 * Sets the position for either pan or tilt axis
 * @param axis 'P' for pan, 'T' for tilt
 */
void Driver::setPosition(int device_id, char axis, float angle)
{
    Packet packet(device_id);
    packet.setCommand(axis, 'P');
    packet.data_size = 3;
    Packet::encodeAngle(packet.data, angle);
    writePacket(packet);
    readResponse(packet, 3);
}

// Speed control methods
void Driver::setPanSpeed(int device_id, float speed)
{
    setSpeed(device_id, 'D', 'S', speed);
}

void Driver::setTiltSpeed(int device_id, float speed)
{
    setSpeed(device_id, 'T', 'A', speed);
}

/**
 * Sets the speed for either pan or tilt movement
 * @param speed Value between 0.0 (stopped) and 1.0 (maximum speed)
 */
void Driver::setSpeed(int device_id, char cmd0, char cmd1, float speed)
{
    if (speed < 0 || speed > 1)
        throw std::range_error("invalid range for speed, should be in [0,1] and got " + lexical_cast<string>(speed));

    Packet packet(device_id);
    packet.setCommand(cmd0, cmd1);
    packet.data_size = 1;
    packet.data[0] = round(speed * 0x64);  // Convert to percentage (0-100)
    writePacket(packet);
    readResponse(packet, 0);
}

/**
 * Reads and validates a response packet
 * Handles command echo in response and validates data size
 */
Packet Driver::readResponse(Packet const& cmd, int expectedSize)
{
    Packet response = readPacket();
    response.validateResponseFor(cmd);
    if (response.data_size != (expectedSize + cmd.command_size))
    {
        throw std::runtime_error("expected response to " + cmd.getCommandAsString() + " with " +
                lexical_cast<string>(expectedSize) +
                " bytes of data, but got " +
                lexical_cast<string>(static_cast<int>(response.data_size)));
    }
    // Remove echoed command from response data
    memmove(response.data,
            response.data + cmd.command_size,
            response.data_size - cmd.command_size);
    response.data_size -= cmd.command_size;
    return response;
}

/**
 * Low-level method to read a packet from the device
 * Handles the actual communication and packet parsing
 */
Packet Driver::readPacket()
{
    byte buffer[Packet::MAX_PACKET_SIZE];
    int packetSize = iodrivers_base::Driver::readPacket(buffer, Packet::MAX_PACKET_SIZE);
    return Packet::parse(buffer, packetSize, false);
}

/**
 * Low-level method to write a packet to the device
 * Handles packet marshalling and actual communication
 */
void Driver::writePacket(Packet const& packet)
{
    writeBuffer.clear();
    packet.marshal(writeBuffer);
    LOG_DEBUG_S << "writing " << writeBuffer.size() << " bytes: " << Packet::kongsberg_com(&writeBuffer[0], writeBuffer.size());
    iodrivers_base::Driver::writePacket(&writeBuffer[0], writeBuffer.size());
}

/**
 * Extracts a complete packet from the raw buffer
 * Used by the base driver class for packet extraction
 */
int Driver::extractPacket(boost::uint8_t const* buffer, size_t size) const
{
    return Packet::extractPacket(buffer, size);
}

