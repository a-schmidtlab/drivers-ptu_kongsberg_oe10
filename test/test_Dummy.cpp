#include <boost/test/unit_test.hpp>
#include <ptu_kongsberg_oe10/Dummy.hpp>

using namespace ptu_kongsberg_oe10;

BOOST_AUTO_TEST_CASE(it_should_not_crash_when_welcome_is_called)
{
    ptu_kongsberg_oe10::DummyClass dummy;
    dummy.welcome();
}
