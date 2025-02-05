# PTU Kongsberg OE10 Driver

## Overview
This software provides a driver implementation for controlling the Kongsberg OE10 Pan-Tilt Unit (PTU), a precision positioning system commonly used in marine and industrial applications. The driver enables precise control over pan and tilt movements while providing comprehensive status monitoring capabilities.

## Features
- **Pan-Tilt Control**: Full control over pan (horizontal) and tilt (vertical) movements
  - Position control with degree precision
  - Variable speed control (0-100% of maximum speed)
  - Configurable end stops for safety
  - Simple movement commands (tilt up/down)

- **Status Monitoring**
  - Real-time position feedback
  - Speed monitoring
  - Temperature and humidity sensing
  - End stop status

- **Camera Integration**
  - Support for attached camera control
  - Focus and zoom capabilities
  - Auto-focus functionality
  - Manual exposure control
  - Still image capture
  - Auxiliary features (wipers, washer, lamp control, flash)

## Communication Protocol
The driver implements a robust communication protocol with the following features:
- Binary packet-based communication
- Checksum validation for data integrity
- Command acknowledgment system (ACK/NAK)
- Error reporting with detailed status codes
- Support for broadcast commands
- Device ID-based addressing

## Technical Details
- Written in C++ with modern coding practices
- Built on the iodrivers_base framework
- Thread-safe implementation
- Comprehensive error handling
- Extensive documentation using Doxygen
- Unit tests for core functionality

## Usage Example
```cpp
// Initialize the driver
ptu_kongsberg_oe10::Driver driver;
driver.openURI("serial:///dev/ttyUSB0:19200");

// Set pan and tilt positions (in radians)
driver.setPanPosition(0x01, 1.57);  // 90 degrees
driver.setTiltPosition(0x01, 0.785); // 45 degrees

// Get current status
Status status = driver.getStatus(0x01);
std::cout << "Pan: " << status.pan * 180/M_PI << " degrees\n";
std::cout << "Tilt: " << status.tilt * 180/M_PI << " degrees\n";
```

## Directory Structure
NOTE: This directory structure follows some simple rules, to allow for generic build
processes and simplify reuse of this project. 

For build automation the project structure should be parsed and validated.

### STRUCTURE
- **src/**
	Contains all header (*.h/*.hpp) and source files
- **build/**
	The target directory for the build process, temporary content
- **bindings/**
	Language bindings for this package, e.g. put into subfolders such as
   - **ruby/**
        Ruby language bindings
- **viz/**
        Source files for a vizkit plugin / widget related to this library 
- **resources/**
	General resources such as images that are needed by the program
- **configuration/**
	Configuration files for running the program
- **external/**
	When including software that needs a non standard installation process, or one that can be
	easily embedded include the external software directly here
- **doc/**
	should contain the existing doxygen file: doxygen.conf 