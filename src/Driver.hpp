#ifndef PTU_KONGSBERG_OE10_DRIVER_HPP
#define PTU_KONGSBERG_OE10_DRIVER_HPP

#include <ptu_kongsberg_oe10/Packet.hpp>
#include <iodrivers_base/Driver.hpp>
#include <ptu_kongsberg_oe10/Status.hpp>
#include <ptu_kongsberg_oe10/PanTiltStatus.hpp>

namespace ptu_kongsberg_oe10
{
    class Driver
        : public iodrivers_base::Driver
    {
    public:
        Driver();

        Status getStatus(int device_id);

        void requestPanTiltStatus(int device_id);
        PanTiltStatus readPanTiltStatus(int device_id);
        PanTiltStatus getPanTiltStatus(int device_id);

        void useEndStops(int device_id, bool enable);
        void setPanPositiveEndStop(int device_id);
        void setPanNegativeEndStop(int device_id);
        void setTiltPositiveEndStop(int device_id);
        void setTiltNegativeEndStop(int device_id);

        void setPanPosition(int device_id, float pan);
        void setTiltPosition(int device_id, float tilt);

        void setPanSpeed(int device_id, float speed);
        void setTiltSpeed(int device_id, float speed);

	double tiltUp(int device_id);
	double tiltDown(int device_id);
	double tiltStop(int device_id);

    protected:
        /** Read the response of a command and validates it
         *
         * The protocol specifies that the data field of ACKs starts with the
         * command that is being ACKed. This method removes the prefix (after
         * having validated it), so the returned packet's data array starts with
         * the actual data.
         */
        Packet readResponse(Packet const& cmd, int expectedSize);

        void setPosition(int device_id, char axis, float angle);
        void setSpeed(int device_id, char cmd0, char cmd1, float speed);
        double simpleMovement(int device_id, char cmd0, char cmd1);
        void setEndStop(int device_id, char cmd0, char cmd1);

        void writePacket(Packet const& packet);
        Packet readPacket();

        std::vector<boost::uint8_t> writeBuffer;
        int extractPacket(boost::uint8_t const* buffer, size_t size) const;
    };
}

#endif
