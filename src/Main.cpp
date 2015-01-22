#include <iostream>
#include <ptu_kongsberg_oe10/Driver.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using boost::lexical_cast;
using namespace ptu_kongsberg_oe10;

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

int main(int argc, char** argv)
{
    ptu_kongsberg_oe10::Driver driver;

    if (argc < 4)
        return usage(argv[0]);

    driver.openURI(argv[1]);
    int device_id = lexical_cast<int>(argv[2]);
    string cmd = argv[3];

    if (cmd == "info")
    {
        Status status = driver.getStatus(device_id);
        cout
            << "Capabilities\n"
            << "  Pan: " << status.ptu.pan << "\n"
            << "  Tilt: " << status.ptu.tilt << "\n"
            << "Temperature: " << status.temperature.getCelsius() << "\n"
            << "Humidity: " << status.humidity << "\n"
            << "Pan: " << round(status.pan * 180 / M_PI) << "\n"
            << "Tilt: " << round(status.tilt * 180 / M_PI) << endl;
    }
    else if (cmd == "status")
    {
        PanTiltStatus status = driver.getPanTiltStatus(device_id);
        cout
            << "Status\n"
            << "Pan Speed: " << status.pan_speed << "\n"
            << "Tilt Speed: " << status.tilt_speed << "\n"
            << "Pan: " << round(status.pan * 180 / M_PI) << " deg\n"
            << "Tilt: " << round(status.tilt * 180 / M_PI) << " deg\n"
            << "Uses Pan Stop: " << status.uses_pan_stop << "\n"
            << "Uses Tilt Stop: " << status.uses_tilt_stop << endl;
    }
    else if (cmd == "tilt" || cmd == "pan")
    {
        if (argc != 5 && argc != 6)
            return usage(argv[0]);

        double angle = lexical_cast<double>(argv[4]) * M_PI / 180;
        double speed = 0.1;
        if (argc == 6)
            speed = lexical_cast<double>(argv[5]);

        if (cmd == "pan")
        {
            driver.setPanSpeed(device_id, speed);
            driver.setPanPosition(device_id, angle);
        }
        else
        {
            driver.setTiltSpeed(device_id, speed);
            driver.setTiltPosition(device_id, angle);
        }
    }
    else
    {
        cerr << "Unrecognized command " << cmd << endl << endl;
        return usage(argv[0]);
    }
    return 0;
}
