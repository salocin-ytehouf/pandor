# hardware overview
# zwave_interface

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



# video_interface

This project is a multiworker C++ application designed to detect motion from IP camera network feed. When motion is detected, the application captures an image and pushes it to an API for further processing using a deep neural network. It's a abstraction layer between cv2 protocol and endpoint API for video stremaprocessing 


it canr run on a RPI and grab video stream from IP protocol as RSTP

## Features

- **Motion Detection**: Continuously monitors the IP camera feed for any motion using computer vision algorithms.
- **Image Capture**: Captures an image from the camera feed when motion is detected.
- **API Integration**: Pushes the captured image to an API endpoint for processing using a deep neural network.
- **Multiworker Architecture**: Utilizes a multiworker architecture to handle simultaneous motion detection and image processing tasks efficiently.

## Dependencies

Before building and running the application, ensure you have the following dependencies installed:

- OpenCV: Required for capturing and processing camera images.
- libcurl: Used for making HTTP requests to the API endpoint.

Ensure these dependencies are installed on your system before proceeding.

## Configuration

The configuration file (`config.json`) contains settings for camera configurations, database connection details, and worker credentials.

### Cameras

The `cameras` section defines the configuration for each camera. Each camera object includes the following fields:

- `id`: Unique identifier for the camera.
- `name`: Name of the camera.
- `location`: Location where the camera is installed.
- `protocol`: Protocol used to access the camera (e.g., V4L).
- `port`: Port number for the camera connection.
- `username`: Username for accessing the camera (if authentication is required).
- `password`: Password for accessing the camera (if authentication is required).
- `fps`: Frames per second for capturing images from the camera.
- `rgb_feed`: Boolean indicating whether the camera provides RGB feed.
- `ir_feed`: Boolean indicating whether the camera provides infrared feed.
- `battery`: Boolean indicating whether the camera is battery-powered.
- `final_address`: Final address or device path for accessing the camera. 
- `is_door`: Boolean indicating whether the camera is installed at a door.

### Database

The `database` section defines the connection details and node information for the database. Fields include:

- `NODE_ID`: Node ID for the database connection.
- `NODE_BATTERY`: Battery status of the node.
- `LAST_BATTERY_TS`: Timestamp for the last battery update.
- `LAST_STATE_TS`: Timestamp for the last state update.

### Worker

The `worker` section contains credentials for the worker accessing the API. Fields include:

- `worker_id`: Unique identifier for the worker.
- `username`: Username for API authentication.
- `password`: Password for API authentication.

Modify the `config.json` file to configure the application according to your setup.

## Building the Application

Follow these steps to build the application:

1. **Clone the Repository**: Clone this repository to your local machine.

2. **Install Dependencies**: Make sure you have OpenCV and libcurl installed on your system.

3. **Build the Project**: Navigate to the project directory and build the application using CMake:

4. **Configure API Endpoint**: Edit the configuration file to specify the URL of the API endpoint where images will be pushed for processing. `hardware/video_interface/video_config.json` yo can try with webcam path

5. **Run the Application**: Execute the compiled binary to start the application:

6. **Verify Operation**: Ensure that the application successfully detects motion, captures images, and pushes them to the API for processing.

## Usage

The application runs continuously once started. It monitors the IP camera feed for motion and triggers the image capture and API push process when motion is detected.

## Contributing

Contributions to this project are welcome! If you'd like to contribute, fork the repository, make your changes, and submit a pull request. Bug reports and feature requests can also be submitted via GitHub issues.

## License

This project is licensed under the [MIT License](LICENSE).
