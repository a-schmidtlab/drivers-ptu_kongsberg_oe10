#ifndef PTU_KONGSBERG_OE10_STATUS_HPP
#define PTU_KONGSBERG_OE10_STATUS_HPP

// Include required base types for time and temperature measurements
#include <base/Time.hpp>
#include <base/Temperature.hpp>

namespace ptu_kongsberg_oe10
{
    /**
     * Structure defining the capabilities of the attached camera
     * This represents all possible features that may be available
     * on the camera unit connected to the PTU
     */
    struct CameraCapabilities
    {
        bool enabled;        ///< Whether the camera is enabled and operational
        bool focus;         ///< Whether focus control is available
        bool zoom;          ///< Whether zoom control is available
        bool auto_focus;    ///< Whether auto-focus capability is available
        bool manual_exposure; ///< Whether manual exposure control is available
        bool stills;        ///< Whether still image capture is supported
        bool wipers;        ///< Whether camera has wiper functionality
        bool washer;        ///< Whether camera has washer functionality
        bool lamp_control;  ///< Whether lighting control is available
        bool flash;         ///< Whether flash functionality is available
        bool flash_charged; ///< Whether flash is charged and ready to use
    };

    /**
     * Structure defining the movement capabilities of the PTU
     * Indicates which axes of movement are available
     */
    struct PTUCapabilities
    {
        bool pan;   ///< Whether pan (horizontal) movement is available
        bool tilt;  ///< Whether tilt (vertical) movement is available
    };

    /**
     * Comprehensive status information reported by the PTU device
     * This structure combines all status information including
     * capabilities, environmental conditions, and current positions
     */
    struct Status
    {
        CameraCapabilities camera;  ///< Status of camera capabilities
        PTUCapabilities ptu;       ///< Status of PTU movement capabilities

        base::Time time;           ///< Timestamp of the status reading
        base::Temperature temperature; ///< Current temperature reading
        float humidity;            ///< Current humidity level (0-100%)

        float pan;                 ///< Current pan position in radians
        float tilt;               ///< Current tilt position in radians
    };
}

#endif
