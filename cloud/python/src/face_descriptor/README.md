consumer.py
data/
detector/
Dockerfile
.dockerignore
manifests/
models/
requirements.txt
venv/


input 
{"image_fid": "651f94f9ebfe93a937f14a6e", "worker_id": "nico@gmail.com", "ts": "timestamp", "camera_id": "1", "nb_attempt": 0, "res": {"bbox": [271.0, 152.0, 357.0, 262.0], "confidence": 0.9213894009590149, "label": "face"}}
def is_door_camera():
    #check if a camera access door 
    if yes 
        #load from mysql wich user is bind to the camera door, wich door is it 
        #load stored face user features
        return list 
    if not return None 

def load_cropped_img(, mongo_client, image_fid):
    #findimage in mongodb gridfs
    #load image from binaries into numpy array with cv2
    return cropped_img

def extract_feature(model, cropped_img):
    #pass image to the model
    save features_vector to mongo db 
    return features_vector
    
def compare_feature(list_face features, face_features):
    #compare if match 
        publish opendoor -
        return true

test cli 
    choisir deux photos avec des visages  
    envoyer les deux photo service upload 
    detection deux visages publish on queue 
    consume favce queue message 


acheter serrure plus camera cette semaine !!!!!!
