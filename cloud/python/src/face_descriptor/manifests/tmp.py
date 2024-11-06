from pymongo import MongoClient
import gridfs
from PIL import Image
import io
from bson import ObjectId
import numpy as np

# MongoDB connection settings
mongodb_host = "localhost"
mongodb_port = 27017

# Connect to MongoDB
client = MongoClient(mongodb_host, mongodb_port)
db_image = client.images

# GridFS
fs_images = gridfs.GridFS(db_image)

# Image ID (image_fid) to retrieve
image_fid = '64e32c9f18d1803a352edd98'  # Update with the actual image_fid

# Convert the image_fid string to an ObjectId
image_id = ObjectId(image_fid)

# Retrieve the image binary data from GridFS based on _id
img_gridout = fs_images.get(image_id)

if img_gridout:
    img_binary_data = img_gridout.read()
    # Convert binary data to a NumPy array
    img_array = np.frombuffer(img_binary_data, dtype=np.uint8)
    print(img_array.shape)
    # Convert the binary data to an image using PIL
    image = Image.open(io.BytesIO(img_binary_data))
    image_array = np.asarray(image)
    print(image_array.shape)
    # Display the image
    image.show()
else:
    print("Image not found.")
