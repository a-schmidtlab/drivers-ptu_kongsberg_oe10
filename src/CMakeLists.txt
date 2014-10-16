rock_library(ptu_kongsberg_oe10
    SOURCES Packet.cpp Driver.cpp
    HEADERS Packet.hpp Driver.hpp Status.hpp PanTiltStatus.hpp
    DEPS_PKGCONFIG base-types base-lib iodrivers_base)

rock_executable(ptu_kongsberg_oe10_bin Main.cpp
    DEPS ptu_kongsberg_oe10)

