#ifndef PTU_KONGSBERG_OE10
#define PTU_KONGSBERG_OE10

#include <boost/cstdint.hpp>
#include <base/Angle.hpp>
#include <vector>

namespace ptu_kongsberg_oe10
{
    /** Type alias for byte-level operations */
    typedef boost::uint8_t byte;

    /**
     * Class representing a communication packet for the Kongsberg OE10 protocol
     * 
     * The protocol uses a binary packet format with the following structure:
     * - Device IDs (from/to)
     * - Command (1-2 bytes)
     * - Optional data payload
     * - Checksum for error detection
     * 
     * Special characters in the protocol:
     * - ':' and '>' are used as field separators
     * - ACK (0x06) indicates successful command execution
     * - NAK (0x15) indicates command failure
     */
    struct Packet
    {
        /** Default controller ID in the protocol */
        static const int CONTROLLER = 0x01;
        /** Broadcast address for commands to all devices */
        static const int BROADCAST  = 0xFF;
        /** Maximum size of the data payload */
        static const int MAX_DATA_SIZE   = 0xFF;
        /** Maximum total packet size including headers and payload */
        static const int MAX_PACKET_SIZE = 14 + MAX_DATA_SIZE;

        /** Acknowledgment byte indicating successful command */
        static const byte ACK = 0x06;
        /** Negative acknowledgment byte indicating command failure */
        static const byte NAK = 0x15;

        /** Source device ID of the packet */
        byte from;
        /** Destination device ID for the packet */
        byte to;
        /** Size of the command field (1 or 2 bytes) */
        byte command_size;
        /** Command bytes - can be one or two bytes */
        byte command[2];
        /** Size of the data payload */
        byte data_size;
        /** Data payload buffer */
        byte data[MAX_DATA_SIZE];

        /**
         * Constructor initializes a packet with destination and source IDs
         * @param to Destination device ID (defaults to broadcast)
         * @param from Source device ID (defaults to controller)
         */
        Packet(byte to = BROADCAST, byte from = CONTROLLER);

        /**
         * Sets a single-byte command
         * @param c0 Command byte
         */
        void setCommand(byte c0);

        /**
         * Sets a two-byte command
         * @param c0 First command byte
         * @param c1 Second command byte
         */
        void setCommand(byte c0, byte c1);

        /**
         * Serializes the packet into a byte buffer for transmission
         * Handles special character escaping and checksum calculation
         * @param buffer Vector to store the serialized packet
         */
        void marshal(std::vector<byte>& buffer) const;

        /**
         * Converts a 3-byte angle representation to float
         * @param buffer Pointer to the 3-byte angle data
         * @return Angle value in radians
         */
        static float parseAngle(byte const* buffer);

        /**
         * Converts a float angle to 3-byte representation
         * @param buffer Buffer to store the encoded angle
         * @param angle Angle value in radians
         */
        static void encodeAngle(byte* buffer, float angle);

        /**
         * Calculates packet checksum for error detection
         * @param begin Start of data to checksum
         * @param end End of data to checksum
         * @return Computed checksum byte
         */
        static byte computeChecksum(byte const* begin, byte const* end);

        /**
         * Encodes a checksum value according to protocol rules
         * Handles special cases where checksum matches field separators
         * @param checksum Checksum value to encode
         * @param buffer Buffer to store encoded checksum
         */
        static void marshalChecksum(byte checksum, byte* buffer);

        /**
         * Validates an encoded checksum against expected value
         * Handles special cases where checksum matches field separators
         * @param expected Expected checksum value
         * @param buffer Buffer containing encoded checksum
         * @return True if checksum matches
         */
        static bool compareChecksum(byte expected, byte const* buffer);

        /**
         * Extracts a complete packet from a buffer of received data
         * Used by the driver's extractPacket method for packet framing
         * @param buffer Raw received data
         * @param size Size of received data
         * @return Size of packet if found, 0 if incomplete, -1 if invalid
         */
        static int extractPacket(byte const* buffer, int size);

        /**
         * Parses a complete packet from a buffer into a Packet structure
         * @param buffer Buffer containing complete packet
         * @param size Size of the packet data
         * @param validate Whether to validate packet integrity
         * @return Parsed Packet structure
         */
        static Packet parse(byte const* buffer, int size, bool validate = true);

        /**
         * Validates that this packet is a proper response to a command
         * Checks device IDs, command echo, and ACK/NAK status
         * @param cmd Original command packet
         * @throws runtime_error if validation fails
         */
        void validateResponseFor(Packet const& cmd);

        /**
         * Creates a human-readable string of the command
         * Useful for debugging and error messages
         * @return String representation of the command
         */
        std::string getCommandAsString() const;

        /**
         * Parses a NAK error byte into a human-readable message
         * @param errorByte The error code from a NAK response
         * @return Human-readable error description
         */
        static std::string parseNACKError(byte errorByte);

        /**
         * Creates a debug representation of raw packet data
         * @param buffer Raw packet data
         * @param size Size of the data
         * @return Human-readable packet representation
         */
        static std::string kongsberg_com(byte const* buffer, int size);
    };
}

#endif

