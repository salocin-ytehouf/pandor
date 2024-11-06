# Z-Wave Network Project Overview

## Introduction

This project aims to implement a Z-Wave network using a Z-Wave bridge controller and Z-Ware SDK C API. The network includes binary sensors and binary switch controllers. The primary functionalities of the project are facilitated through an application called `controller_app`, which serves as a command-line interface (CLI). The `hw_int_sen` component, short for hardware interface sensor, is responsible for interfacing with hardware to retrieve values from binary switches and sensor battery levels. It also stores this information in a SQLite database. The project is designed to run on a Raspberry Pi (RPI) with a Z-Wave USB stick device and has the capability to handle up to 255 Z-Wave devices.

## Components

### 1. `controller_app`

- **Purpose**: The `controller_app` is a command-line application designed to manage the Z-Wave network. It allows users to add or remove new nodes/sensors within the network and conduct health tests.
- **Files**:
  - `cmd_class.cfg`: Configuration file for command classes.
  - `controller_app`: Executable file.
  - `Makefile`: Makefile for building the application.
  - `nifFF832C74.jsn`: JSON zwave network config file.
  - `zwave_device_rec.txt`: Text file containing Z-Wave device records.

### 2. `hw_int_sen`

- **Purpose**: The `hw_int_sen` component serves as the hardware interface for sensors. It implements callbacks to retrieve values from binary switches and sensor battery levels. Additionally, it stores this information in a SQLite database.
- **Files**:
  - `cmd_class.cfg`: Configuration file for command classes.
  - `controller_app.cfg`: Configuration file for the controller application.
  - `hw_int_sen`: Executable file.
  - `Makefile`: Makefile for building the hardware interface sensor.
  - `nifFF832C74.jsn`: JSON zwave network config file.
  - `pandore.db`: SQLite database for storing sensor data.
  - `zwave_device_rec.txt`: Text file containing Z-Wave device records.

### 3. `hw_zwave_doc`

- **Purpose**: This directory contains documentation related to the Z-Wave SDK and its usage.
- **Files**:
  - `INS14416-19 - Z-Ware SDK 7.18.x Library C API Reference Manual.pdf`: Reference manual for the Z-Ware SDK C API.
  - `INS14428-22 - Z-Ware SDK 7.18.x Web User Guide.pdf`: User guide for the Z-Ware SDK web interface.
  - `INS14430-14 - Z-Ware SDK 7.18.x Web Developer Guide.pdf`: Developer guide for the Z-Ware SDK web interface.
  - `INS14486-15 - Z-Ware 7.18.x Web Server Installation Guide.pdf`: Installation guide for the Z-Ware web server.
  - `INS14606-13 - Z-Ware SDK 7.18.x Library User Guide.pdf`: User guide for the Z-Ware SDK library.



This project is intended to provide a robust Z-Wave network solution with support for various functionalities such as adding/removing nodes, conducting health tests, and managing sensor data. Still under development 
