import cv2
import pymongo 
import numpy as np
import gridfs
from bson.objectid import ObjectId
import face_recognition
import dlib
import time
import json

def load_img_from_mongo(id_img, client):
    db_image = client.images
    fs_images = gridfs.GridFS(db_image)
    img_gridout = fs_images.get(ObjectId(id_img))
    img_binary_data = img_gridout.read()
    nparr = np.frombuffer(img_binary_data, np.uint8)
    image = cv2.imdecode(nparr, cv2.IMREAD_COLOR)  
    img = np.asarray(image)
    return img

def draw_bboxes_on_image(img, detection):
    # Make a copy of the image to avoid modifying the original
    image_with_boxes = img.copy()

    # Define colors for drawing boxes (BGR format)
    box_color = (0, 255, 0)  # Green color
    text_color = (0, 0, 255)  # Red color for text

    bbox = detection.get("bbox")
    label = detection.get("label")
    confidence = detection.get("confidence")

    # Extract coordinates of the bounding box
    x_min, y_min, x_max, y_max = [int(coord) for coord in bbox]

    # Draw the bounding box rectangle
    cv2.rectangle(image_with_boxes, (x_min, y_min), (x_max, y_max), box_color, 2)

    # Create a label string (e.g., "car: 0.90")
    label_text = f"{label}: {confidence:.2f}"

    # Define the position for the label text
    label_position = (x_min, y_min - 10)  # Above the box

    # Draw the label text on the image
    cv2.putText(image_with_boxes, label_text, label_position, cv2.FONT_HERSHEY_SIMPLEX, 0.5, text_color, 2)

    return image_with_boxes

def bbox_to_rect(bbox):
    """
    Convert a tuple in (top, right, bottom, left) order to a dlib `rect` object

    :param css:  plain tuple representation of the rect in (top, right, bottom, left) order
    :return: a dlib `rect` object
    """
    x_min, y_min, x_max, y_max = [int(coord) for coord in bbox]
    return dlib.rectangle(x_min, y_min, x_max, y_max)

def rect_to_css(rect):
    """
    Convert a dlib 'rect' object to a plain tuple in (top, right, bottom, left) order

    :param rect: a dlib 'rect' object
    :return: a plain tuple representation of the rect in (top, right, bottom, left) order
    """
    return rect.top(), rect.right(), rect.bottom(), rect.left()

def raw_face_landmarks(face_image, face_locations=None, model="large"):
    if face_locations is None:
        face_locations = _raw_face_locations(face_image)
    else:
        face_locations = [_css_to_rect(face_location) for face_location in face_locations]

    pose_predictor = pose_predictor_68_point

    if model == "small":
        pose_predictor = pose_predictor_5_point

    return [pose_predictor(face_image, face_location) for face_location in face_locations]

def draw_points_on_image(image, points_list):


    # Loop through the points and draw them on the image
    for point_type, point_coordinates in points_list[0].items():
        for x, y in point_coordinates:
            # Draw a point on the image
            cv2.circle(image, (x, y), 2, (0, 0, 255), -1)  # Red point, thickness -1 to fill the circle


    cv2.imshow('Image with Points', image)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

def facial_encode(input, img):
    rect = bbox_to_rect(input["res"]["bbox"])
    rect = rect_to_css(rect)

    start_time = time.time()
    encoding = face_recognition.face_encodings(img, [rect], num_jitters=1, model="large")
    end_time = time.time()
    elapsed_time = end_time - start_time
    print(f"Elapsed time: {elapsed_time:.6f} seconds")

    return encoding

def is_authorised(input, encoding, threshold, client):
    worker_id = input["worker_id"]
    is_door = input["is_door"]
    camera_id = input["camera_id"]
    #compute distance
    
    db = client["auth_face"]
    # Specify the collection where you want to insert data (e.g., "users")
    collection = db["users"]
    faces = collection.find()
    for face in faces:
        dist = np.linalg.norm(np.array(face['encoding']) - np.array(encoding), axis=1)
        print("distance between faces :{}".format(dist))
        if dist < threshold:
            return face
    return None

def start(message, client):
    message = json.loads(message)
    print(message,  flush=True)
    img = load_img_from_mongo(message["image_fid"], client)
    encoding = facial_encode(message, img)
    res = is_authorised(message, encoding, 0.5, client)

    if res:
        print('authorised person ')
        #put in db_face with image, camera_id, worker_id, face_id and ts, authorized status
        db = client["faces"]
        collection = db["faces"]
        data={"camera_id":message["camera_id"], 
        "worker_id":message["worker_id"],
        "face_id":res["user_id"], 
        "ts":message["ts"],
        "res":message["res"],
        "auth_status":True}
        try:
            result = collection.insert_one(data)
            print("Inserted document ID:", result.inserted_id)
        except Exception as e:
            print("Error inserting data:", e)
        if message["is_door"] == "True":
            print("open door_id ")
    else:
        print("unauthorized people detected")
        db = client["faces"]
        collection = db["faces"]

        data={"camera_id":message["camera_id"], 
        "worker_id":message["worker_id"],
        "face_id":"", 
        "ts":message["ts"],
        "res":message["res"],
        "auth_status":False}

        try:
            result = collection.insert_one(data)
            print("Inserted document ID:", result.inserted_id)
        except Exception as e:
            print("Error inserting data:", e)
        #put in db_face with image, camera_id, worker_id, face_id and ts, authorized status


    #tmpp = face_recognition.face_landmarks(img, [rect], model="large")
    #draw_points_on_image(final, tmpp)

