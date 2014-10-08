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
        << "  status\n"
        << "      reports the device's status\n"
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

    if (cmd == "status")
    {
        Status status = driver.getStatus(device_id);
        cout
            << "Capabilities\n"
            << "  Pan: " << status.ptu.pan << "\n"
            << "  Tilt: " << status.ptu.tilt << "\n"
            << "Temperature: " << status.temperature.getCelsius() << "\n"
            << "Humidity: " << status.humidity << "\n"
            << "Pan: " << status.pan.getDeg() << "\n"
            << "Tilt: " << status.tilt.getDeg() << endl;
    }
    else
    {
        cerr << "Unrecognized command " << cmd << endl << endl;
        return usage(argv[0]);
    }
    return 0;
}
