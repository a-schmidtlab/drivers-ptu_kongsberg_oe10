#ifndef PTU_KONGSBERG_OE10
#define PTU_KONGSBERG_OE10

#include <boost/cstdint.hpp>
#include <base/Angle.hpp>
#include <vector>

namespace ptu_kongsberg_oe10
{
    typedef boost::uint8_t byte;

    /** Management of a communication packet */
    struct Packet
    {
        static const int CONTROLLER = 0x01;
        static const int BROADCAST  = 0xFF;
        static const int MAX_DATA_SIZE   = 0xFF;
        static const int MAX_PACKET_SIZE = 14 + MAX_DATA_SIZE;

        static const byte ACK = 0x06;
        static const byte NAK = 0x15;

        /** The device ID that sent this packet */
        byte from;
        /** The device ID for which this packet is intended */
        byte to;
        /** The size of the command field (can either be 1 or 2) */
        byte command_size;
        byte command[2];
        /** The size of the data field */
        byte data_size;
        byte data[MAX_DATA_SIZE];

        Packet(byte to = BROADCAST, byte from = CONTROLLER);

        /** Set a one-byte command */
        void setCommand(byte c0);
        /** Set a two-byte command */
        void setCommand(byte c0, byte c1);

        /** Marshal this packet in a ready-to-send form */
        void marshal(std::vector<byte>& buffer) const;

        /** Parse a 3-byte representation of an angle */
        static float parseAngle(byte const* buffer);

        /** Compute the checksum byte */
        static byte computeChecksum(byte const* begin, byte const* end);

        /** Compute the checksum ind byte */
        static byte computeChecksumInd(byte checksum);

        /** Gives information about the presence of a packet in the provided
         * buffer
         *
         * See the documentation of iodrivers_base::extractPacket for the
         * complete documentation
         */
        static int extractPacket(byte const* buffer, int size);

        /** Parses a packet contained in a buffer into a Packet structure
         *
         * The packet is assumed to have been validated using extractPacket
         * already
         */
        static Packet parse(byte const* buffer, int size, bool validate = true);

        /** Validates that this packet is a proper response for the command
         * given as argument
         */
        void validateResponseFor(Packet const& cmd);

        /** Return a printable representation of the command
         */
        std::string getCommandAsString() const;

        static std::string parseNACKError(byte errorByte);

        static std::string kongsberg_com(byte const* buffer, int size);
    };
}

#endif

