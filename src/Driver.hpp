#ifndef PTU_KONGSBERG_OE10_DRIVER_HPP
#define PTU_KONGSBERG_OE10_DRIVER_HPP

// Include required headers for packet handling, base driver functionality, and status structures
#include <ptu_kongsberg_oe10/Packet.hpp>
#include <iodrivers_base/Driver.hpp>
#include <ptu_kongsberg_oe10/Status.hpp>
#include <ptu_kongsberg_oe10/PanTiltStatus.hpp>

namespace ptu_kongsberg_oe10
{
    /**
     * Driver class for controlling the Kongsberg OE10 Pan-Tilt Unit (PTU)
     * This class provides a high-level interface to control pan and tilt movements,
     * manage end stops, and retrieve status information from the device.
     */
    class Driver
        : public iodrivers_base::Driver
    {
    public:
        /** Constructor initializes the driver with default settings */
        Driver();

        /**
         * Retrieves the complete status of the device including capabilities and positions
         * @param device_id The ID of the target device (0xFF for broadcast)
         * @return Status structure containing device information
         */
        Status getStatus(int device_id);

        /**
         * Asynchronously requests pan-tilt status from the device
         * @param device_id The ID of the target device
         */
        void requestPanTiltStatus(int device_id);

        /**
         * Reads the previously requested pan-tilt status
         * @param device_id The ID of the target device
         * @return PanTiltStatus structure containing current positions and speeds
         */
        PanTiltStatus readPanTiltStatus(int device_id);

        /**
         * Synchronously gets the current pan-tilt status
         * @param device_id The ID of the target device
         * @return PanTiltStatus structure containing current positions and speeds
         */
        PanTiltStatus getPanTiltStatus(int device_id);

        /**
         * Enables or disables the use of end stops for safety
         * @param device_id The ID of the target device
         * @param enable True to enable end stops, false to disable
         */
        void useEndStops(int device_id, bool enable);

        /**
         * Sets the positive (maximum) end stop for the pan axis
         * @param device_id The ID of the target device
         */
        void setPanPositiveEndStop(int device_id);

        /**
         * Sets the negative (minimum) end stop for the pan axis
         * @param device_id The ID of the target device
         */
        void setPanNegativeEndStop(int device_id);

        /**
         * Sets the positive (maximum) end stop for the tilt axis
         * @param device_id The ID of the target device
         */
        void setTiltPositiveEndStop(int device_id);

        /**
         * Sets the negative (minimum) end stop for the tilt axis
         * @param device_id The ID of the target device
         */
        void setTiltNegativeEndStop(int device_id);

        /**
         * Sets the pan position of the device
         * @param device_id The ID of the target device
         * @param pan Target pan angle in radians
         */
        void setPanPosition(int device_id, float pan);

        /**
         * Sets the tilt position of the device
         * @param device_id The ID of the target device
         * @param tilt Target tilt angle in radians
         */
        void setTiltPosition(int device_id, float tilt);

        /**
         * Sets the pan movement speed
         * @param device_id The ID of the target device
         * @param speed Speed as fraction of maximum (0.0 to 1.0)
         */
        void setPanSpeed(int device_id, float speed);

        /**
         * Sets the tilt movement speed
         * @param device_id The ID of the target device
         * @param speed Speed as fraction of maximum (0.0 to 1.0)
         */
        void setTiltSpeed(int device_id, float speed);

        /**
         * Initiates upward tilt movement
         * @param device_id The ID of the target device
         * @return Current tilt angle in radians
         */
        double tiltUp(int device_id);

        /**
         * Initiates downward tilt movement
         * @param device_id The ID of the target device
         * @return Current tilt angle in radians
         */
        double tiltDown(int device_id);

        /**
         * Stops the tilt movement
         * @param device_id The ID of the target device
         * @return Final tilt angle in radians
         */
        double tiltStop(int device_id);

    protected:
        /** 
         * Reads and validates the response to a command
         * The protocol specifies that the data field of ACKs starts with the
         * command that is being ACKed. This method removes the prefix (after
         * having validated it), so the returned packet's data array starts with
         * the actual data.
         * 
         * @param cmd The original command packet
         * @param expectedSize Expected size of the response data
         * @return Validated response packet
         */
        Packet readResponse(Packet const& cmd, int expectedSize);

        /**
         * Helper method to set position for either pan or tilt axis
         * @param device_id The ID of the target device
         * @param axis Axis identifier ('P' for pan, 'T' for tilt)
         * @param angle Target angle in radians
         */
        void setPosition(int device_id, char axis, float angle);

        /**
         * Helper method to set speed for either pan or tilt axis
         * @param device_id The ID of the target device
         * @param cmd0 First command byte
         * @param cmd1 Second command byte
         * @param speed Speed as fraction of maximum (0.0 to 1.0)
         */
        void setSpeed(int device_id, char cmd0, char cmd1, float speed);

        /**
         * Executes a simple movement command (like tilt up/down)
         * @param device_id The ID of the target device
         * @param cmd0 First command byte
         * @param cmd1 Second command byte
         * @return Current position after movement initiation
         */
        double simpleMovement(int device_id, char cmd0, char cmd1);

        /**
         * Helper method to set end stops
         * @param device_id The ID of the target device
         * @param cmd0 First command byte
         * @param cmd1 Second command byte
         */
        void setEndStop(int device_id, char cmd0, char cmd1);

        /**
         * Low-level method to write a packet to the device
         * @param packet The packet to write
         */
        void writePacket(Packet const& packet);

        /**
         * Low-level method to read a packet from the device
         * @return The read packet
         */
        Packet readPacket();

        /** Buffer for writing data to the device */
        std::vector<boost::uint8_t> writeBuffer;

        /**
         * Extracts a packet from the raw buffer
         * @param buffer Raw data buffer
         * @param size Size of the buffer
         * @return Size of the extracted packet, or -1 if no complete packet
         */
        int extractPacket(boost::uint8_t const* buffer, size_t size) const;
    };
}

#endif
