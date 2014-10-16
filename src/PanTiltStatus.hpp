#ifndef PTU_KONGSBERG_OE10_PAN_TILT_STATUS_HPP
#define PTU_KONGSBERG_OE10_PAN_TILT_STATUS_HPP

#include <base/Time.hpp>

namespace ptu_kongsberg_oe10 {
    /** Status of the pan/tilt angles, speeds and end stop
     */
    struct PanTiltStatus
    {
        /** The time at which this status got acquired */
        base::Time time;

        /** Position on the pan axis */
        float pan;

        /** Position on the tilt axis */
        float tilt;

        /** Speed on the pan axis, as a fraction of maximal speed (0 for
         * stopped, 1 for maximal speed)
         */
        float pan_speed;

        /** Speed on the tilt axis, as a fraction of maximal speed (0 for
         * stopped, 1 for maximal speed)
         */
        float tilt_speed;

        /** Whether the unit uses pan stops */
        bool uses_pan_stop;
        /** Whether the unit uses tilt stops */
        bool uses_tilt_stop;
    };
}

#endif
