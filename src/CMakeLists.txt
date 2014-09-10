rock_library(ptu_kongsberg_oe10
    SOURCES Dummy.cpp
    HEADERS Dummy.hpp)

rock_executable(ptu_kongsberg_oe10_bin Main.cpp
    DEPS ptu_kongsberg_oe10)

