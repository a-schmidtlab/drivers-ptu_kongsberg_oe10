#ifndef PTU_KONGSBERG_OE10_PAN_TILT_STATUS_HPP
#define PTU_KONGSBERG_OE10_PAN_TILT_STATUS_HPP

// Include base time type for timestamps
#include <base/Time.hpp>

namespace ptu_kongsberg_oe10 {
    /**
     * Detailed status information for the pan/tilt unit
     * This structure provides comprehensive information about
     * the current state of both pan and tilt axes, including
     * positions, speeds, and end stop usage
     */
    struct PanTiltStatus
    {
        /** 
         * The time at which this status was acquired
         * Used for tracking when the status information was read from the device
         */
        base::Time time;

        /** 
         * Current position on the pan axis in radians
         * Positive values indicate clockwise rotation from the zero position
         */
        float pan;

        /** 
         * Current position on the tilt axis in radians
         * Positive values indicate upward tilt from the horizontal position
         */
        float tilt;

        /** 
         * Current speed on the pan axis
         * Expressed as a fraction of maximal speed:
         * - 0.0 means the axis is stopped
         * - 1.0 means the axis is moving at maximum speed
         */
        float pan_speed;

        /** 
         * Current speed on the tilt axis
         * Expressed as a fraction of maximal speed:
         * - 0.0 means the axis is stopped
         * - 1.0 means the axis is moving at maximum speed
         */
        float tilt_speed;

        /** 
         * Indicates whether the unit is configured to use pan end stops
         * When true, the pan axis will stop at defined limits
         */
        bool uses_pan_stop;

        /** 
         * Indicates whether the unit is configured to use tilt end stops
         * When true, the tilt axis will stop at defined limits
         */
        bool uses_tilt_stop;
    };
}

#endif
