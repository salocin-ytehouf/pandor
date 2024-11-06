# IoT Surveillance and Automation System

This project is an educational platform that demonstrates the integration of Z-Wave IoT devices, IP cameras, and AI-based object detection and license plate recognition, orchestrated through a cloud-based API with Kubernetes. The system is designed to teach real-time monitoring, event detection, and automated responses, making it ideal for understanding the principles of microservice, api, AI and video processing, in a practical environment.

## Global Flow

1. **Event Detection (Hardware)**: Z-Wave devices (sensors and switches) and IP cameras monitor for events like motion or state changes.
2. **Data Capture and Transmission**:
   - **Z-Wave Interface** stores sensor data (e.g., battery levels) locally in SQLite.
   - **Video Interface** detects motion, captures images, and sends them to a cloud API for processing.
3. **Cloud Processing (AI and API)**:
   - **AI Modules** apply object detection (YOLOv8), License plate recognition (LPRNet), and face recognition on images.
   - **Cloud Microservices** manage data, scale requests, and handle authentication, utilizing Kubernetes and RabbitMQ for efficiency.
4. **Automated Response**: Based on detection results, the system can trigger alerts, update device statuses, or log data for further analysis.

## Installation and Configuration

To set up the system, refer to individual README files for detailed instructions on each component:

- **[Cloud and API Microservices](cloud/CLOUD_README.md)**: API setup and Kubernetes deployment.
- **[AI Modules](ai/AI_README.md.md)**: Object detection and license plate recognition models.
- **[Hardware Interfaces](hardware/HW_README.md)**: Z-Wave device management and video motion detection.

Follow these guides to deploy locally or in a cloud environment for a complete IoT-based monitoring system.

