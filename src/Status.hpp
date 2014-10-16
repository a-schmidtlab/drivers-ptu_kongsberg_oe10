#ifndef PTU_KONGSBERG_OE10_STATUS_HPP
#define PTU_KONGSBERG_OE10_STATUS_HPP

#include <base/Time.hpp>
#include <base/Temperature.hpp>

namespace ptu_kongsberg_oe10
{
    /** Capability of the camera */
    struct CameraCapabilities
    {
        bool enabled;
        bool focus;
        bool zoom;
        bool auto_focus;
        bool manual_exposure;
        bool stills;
        bool wipers;
        bool washer;
        bool lamp_control;
        bool flash;
        bool flash_charged;
    };

    /** Capability of the PTU */
    struct PTUCapabilities
    {
        bool pan;
        bool tilt;
    };

    /** Global status information reported by the PTU
     */
    struct Status
    {
        CameraCapabilities camera;
        PTUCapabilities ptu;

        base::Time time;
        base::Temperature temperature;
        float humidity;

        float pan;
        float tilt;
    };
}

#endif
