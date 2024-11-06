# Multiworker Motion Detection and Image Processing

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
