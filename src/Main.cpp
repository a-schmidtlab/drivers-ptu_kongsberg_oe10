// Standard includes for I/O operations
#include <iostream>
// Include the custom PTU (Pan-Tilt Unit) driver header
#include <ptu_kongsberg_oe10/Driver.hpp>
// Include boost library for string-to-number conversion
#include <boost/lexical_cast.hpp>

// Using declarations to simplify code
using namespace std;
using boost::lexical_cast;
using namespace ptu_kongsberg_oe10;

/**
 * Displays the usage information for the program
 * @param argv0 The program name (from command line)
 * @return -1 to indicate improper usage
 */
static int usage(string const& argv0)
{
    cerr
        << "usage: " << argv0 << " DEVICE DEVICE_ID CMD [ARGS]\n"
        << "  use 0xFF as device ID for broadcast, otherwise use the\n"
        << "  actual device ID\n"
        << "\n"
        << "  the following commands are recognized:\n"
        << "\n"
        << "  info\n"
        << "      reports the device's general info\n"
        << "  status\n"
        << "      reports the device's axis positions and associated info\n"
        << "  pan ANGLE [SPEED]\n"
        << "      moves the pan axis to the specified angle. Angle is\n"
        << "      specified in degrees and must be between 0 and 360.\n"
        << "      The speed is specified at a fraction of the maximum\n"
        << "      speed (between 0 and 1) and defaults to 0.1.\n"
        << "  tilt ANGLE\n"
        << "      moves the tilt axis to the specified angle. Angle is\n"
        << "      specified in degrees and must be between 0 and 360\n"
        << "      The speed is specified at a fraction of the maximum\n"
        << "      speed (between 0 and 1) and defaults to 0.1.\n"
        << endl;

    return -1;
}

/**
 * Main program entry point for controlling the Kongsberg OE10 Pan-Tilt Unit
 * Supports commands for:
 * - Getting device information
 * - Checking device status
 * - Controlling pan movement
 * - Controlling tilt movement
 */
int main(int argc, char** argv)
{
    // Create an instance of the PTU driver
    ptu_kongsberg_oe10::Driver driver;

    // Check if minimum required arguments are provided
    if (argc < 4)
        return usage(argv[0]);

    // Initialize the device connection with provided URI (e.g., serial port)
    driver.openURI(argv[1]);
    // Convert device ID from string to integer (0xFF for broadcast)
    int device_id = lexical_cast<int>(argv[2]);
    // Get the command to execute
    string cmd = argv[3];

    // Handle "info" command - displays general device information
    if (cmd == "info")
    {
        // Get current device status including capabilities and positions
        Status status = driver.getStatus(device_id);
        cout
            << "Capabilities\n"
            << "  Pan: " << status.ptu.pan << "\n"      // Pan axis capabilities
            << "  Tilt: " << status.ptu.tilt << "\n"    // Tilt axis capabilities
            << "Temperature: " << status.temperature.getCelsius() << "\n"  // Device temperature
            << "Humidity: " << status.humidity << "\n"   // Device humidity
            << "Pan: " << round(status.pan * 180 / M_PI) << "\n"    // Current pan position in degrees
            << "Tilt: " << round(status.tilt * 180 / M_PI) << endl; // Current tilt position in degrees
    }
    // Handle "status" command - displays current motion status
    else if (cmd == "status")
    {
        // Get detailed pan-tilt status information
        PanTiltStatus status = driver.getPanTiltStatus(device_id);
        cout
            << "Status\n"
            << "Pan Speed: " << status.pan_speed << "\n"     // Current pan axis speed
            << "Tilt Speed: " << status.tilt_speed << "\n"   // Current tilt axis speed
            << "Pan: " << round(status.pan * 180 / M_PI) << " deg\n"   // Current pan position
            << "Tilt: " << round(status.tilt * 180 / M_PI) << " deg\n" // Current tilt position
            << "Uses Pan Stop: " << status.uses_pan_stop << "\n"    // Pan limit switch status
            << "Uses Tilt Stop: " << status.uses_tilt_stop << endl; // Tilt limit switch status
    }
    // Handle movement commands (pan or tilt)
    else if (cmd == "tilt" || cmd == "pan")
    {
        // Check if correct number of arguments is provided
        if (argc != 5 && argc != 6)
            return usage(argv[0]);

        // Convert angle from degrees to radians
        double angle = lexical_cast<double>(argv[4]) * M_PI / 180;
        // Set default speed to 10% if not specified
        double speed = 0.1;
        if (argc == 6)
            speed = lexical_cast<double>(argv[5]);

        // Execute pan movement
        if (cmd == "pan")
        {
            driver.setPanSpeed(device_id, speed);     // Set pan movement speed
            driver.setPanPosition(device_id, angle);  // Set target pan position
        }
        // Execute tilt movement
        else
        {
            driver.setTiltSpeed(device_id, speed);    // Set tilt movement speed
            driver.setTiltPosition(device_id, angle); // Set target tilt position
        }
    }
    // Handle unrecognized commands
    else
    {
        cerr << "Unrecognized command " << cmd << endl << endl;
        return usage(argv[0]);
    }
    return 0;
}
