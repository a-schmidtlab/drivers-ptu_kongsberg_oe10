#ifndef PTU_KONGSBERG_OE10_DRIVER_HPP
#define PTU_KONGSBERG_OE10_DRIVER_HPP

#include <ptu_kongsberg_oe10/Packet.hpp>
#include <iodrivers_base/Driver.hpp>
#include <ptu_kongsberg_oe10/Status.hpp>

namespace ptu_kongsberg_oe10
{
    class Driver
        : public iodrivers_base::Driver
    {
    public:
        Driver();

        Status getStatus(int device_id);

    protected:
        /** Read the response of a command and validates it
         *
         * The protocol specifies that the data field of ACKs starts with the
         * command that is being ACKed. This method removes the prefix (after
         * having validated it), so the returned packet's data array starts with
         * the actual data.
         */
        Packet readResponse(Packet const& cmd, int expectedSize);

        void writePacket(Packet const& packet);
        Packet readPacket();

        std::vector<boost::uint8_t> writeBuffer;
        int extractPacket(boost::uint8_t const* buffer, size_t size) const;
    };
}

#endif
