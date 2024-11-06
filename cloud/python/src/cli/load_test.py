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
import numpy as np
import ast 

server = Flask(__name__)
mongo_images = PyMongo(server, uri="mongodb://localhost:27017/images")
fs = gridfs.GridFS(mongo_images.db)

message = {"image_fid": "652659bb4be086a63bee71d0", "worker_id": "nico@gmail.com", "ts": 1697012153, "camera_id": "camera2", "is_door": "False", "nb_attempt": 0}


img_gridout = fs.get(ObjectId(message["image_fid"]))
img_binary_data = img_gridout.read()
data_string = img_binary_data.decode('utf-8')

# Étape 2 : Convertir la chaîne de caractères en une liste
import ast
data_list = ast.literal_eval(data_string)

# Étape 3 : Convertir la liste en un tableau NumPy
numpy_array = np.array(data_list)
cv2.imshow(",", numpy_array)
cv2.waitKey(0)
cv2.destroyAllWindows()