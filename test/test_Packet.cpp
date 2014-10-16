#include <boost/test/unit_test.hpp>
#include <ptu_kongsberg_oe10/Packet.hpp>

using namespace std;
using namespace ptu_kongsberg_oe10;

BOOST_AUTO_TEST_CASE(it_should_validate_the_marshalled_data)
{
    Packet packet(1, 2);
    packet.setCommand('T', 'E');
    packet.data_size = 3;
    packet.data[0] = 'S';
    packet.data[1] = 'T';
    packet.data[2] = '1';

    vector<byte> buffer;
    packet.marshal(buffer);

    BOOST_REQUIRE_EQUAL(buffer.size(), Packet::extractPacket(&buffer[0], buffer.size()));

    Packet result = Packet::parse(&buffer[0], buffer.size());
    BOOST_REQUIRE_EQUAL(1, result.to);
    BOOST_REQUIRE_EQUAL(2, result.from);
    BOOST_REQUIRE_EQUAL(3, result.data_size);
    BOOST_REQUIRE_EQUAL(static_cast<byte>('S'), result.data[0]);
    BOOST_REQUIRE_EQUAL(static_cast<byte>('T'), result.data[1]);
    BOOST_REQUIRE_EQUAL(static_cast<byte>('1'), result.data[2]);
    BOOST_REQUIRE_EQUAL("TE", result.getCommandAsString());
}

BOOST_AUTO_TEST_CASE(Packet_checksum_encoding_normal)
{
    byte checksum = 0x10;
    byte buffer[3];
    byte expected[3] = { 0x10, ':', 'G' };
    Packet::marshalChecksum(checksum, buffer);
    BOOST_REQUIRE_EQUAL(expected[0], buffer[0]);
    BOOST_REQUIRE_EQUAL(expected[1], buffer[1]);
    BOOST_REQUIRE_EQUAL(expected[2], buffer[2]);
    BOOST_REQUIRE(Packet::compareChecksum(checksum, buffer));
}

BOOST_AUTO_TEST_CASE(Packet_checksum_encoding_0x3C)
{
    byte checksum = 0x3C;
    byte buffer[3];
    byte expected[3] = { 0xFF, ':', '0' };
    Packet::marshalChecksum(checksum, buffer);
    BOOST_REQUIRE_EQUAL(expected[0], buffer[0]);
    BOOST_REQUIRE_EQUAL(expected[1], buffer[1]);
    BOOST_REQUIRE_EQUAL(expected[2], buffer[2]);
    BOOST_REQUIRE(Packet::compareChecksum(checksum, buffer));
}

BOOST_AUTO_TEST_CASE(Packet_checksum_encoding_0x3E)
{
    byte checksum = 0x3E;
    byte buffer[3];
    byte expected[3] = { 0xFF, ':', '1' };
    Packet::marshalChecksum(checksum, buffer);
    BOOST_REQUIRE_EQUAL(expected[0], buffer[0]);
    BOOST_REQUIRE_EQUAL(expected[1], buffer[1]);
    BOOST_REQUIRE_EQUAL(expected[2], buffer[2]);
    BOOST_REQUIRE(Packet::compareChecksum(checksum, buffer));
}

BOOST_AUTO_TEST_CASE(Packet_encodeAngle)
{
    int degrees = 245;
    float radians = degrees * M_PI / 180;
    byte buffer[3];
    byte expected[3] = { '2', '4', '5' };
    Packet::encodeAngle(buffer, radians);
    BOOST_REQUIRE_EQUAL(expected[0], buffer[0]);
    BOOST_REQUIRE_EQUAL(expected[1], buffer[1]);
    BOOST_REQUIRE_EQUAL(expected[2], buffer[2]);
}

