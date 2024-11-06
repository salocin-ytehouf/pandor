import os, gridfs, pika, json
from flask import Flask, request, send_file
from flask_pymongo import PyMongo
from auth import validate
from auth_svc import access
from storage import util
from bson.objectid import ObjectId
import json

server = Flask(__name__)

mongo_images = PyMongo(server, uri="mongodb://host.minikube.internal:27017/images") 
fs_images = gridfs.GridFS(mongo_images.db)

connection = pika.BlockingConnection(pika.ConnectionParameters("rabbitmq", heartbeat=3600))
channel = connection.channel()


@server.route("/login", methods=["POST"])
def login():
    token, err = access.login(request)

    if not err:
        return token
    else:
        return err


def is_payload_valid():
    return True


@server.route("/upload", methods=["POST"])
def upload():
    access, err = validate.token(request)
    
    if err:
        return err

    access = json.loads(access)
    print(json.dumps(access, indent = 4), flush=True)

    if access["role"]==1:
        try:
            payload = request.get_json()
            #print(json.dumps(payload, indent = 4), flush=True)

        except Exception as e:
            return f"Error processing payload: {e}", 400            
        
        if is_payload_valid():
            print('payload is valid', flush=True)
            err = util.upload(payload['image1'], fs_images, channel, access)

            if err:
                return err

            err = util.upload(payload['image2'], fs_images, channel, access)
            if err:
                return err

        else:
            return "Bad Request: Invalid payload data", 400

        return "success!", 200
        
    else:
        return "not authorized", 401

if __name__ == "__main__":
    server.run(host="0.0.0.0", port=8080)
