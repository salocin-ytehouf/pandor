# Microservice Architecture and System Design with Python, Kubernetes and Pytorch

This repository serves as a comprehensive hands-on tutorial, emphasizing microservices architecture and distributed systems. It leverages Python, Kubernetes, RabbitMQ, MongoDB, and MySQL, alongside cutting-edge deep learning algorithms implemented in PyTorch for advanced object detection and license plate recognition tasks.

## Overview
The project implements an API microservice architecture in Python to manage image processing tasks originating from a video stream. It incorporates a hardware video interface component for seamless interaction with the video source. When motion is detected, the client sends an image payload along with a JWT token to the gateway service. Subsequently, the authentication microservice validates the client's authorization status. If authorized, the image is forwarded to a detector microservice responsible for tasks such as car, license plate, and face detection (see AI detector folder for details).

Detected license plates or faces undergo further processing by specialized services: either the face descriptor service, which utilizes the face_recognition library, or the car descriptor service implementing LPRNET for optical character recognition (OCR). These descriptor services extract ID information and verify authorization in the database for people or vehicles.

By combining Python-based microservices with Kubernetes orchestration and integrating RabbitMQ for Pub/Sub messaging, the project demonstrates a robust and scalable solution for handling complex image processing tasks in real-world scenarios.

Feel free to explore the project's codebase and tutorials to gain insights into microservices architecture, system design, and distributed systems implementation using modern technologies.

## Technology Stack

- **Python**: Powering the API microservices and image processing functions.
- **Docker**: Containerizing microservices for streamlined deployment and scalability.
- **Kubernetes/Minikube**: Orchestrating container deployment and management, ensuring scalability, and simplifying configuration and monitoring.
- **AI detector folder**: Hosting modules for training and testing object detection tasks like car, license plate, and face detection. contain also a link to pretrained models (OCR are trained for earopean licence plate)
- **HW detector folder**: Hosting modules for HW part, the most important is the video_interface folder if you want to run multiple video stram simultaneously
## Dependencies and Installation

Ensure all necessary dependencies, including Docker, Kubernetes/Minikube, and others, are installed following the provided instructions before proceeding.

### Docker

To install Docker, follow the steps below:

1. Update the package index:

    ```bash
    sudo apt-get update
    ```

2. Install dependencies:

    ```bash
    sudo apt-get install ca-certificates curl gnupg
    ```

3. Set up Docker's official GPG key:

    ```bash
    sudo install -m 0755 -d /etc/apt/keyrings
    curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg
    sudo chmod a+r /etc/apt/keyrings/docker.gpg
    ```

4. Add the Docker repository to your system:

    ```bash
    echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu $(. /etc/os-release && echo "$VERSION_CODENAME") stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
    ```

5. Update the package index again:

    ```bash
    sudo apt-get update
    ```

6. Install Docker:

    ```bash
    sudo apt-get install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
    ```

7. Verify the installation:

    ```bash
    sudo docker run hello-world
    ```
8. Note:

you will need a Docker Hub account. Docker Hub serves as a centralized repository for Docker images, which Kubernetes can pull and deploy onto the cluster nodes. By storing Docker images on Docker Hub, users can streamline the process of downloading and running containers within their Kubernetes environments. This seamless integration between Docker Hub and Kubernetes simplifies the deployment and management of containerized applications, allowing users to leverage the full potential of Kubernetes' orchestration capabilities.

### Kubernetes

To install Kubernetes, follow these steps:

1. Install prerequisites:

    ```bash
    sudo apt-get install -y ca-certificates curl apt-transport-https
    ```

2. Add the Kubernetes GPG key:

    ```bash
    sudo curl -fsSLo /etc/apt/keyrings/kubernetes-archive-keyring.gpg https://packages.cloud.google.com/apt/doc/apt-key.gpg
    ```

3. Add the Kubernetes repository:

    ```bash
    echo "deb [signed-by=/etc/apt/keyrings/kubernetes-archive-keyring.gpg] https://apt.kubernetes.io/ kubernetes-xenial main" | sudo tee /etc/apt/sources.list.d/kubernetes.list
    ```

4. Update the package index:

    ```bash
    sudo apt-get update
    ```

5. Install `kubectl`:

    ```bash
    sudo apt-get install -y kubectl
    ```

6. Verify the installation:

    ```bash
    kubectl version --client --output=yaml
    ```

### Minikube

To install Minikube, follow these steps:

1. Download Minikube:

    ```bash
    curl -LO https://storage.googleapis.com/minikube/releases/latest/minikube-linux-amd64
    ```

2. Add execute permission:

    ```bash
    sudo install minikube-linux-amd64 /usr/local/bin/minikube
    ```

3. Start Minikube with Docker:

    ```bash
    minikube start --driver=docker
    ```
### K9S installation 

#### Via LinuxBrew (tested)
   ```bash
 brew install derailed/k9s/k9s
 ```
#### Via PacMan
  ```bash
 pacman -S k9s
 ```
## DB 

## Connecting Python Microservices to Databases

One of the main challenges users may encounter is connecting Python microservices running in Kubernetes to MySQL and MongoDB databases that are hosted on their local computers outside the cluster. To address this issue, it's crucial to check and update your database and firewall configurations as needed.

### Database Configuration

Ensure that your MySQL and MongoDB databases are configured to allow external connections. This may involve updating the database configuration files to allow connections from external IP addresses or configuring network settings to permit access.

### Firewall Configuration

Check your firewall settings to ensure that inbound connections to the MySQL and MongoDB ports are allowed. You may need to add rules to your firewall configuration to permit traffic on the respective database ports.

### Troubleshooting

If you encounter issues with connectivity, such as connection timeouts or refused connections, double-check your database and firewall configurations to ensure that they are correctly configured to allow external access. Additionally, verify that the Kubernetes cluster has network connectivity to your local computer.

By addressing these configuration aspects, you can facilitate the communication between your Python microservices running in Kubernetes and your MySQL and MongoDB databases hosted locally.

### MySQL Installation

To install MySQL on Ubuntu 20.04, you can follow the steps outlined below:

1. **Install MySQL Server:**

   Once the package index is updated, you can install the MySQL server package:

    ```bash
    sudo apt install mysql-server
    ```

2. **Access/test MySQL:**

   After the installation is complete, you can access the MySQL database server. Use the following command to log in with the specified user (`pandor_adm` in this case):

    ```bash
    mysql -h localhost -uroot -p
    ```

   You will be prompted to enter, your sudo and then the mysql root password. After entering the password, you will have access to the MySQL server.

### MySQL Database Setup

To set up the MySQL database for the project, follow these steps:

1. **Create Database and User:**

    Execute the SQL commands in the `pandor_init.sql` script to create the necessary database and user:

    ```bash
    sudo mysql -uroot -p < pandor_init.sql
    ```

2. **Create Tables for User Management:**

    After executing the script, the database `pandor_user` will be created along with the user `pandor_adm`. This script also creates the following tables:

    - `user_roles`: Contains roles for users.
    - `users`: Stores user information.

```
Table: user_roles
  role_id           -      int
  role_name         -      varchar(50)

Table: users
  user_id           -      int
  username          -      varchar(50)
  email             -      varchar(100)
  password          -      varchar(255)
  salt              -      varchar(255)
  role_id           -      int
  registration_date -      datetime
  last_login_date   -      datetime

```

3. **Create Tables for Face and Plate Authorization:**

    Additionally, execute the SQL commands in the `pandor_plate_face_init.sql` script to create tables for face and plate authorization:

    ```bash
    sudo mysql -uroot -p < pandor_plate_face_init.sql
    ```

    This script creates the following tables:

    - `authorized_faces`: Stores authorized faces.
    - `authorized_plates`: Stores authorized plates.

### MongoDB Installation

To install MongoDB on Ubuntu, follow these steps:

1. **Add MongoDB Repository:**

    MongoDB is not included in the default Ubuntu repositories. Add the MongoDB repository to your system:

    ```bash
    wget -qO - https://www.mongodb.org/static/pgp/server-4.4.asc | sudo apt-key add -
    ```

2. **Create a list file for MongoDB:**

    ```bash
    echo "deb [ arch=amd64,arm64 ] https://repo.mongodb.org/apt/ubuntu $(lsb_release -cs)/mongodb-org/4.4 multiverse" | sudo tee /etc/apt/sources.list.d/mongodb-org-4.4.list
    ```

3. **Update the Package Index:**

    ```bash
    sudo apt update
    ```

4. **Install MongoDB:**

    ```bash
    sudo apt install -y mongodb-org
    ```

5. **Start MongoDB Service:**

    ```bash
    sudo systemctl start mongod
    ```

6. **Verify MongoDB Installation:**

    Check the status of MongoDB service to ensure it's running:

    ```bash
    sudo systemctl status mongod
    ```
### MongoDB Database Description

In this project, MongoDB is utilized to store various types of data related to plate and face detection, authorized users, images, and detector results. Below is a description of each collection and its structure:

#### res plate detected

- **Database:** `plates`
- **Collection:** `plates`

result if plate is detected or known for a specific user and camera
Example Document:

```json
{
  "_id": ObjectId("6523f164a0146faa9e93cc00"), // source id_img where the car licence plate was detected 
  "camera_id": "camera2",                     
  "worker_id": "nico@gmail.com",
  "plate_id": "MA078EY", // plate number detected       
  "ts": 1696854372,// timestamp
  "res": {
    "bbox": [707, 579, 920, 635], // localization insid img
    "confidence": 0.8330032825469971, // confidence score 
    "label": "plate" 
  },
  "auth_status": true // the plate is authorized or not
}
```
### db authorized plate
- **Database:** `auth_plate`
- **Collection:** `users`

list of authorized car licence plate per user, per camera 
- **Example Document:**
```json
{
  "_id": ObjectId("6521452d064279b72f4e8d49"),
  "adm_id": "nico",
  "plate_id": "TV503TH",
  "user_id": "test",
  "worker_id": "test",
  "ts": "1696679213.35711"
}
```

### db authorized face
- **Database:** `auth_face`
- **Collection:** `users`

list of authorized people face per user, per camera 

- **Example Document:**
```json
{
  "_id": ObjectId("652143de6b8a9d9a5ca284f0"),
  "adm_id": "nico",
  "encoding": [-0.13191625475883484, -0.01914270967245102, 0.09889524430036545, ...], 
  "user_id": "nico",
  "worker_id": "nico",
  "ts": "1696678878.778207"
}
```
### db image stored if movement detected
- **Database:** `images`
- **Collections:** `fs.files`, `fs.chunks`
- **Example Document:**
```json
{
  "_id": ObjectId("651d369debfe93a937f14a62"),
  "worker_id": "aaaaaa",
  "timestamp": 1696413341,
  "camera_id": "camera1",
  "chunkSize": 261120,
  "length": NumberLong(252864),
  "uploadDate": ISODate("2023-10-04T09:55:41.405Z")
}
```

### db detector store res yolo

Storing results  of YOLO detector if 
- **Database:** `detector`
- **Collection:** `res`
- **Example Document:**
```json
{
  "_id": ObjectId("6523ca938f3519a220f9240c"),
  "0": {
    "bbox": [281, 198, 1090, 702],
    "confidence": 0.9459878206253052,
    "label": "car"
  },
  "1": {
    "bbox": [707, 579, 920, 635],
    "confidence": 0.8330032825469971,
    "label": "plate"
  },
  ....
  ....
  ....
  "image_fid": "6523ca9385c10c7ff51889b4",
  "worker_id": "nico@gmail.com",
  "camera_id": "camera2",
  "ts": 1696844435
}
```

These collections store diverse data crucial for various functionalities of the project. 


# Communication Between Microservices

When building a microservices architecture, communication between services is crucial for the system to function effectively. There are primarily two methods of communication commonly used:

## 1. HTTP Communication

HTTP (Hypertext Transfer Protocol) communication involves services interacting with each other over the network via HTTP requests and responses. This approach is widely used and facilitates synchronous communication between services. It's suitable for scenarios where services require immediate responses and when the interactions are relatively simple and stateless.

## 2. Publish-Subscribe (Pub/Sub) Communication

Pub/Sub communication involves services exchanging messages through a message broker or a message queue. In this model, publishers send messages to specific topics, and subscribers receive messages from those topics. Pub/Sub communication enables asynchronous communication between services, making it suitable for scenarios where services need to decouple their interactions and handle large volumes of messages. It's often used in event-driven architectures and scenarios where services operate independently and asynchronously.

The choice between HTTP and Pub/Sub communication depends on various factors such as the nature of the interactions, latency requirements, scalability needs, and system architecture considerations.

# Communication Between Microservices

When building a microservices architecture, effective communication between services is essential. There are two primary methods of communication commonly used:

## 1. HTTP Communication

HTTP (Hypertext Transfer Protocol) communication involves services interacting over the network via HTTP requests and responses. This approach enables synchronous communication between services, making it suitable for scenarios where immediate responses are required, such as user authentication and authorization checks. HTTP communication is preferred for relatively simple and stateless interactions.

## 2. Publish-Subscribe (Pub/Sub) Communication

Pub/Sub communication involves services exchanging messages through a message broker or queue. Publishers send messages to specific topics, and subscribers receive messages from those topics asynchronously. This approach facilitates decoupled interactions between services and is ideal for handling large volumes of messages efficiently. Pub/Sub communication is commonly used in event-driven architectures and scenarios where services need to operate independently and asynchronously.

The choice between HTTP and Pub/Sub communication depends on factors such as the nature of interactions, latency requirements, scalability needs, and system architecture considerations.

## Communication Patterns in the Microservices Architecture

In our microservices architecture, we employ a combination of HTTP and Pub/Sub communication patterns to facilitate interactions between services:

### HTTP Communication

- **Gateway** and **authentication microservices**: These services utilize HTTP communication for synchronous request-response exchanges over the network. This pattern is suitable for immediate responses required during user authentication and authorization checks.

### Pub/Sub Communication

- **Gateway** and **detector microservices**: These services communicate with the **car_descriptor** and **face_descriptor** services using a Pub/Sub communication pattern via RabbitMQ. Pub/Sub communication enables asynchronous message exchange, with publishers sending messages to specific topics and subscribers receiving and processing messages independently. This pattern is ideal for decoupling services and efficiently handling large volumes of messages. In this project, the Pub/Sub service is implemented using RabbitMQ. However, in real-world applications, another popular solution for Pub/Sub messaging is Kafka.

### RabbitMQ Message and Queues

#### Message Structure for Image Processing

```json
{
  "image_fid": "str(fid)",
  "worker_id": "access['username']",
  "ts": "metadata['timestamp']",
  "camera_id": "metadata['camera_id']",
  "is_door": "metadata['is_door']",
  "nb_attempt": 0
}
```


#### Message Structure for Face Detection and Plate Detection

```json

Copy code
{
  "image_fid": "str(fid)",
  "worker_id": "access['username']",
  "ts": "metadata['timestamp']",
  "camera_id": "metadata['camera_id']",
  "is_door": "metadata['is_door']",
  "nb_attempt": 0,
  "res": {
    "bbox": [x1, y1, x2, y2],
    "confidence": ...,
    "label": "label"
  }
}
```

## Storing Images in the Database

In the current implementation, images are stored directly in MongoDB using GridFS, which is suitable for small-scale applications or development environments. However, in a real-world scenario or production environment, it's recommended to store images in a dedicated storage service like Amazon S3 (Simple Storage Service) and store only the path or URL to the image in the database.

Storing images in a dedicated storage service offers several advantages:

1. **Scalability**: Dedicated storage services like Amazon S3 are designed to handle large amounts of data and can scale seamlessly as the application grows.

2. **Performance**: These services are optimized for serving static assets like images, ensuring fast and reliable access to the stored images.

3. **Cost-Effectiveness**: By separating storage from the database, you can optimize costs by choosing storage solutions that offer cost-effective pricing models based on usage.

4. **Security and Reliability**: Storage services like S3 provide built-in features for data encryption, access control, and data durability, ensuring the security and reliability of stored images.

By storing only the path or URL to the image in the database, you keep the database size manageable and reduce the overhead of storing and retrieving large binary data. This approach also allows for easier management and backup of image files, as they are stored separately from the database.


# Global Microservice Architecture

Building the entire architecture, including building, pushing, and then pulling Docker images through Kubernetes, can be complex and challenging, especially if you don't have prior knowledge of these technologies. If you find yourself struggling, I highly recommend first following this excellent  [tutorial](https://www.youtube.com/watch?v=hmkF77F9TLw&t=6649s&ab_channel=freeCodeCamp.org)on API and microservices in Python . It provides a comprehensive overview and step-by-step guidance, which can help you understand the concepts and processes more effectively.

this [tutorial](https://www.youtube.com/watch?v=X48VuDVv0do&t=4207s&ab_channel=TechWorldwithNana) is also very interesting. 
## Running Kubernetes Cluster
To set up a Kubernetes cluster, you'll need to edit the `/etc/hosts` file using your preferred text editor. Here's how you can do it:

1. Open the `/etc/hosts` file:
    ```
    sudo nano /etc/hosts
    ```

2. Add the following loopback addresses:
    ```
    192.168.49.2  pandor.com
    192.168.49.2  rabbitmq-manager.com
    ```

These loopback addresses should point to the external Kubernetes node running or your local machine. 
- `pandor.com` is used to access the API.
- `rabbitmq-manager.com` is the endpoint for the RabbitMQ service's GUI. This endpoint is useful for debugging and defining the queues used in the system.

# run minikube
open a terminal
start minikube
```
$ minikube start
😄  minikube v1.31.1 on Ubuntu 20.04
🎉  minikube 1.32.0 is available! Download it: https://github.com/kubernetes/minikube/releases/tag/v1.32.0
💡  To disable this notice, run: 'minikube config set WantUpdateNotification false'

✨  Using the docker driver based on existing profile
👍  Starting control plane node minikube in cluster minikube
🚜  Pulling base image ...
🤷  docker "minikube" container is missing, will recreate.
🔥  Creating docker container (CPUs=2, Memory=3900MB) ...
🐳  Preparing Kubernetes v1.27.3 on Docker 24.0.4 ...
🔗  Configuring bridge CNI (Container Networking Interface) ...
🔎  Verifying Kubernetes components...
    ▪ Using image gcr.io/k8s-minikube/storage-provisioner:v5
    ▪ Using image registry.k8s.io/ingress-nginx/controller:v1.8.1
    ▪ Using image registry.k8s.io/ingress-nginx/kube-webhook-certgen:v20230407
    ▪ Using image registry.k8s.io/ingress-nginx/kube-webhook-certgen:v20230407
🔎  Verifying ingress addon...
🌟  Enabled addons: storage-provisioner, default-storageclass, ingress
🏄  Done! kubectl is now configured to use "minikube" cluster and "default" namespace by default
```
in an other terminal 

enable ingess inbound 
```
$ minikube addons enable ingress
💡  ingress is an addon maintained by Kubernetes. For any concerns contact minikube on GitHub.
You can view the list of minikube maintainers at: https://github.com/kubernetes/minikube/blob/master/OWNERS
    ▪ Using image registry.k8s.io/ingress-nginx/kube-webhook-certgen:v20230407
    ▪ Using image registry.k8s.io/ingress-nginx/kube-webhook-certgen:v20230407
    ▪ Using image registry.k8s.io/ingress-nginx/controller:v1.8.1
🔎  Verifying ingress addon...
🌟  The 'ingress' addon is enabled

```
show ingress list 
```
$ minikube addons list
|-----------------------------|----------|--------------|--------------------------------|
|         ADDON NAME          | PROFILE  |    STATUS    |           MAINTAINER           |
|-----------------------------|----------|--------------|--------------------------------|
| ambassador                  | minikube | disabled     | 3rd party (Ambassador)         |
| auto-pause                  | minikube | disabled     | minikube                       |
| cloud-spanner               | minikube | disabled     | Google                         |
| csi-hostpath-driver         | minikube | disabled     | Kubernetes                     |
| dashboard                   | minikube | disabled     | Kubernetes                     |
| default-storageclass        | minikube | enabled ✅   | Kubernetes                     |
| efk                         | minikube | disabled     | 3rd party (Elastic)            |
| freshpod                    | minikube | disabled     | Google                         |
| gcp-auth                    | minikube | disabled     | Google                         |
| gvisor                      | minikube | disabled     | minikube                       |
| headlamp                    | minikube | disabled     | 3rd party (kinvolk.io)         |
| helm-tiller                 | minikube | disabled     | 3rd party (Helm)               |
| inaccel                     | minikube | disabled     | 3rd party (InAccel             |
|                             |          |              | [info@inaccel.com])            |
| ingress                     | minikube | enabled ✅   | Kubernetes                     |
| ingress-dns                 | minikube | disabled     | minikube                       |
| inspektor-gadget            | minikube | disabled     | 3rd party                      |
|                             |          |              | (inspektor-gadget.io)          |
| istio                       | minikube | disabled     | 3rd party (Istio)              |
| istio-provisioner           | minikube | disabled     | 3rd party (Istio)              |
| kong                        | minikube | disabled     | 3rd party (Kong HQ)            |
| kubevirt                    | minikube | disabled     | 3rd party (KubeVirt)           |
| logviewer                   | minikube | disabled     | 3rd party (unknown)            |
| metallb                     | minikube | disabled     | 3rd party (MetalLB)            |
| metrics-server              | minikube | disabled     | Kubernetes                     |
| nvidia-driver-installer     | minikube | disabled     | 3rd party (Nvidia)             |
| nvidia-gpu-device-plugin    | minikube | disabled     | 3rd party (Nvidia)             |
| olm                         | minikube | disabled     | 3rd party (Operator Framework) |
| pod-security-policy         | minikube | disabled     | 3rd party (unknown)            |
| portainer                   | minikube | disabled     | 3rd party (Portainer.io)       |
| registry                    | minikube | disabled     | minikube                       |
| registry-aliases            | minikube | disabled     | 3rd party (unknown)            |
| registry-creds              | minikube | disabled     | 3rd party (UPMC Enterprises)   |
| storage-provisioner         | minikube | enabled ✅   | minikube                       |
| storage-provisioner-gluster | minikube | disabled     | 3rd party (Gluster)            |
| volumesnapshots             | minikube | disabled     | Kubernetes                     |
|-----------------------------|----------|--------------|--------------------------------|
```

```
$ minikube tunnel 
[sudo] password for nico: 
Status:	
	machine: minikube
	pid: 326340
	route: 10.96.0.0/12 -> 192.168.49.2
	minikube: Running
	services: []
    errors: 
		minikube: no errors
		router: no errors
		loadbalancer emulator: no errors

```
the ip adreeses you edit in the host file should match the outbound routing ip adress of the minikube cluster 

open a third terminal and run k9s
`add screen shot`

# Deploying Microservices

To deploy the six microservices, you need to build, tag, and push six Docker images to Docker Hub. Once the images are available in your Docker repository, you can use the following commands to apply the deployment manifests for each microservice:

## Building and Pushing Images:

1. Build and tag Docker images for each microservice.
2. Push the images to your Docker Hub repository.

## Applying Deployment Manifests:

Use the `kubectl apply -f` command to apply the deployment manifests for each microservice. Here are the commands:

```bash
kubectl apply -f ./python/src/gateway/manifest/
kubectl apply -f ./python/src/auth/manifest/
kubectl apply -f ./python/src/rabbit/manifest/
```

At this point, you need to configure the RabbitMQ microservice, you need to set up three queues using the GUI.

1. Open your web browser and navigate to [rabbitmq-manager.com](http://rabbitmq-manager.com).
2. Use the default username and password to log in:
   - Username: guest
   - Password: guest
3. Create three queues with the following names:
   - image
   - faces
   - plates

Then launch the last three microservices  
```bash
kubectl apply -f ./python/src/gateway/manifest/
kubectl apply -f ./python/src/auth/manifest/
kubectl apply -f ./python/src/rabbit/manifest/
```

# Modifying Deployment Manifests (Optional)

If you prefer to use pre-built Docker images, modify the deployment manifests for each microservice folders (except for rabbit, we use the default rabbit-mq image from their official repo) to point to a public Docker repository.

Example for the auth microservice (./auth/manifest/auth-deploy.yaml):

```yaml
spec:
  replicas: 1
  selector:
    matchLabels:
      app: auth
  strategy:
    type: RollingUpdate
    rollingUpdate:
      maxSurge: 3
  template:
    metadata:
      labels:
        app: auth
    spec:
      containers:
        - name: auth
          image: nicolas457/auth   # Change this line to point to your Docker image URL https://hub.docker.com/repository/docker/nicolas457/auth
          ports:
            - containerPort: 5000
          envFrom:
            - configMapRef:
                name: auth-configmap
            - secretRef:
                name: auth-secret
```

By following these instructions, you can deploy the microservices to your Kubernetes cluster either using your custom Docker images or by referencing pre-built images from a public Docker repository.

# Testing the full API using CLI interface 

change directory to the cli folder 

you'll need to edit the configuration file located at `./python/src/cli/config.ini`. Here's the content you need to edit:

```ini
[db_mysql]
host = localhost
user = pandor_adm
password = Z4TsHC?uE0ce,o!
database = pandor_user

[db_mongo]
host = none
user = none
password = none

[api]
url = http://pandor.com
username = nico@gmail.com
password = 1234
```
the MongoDB fields in the configuration file are set to none because I did not set up a password during the MongoDB installation process. 

activate venv 
source venve/bin/activate 
python cli.py 

first menu level 
```
Choose option you want to use : 

a: User sub menu
b: DB sub menu
c: faces and plates sub menu
d: image test sub menu
q: Exit

Option>> 

```
first you need to create your first user 
then create a jwt for this user 
 to streamline debugging and testing the config.ini file allow you to pass in api section one username and one password to automaticly create a valid JWT for this user 

 Jwt has a temporal valid time check auth server.py microservice to set to the desire time 
             "exp": datetime.datetime.now(tz=datetime.timezone.utc) # time of validity of the token to access services
            + datetime.timedelta(days=1), 