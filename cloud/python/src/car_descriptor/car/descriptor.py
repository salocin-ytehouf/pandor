import cv2
from pymongo import MongoClient
import pymongo 
import numpy as np
import gridfs
from bson.objectid import ObjectId
import time
import json
from torch.autograd import Variable
import torch.nn.functional as F
from torch import optim
import torch.nn as nn
import torch
import os


class small_basic_block(nn.Module):
    def __init__(self, ch_in, ch_out):
        super(small_basic_block, self).__init__()
        self.block = nn.Sequential(
            nn.Conv2d(ch_in, ch_out // 4, kernel_size=1),
            nn.ReLU(),
            nn.Conv2d(ch_out // 4, ch_out // 4, kernel_size=(3, 1), padding=(1, 0)),
            nn.ReLU(),
            nn.Conv2d(ch_out // 4, ch_out // 4, kernel_size=(1, 3), padding=(0, 1)),
            nn.ReLU(),
            nn.Conv2d(ch_out // 4, ch_out, kernel_size=1),
        )
    def forward(self, x):
        return self.block(x)

class LPRNet(nn.Module):
    def __init__(self, lpr_max_len, phase, class_num, dropout_rate):
        super(LPRNet, self).__init__()
        self.phase = phase
        self.lpr_max_len = lpr_max_len
        self.class_num = class_num
        self.backbone = nn.Sequential(
            nn.Conv2d(in_channels=3, out_channels=64, kernel_size=3, stride=1), # 0
            nn.BatchNorm2d(num_features=64),
            nn.ReLU(),  # 2
            nn.MaxPool3d(kernel_size=(1, 3, 3), stride=(1, 1, 1)),
            small_basic_block(ch_in=64, ch_out=128),    # *** 4 ***
            nn.BatchNorm2d(num_features=128),
            nn.ReLU(),  # 6
            nn.MaxPool3d(kernel_size=(1, 3, 3), stride=(2, 1, 2)),
            small_basic_block(ch_in=64, ch_out=256),   # 8
            nn.BatchNorm2d(num_features=256),
            nn.ReLU(),  # 10
            small_basic_block(ch_in=256, ch_out=256),   # *** 11 ***
            nn.BatchNorm2d(num_features=256),   # 12
            nn.ReLU(),
            nn.MaxPool3d(kernel_size=(1, 3, 3), stride=(4, 1, 2)),  # 14
            nn.Dropout(dropout_rate),
            nn.Conv2d(in_channels=64, out_channels=256, kernel_size=(1, 4), stride=1),  # 16
            nn.BatchNorm2d(num_features=256),
            nn.ReLU(),  # 18
            nn.Dropout(dropout_rate),
            nn.Conv2d(in_channels=256, out_channels=class_num, kernel_size=(13, 1), stride=1), # 20
            nn.BatchNorm2d(num_features=class_num),
            nn.ReLU(),  # *** 22 ***
        )
        self.container = nn.Sequential(
            nn.Conv2d(in_channels=448+self.class_num, out_channels=self.class_num, kernel_size=(1, 1), stride=(1, 1)),
            # nn.BatchNorm2d(num_features=self.class_num),
            # nn.ReLU(),
            # nn.Conv2d(in_channels=self.class_num, out_channels=self.lpr_max_len+1, kernel_size=3, stride=2),
            # nn.ReLU(),
        )

    def forward(self, x):
        keep_features = list()
        for i, layer in enumerate(self.backbone.children()):
            x = layer(x)
            if i in [2, 6, 13, 22]: # [2, 4, 8, 11, 22]
                keep_features.append(x)

        global_context = list()
        for i, f in enumerate(keep_features):
            if i in [0, 1]:
                f = nn.AvgPool2d(kernel_size=5, stride=5)(f)
            if i in [2]:
                f = nn.AvgPool2d(kernel_size=(4, 10), stride=(4, 2))(f)
            f_pow = torch.pow(f, 2)
            f_mean = torch.mean(f_pow)
            f = torch.div(f, f_mean)
            global_context.append(f)

        x = torch.cat(global_context, 1)
        x = self.container(x)
        logits = torch.mean(x, dim=2)

        return logits

def build_lprnet(lpr_max_len=8, phase=False, class_num=66, dropout_rate=0.5):

    Net = LPRNet(lpr_max_len, phase, class_num, dropout_rate)

    if phase == "train":
        return Net.train()
    else:
        return Net.eval()

def load_lprnet(path_model, cuda, CHARS):
    
    lprnet = build_lprnet(lpr_max_len=8, phase=False, class_num=len(CHARS), dropout_rate=0)
    device = torch.device("cuda:0" if cuda else "cpu")
    lprnet.to(device)
    print("Successful to build network!")
    lprnet.load_state_dict(torch.load(path_model, map_location=torch.device('cpu')))
    print("load pretrained model successful!")

    return lprnet

def load_list_img(list_file, path):
    list_img = []
    for name in list_file:
        list_img.append(cv2.imread(path+name))
    return list_img

def post_process(prebs, CHARS):
    preb_labels = list()
    for i in range(prebs.shape[0]):
        preb = prebs[i, :, :]
        preb_label = list()
        for j in range(preb.shape[1]):
            preb_label.append(np.argmax(preb[:, j], axis=0))
        no_repeat_blank_label = list()
        pre_c = preb_label[0]
        if pre_c != len(CHARS) - 1:
            no_repeat_blank_label.append(pre_c)
        for c in preb_label: # dropout repeate label and blank label
            if (pre_c == c) or (c == len(CHARS) - 1):
                if c == len(CHARS) - 1:
                    pre_c = c
                continue
            no_repeat_blank_label.append(c)
            pre_c = c
        preb_labels.append(no_repeat_blank_label)
    lb = ""
    for i in preb_labels[0]:
        lb += CHARS[i]
    return lb

def pre_process(img, cuda):
    img = cv2.resize(img,(94,24))
    img = img.astype('float32')
    img -= 127.5
    img *= 0.0078125
    img = np.transpose(img, (2, 0, 1))
    img = torch.tensor(img)
    batched_image = img.unsqueeze(0)
    if cuda:
        batched_image = Variable(batched_image.cuda())
    else:
        batched_image = Variable(batched_image)
    return batched_image

def predict(img, lprnet, cuda):
    CHARS = ['京', '沪', '津', '渝', '冀', '晋', '蒙', '辽', '吉', '黑',
            '苏', '浙', '皖', '闽', '赣', '鲁', '豫', '鄂', '湘', '粤',
            '桂', '琼', '川', '贵', '云', '藏', '陕', '甘', '青', '宁',
            '新',
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K',
            'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
            'W', 'X', 'Y', 'Z', 'I', 'O', '-'
            ]
    img = pre_process(img, cuda)
    prebs = lprnet(img)
    prebs = prebs.cpu().detach().numpy()
    final_res = post_process(prebs,CHARS)
    return final_res

def load_img_from_mongo(id_img, client):
    db_image = client.images
    fs_images = gridfs.GridFS(db_image)
    img_gridout = fs_images.get(ObjectId(id_img))
    img_binary_data = img_gridout.read()
    nparr = np.frombuffer(img_binary_data, np.uint8)
    image = cv2.imdecode(nparr, cv2.IMREAD_COLOR)  
    img = np.asarray(image)
    return img

def crop_image(image, coordinates, padding_percent=15):

    #x1, y1, x2, y2, score, label = coordinates
    x1=coordinates["bbox"][0]
    y1=coordinates["bbox"][1]
    x2=coordinates["bbox"][2]
    y2=coordinates["bbox"][3]
    
    # Calculate the width and height of the region
    width = x2 - x1
    height = y2 - y1
    
    # Calculate the padding values
    padding_x = int(width * padding_percent / 100)
    padding_y = int(height * padding_percent / 100)
    
    # Adjust the coordinates to include padding
    x1 -= padding_x
    y1 -= padding_y
    x2 += padding_x
    y2 += padding_y
    
    # Ensure the coordinates are within the image bounds
    x1 = max(0, x1)
    y1 = max(0, y1)
    x2 = min(image.shape[1], x2)
    y2 = min(image.shape[0], y2)
    cropped_image = image[int(y1):int(y2), int(x1):int(x2)]
    return cropped_image


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

def start(message, client, lprnet, cuda_lprnet):
    message = json.loads(message)
    print(message,  flush=True)
    img = load_img_from_mongo(message["image_fid"], client)
    img_crop = crop_image(img, message["res"])
    res = predict(img_crop, lprnet, cuda_lprnet)
    print("number plate deteted is: {}".format(res))

    db = client["plates"]
    collection = db["plates"]
    data={"camera_id":message["camera_id"], 
    "worker_id":message["worker_id"],
    "plate_id": res, 
    "ts":message["ts"],
    "res":message["res"],
    "auth_status":True}
    try:
        result = collection.insert_one(data)
        print("Inserted document ID:", result.inserted_id)
    except Exception as e:
        print("Error inserting data:", e)


