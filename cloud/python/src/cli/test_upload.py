import cv2
import time
import os
import gridfs
import pika
import json
import traceback
from bson import ObjectId
from flask_pymongo import PyMongo
from flask import Flask

# Initialize Flask app and MongoDB connection
server = Flask(__name__)
mongo_images = PyMongo(server, uri="mongodb://localhost:27017/images")
fs = gridfs.GridFS(mongo_images.db)

# Load image data using OpenCV
image1 = cv2.imread('./data/1.jpeg')
image2 = cv2.imread('./data/2.jpeg')

current_timestamp = int(time.time())

# Prepare metadata for each image
image1_metadata = {
    "worker_id": 'aaaaaa',
    "timestamp": current_timestamp,
    "camera_id": "camera1",
    "is_door": True  # Change to a boolean value
}

image2_metadata = {
    "worker_id": 'aaaaaa',
    "timestamp": current_timestamp,
    "camera_id": "camera1",
    "is_door": True  # Change to a boolean value
}

# Convert image data to a list (nested list)
image1_data_list = image1.tolist()
image2_data_list = image2.tolist()

# Prepare the payload
payload = {
    "image1": {
        "data": image1_data_list,
        "metadata": image1_metadata
    },
    "image2": {
        "data": image2_data_list,
        "metadata": image2_metadata
    }
}

payload_json = json.dumps(payload)

try:
    print('before fs.put()', flush=True)
    
    metadata = payload["image1"]["metadata"]
    image_data_list = payload["image1"]["data"]
    
    # Encode the image data as a JSON string
    image_data_json = json.dumps(image_data_list)
    
    # Store the encoded image data in GridFS
    fid = fs.put(image_data_json.encode('utf-8'), **metadata)
    
    print('after fs.put()', flush=True)
    
except Exception as err:
    traceback.print_exc()
    print("Internal server error: MongoDB image not uploaded, retry 500")
