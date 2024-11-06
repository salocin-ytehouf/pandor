from src.yolo_utils import *

from pathlib import Path
import urllib.request
from PIL import Image
from typing import Tuple, Dict
import cv2
from ultralytics.yolo.utils.plotting import colors
from ultralytics import YOLO
from ultralytics.yolo.utils import ops
import torch
import numpy as np
import time
from openvino.runtime import Core, Model
import os
import torch
from torch.utils.data import Dataset, DataLoader
from openvino.runtime import serialize
import nncf  # noqa: F811
from typing import Dict
import argparse


def load_test_img():
    img = cv2.imread('./data/coco_bike.jpg')
    img_np =np.asarray(img)
    return img_np

####################################################################
################# convert model to onnx format #####################
####################################################################

def convert_pt_to_onnx(path_pt_model):
    # object detection model
    det_model = YOLO(path_pt_model)
    det_model.export(format="openvino", dynamic=True, half=False)

def yolo_compiled_detect(image:np.ndarray, model:Model):
    """
    OpenVINO YOLOv8 model inference function. Preprocess image, runs model inference and postprocess results using NMS.
    Parameters:
        image (np.ndarray): input image.
        model (Model): OpenVINO compiled model.
    Returns:
        detections (np.ndarray): detected boxes in format [x1, y1, x2, y2, score, label]
    """
    num_outputs = len(model.outputs)
    preprocessed_image = yolo_preprocess_image(image)
    input_tensor = yolo_image_to_tensor(preprocessed_image)
    result = model(input_tensor)
    boxes = result[model.output(0)]
    masks = None
    if num_outputs > 1:
        masks = result[model.output(1)]
    input_hw = input_tensor.shape[2:]
    detections = yolo_postprocess(pred_boxes=boxes, input_hw=input_hw, orig_img=image, pred_masks=masks)
    return detections

def test_onnx_model():
    label_map = {0: 'person', 1: 'face', 2: 'bicycle', 3: 'car', 4: 'motorcycle', 5: 'bus', 6: 'truck', 7: 'plate', 8: 'cat', 9: 'dog'}

    model_onnx_xml_path='./models/best_openvino_model/best.xml'
    core = Core()
    det_ov_model = core.read_model(model_onnx_xml_path)
    det_compiled_model = core.compile_model(det_ov_model, "CPU")
    input_image = load_test_img()
    print("##############################################")

    start_time = time.time()
    detections = yolo_compiled_detect(input_image, det_compiled_model)[0]
    end_time = time.time()
    elapsed_time = end_time - start_time
    print(f"Elapsed time: {elapsed_time:.6f} seconds")
    print("##############################################")
    image_with_boxes = yolo_draw_results(detections, input_image, label_map)
    cv2.imshow('Image',image_with_boxes)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

def transform_fn(data_item):
    images, _ = data_item
    return images[0]

####################################################################
##### Optimize model using NNCF Post-training Quantization AP ######
####################################################################

def quantizer_int_8():

    data_dir = './data/valid'
    label_map = {0: 'person', 1: 'face', 2: 'bicycle', 3: 'car', 4: 'motorcycle', 5: 'bus', 6: 'truck', 7: 'plate', 8: 'cat', 9: 'dog'}

    core = Core()
    det_model_path = './models/best_openvino_model/best.xml'
    det_ov_model = core.read_model(det_model_path)
    det_compiled_model = core.compile_model(det_ov_model, "CPU")

    valid_dataset = CustomYOLODataset(data_dir, label_map)
    calibration_loader = torch.utils.data.DataLoader(valid_dataset) #shuffle = True

    calibration_dataset = nncf.Dataset(calibration_loader, transform_fn)

    ignored_scope = nncf.IgnoredScope(
        types=["Multiply", "Subtract", "Sigmoid"],  # ignore operations
        names=[
            "/model.22/dfl/conv/Conv",           # in the post-processing subgraph
            "/model.22/Add",
            "/model.22/Add_1",
            "/model.22/Add_2",
            "/model.22/Add_3",
            "/model.22/Add_4",
            "/model.22/Add_5",
            "/model.22/Add_6",
            "/model.22/Add_7",
            "/model.22/Add_8",
            "/model.22/Add_9",
            "/model.22/Add_10"
        ]
    )

    # Detection model
    quantized_det_model = nncf.quantize(
        det_ov_model,
        calibration_dataset,
        preset=nncf.QuantizationPreset.MIXED,
        ignored_scope=ignored_scope,
        subset_size=3700
    )

    #################################################################################################################
    #################################################################################################################
    #################################################################################################################


    int8_model_det_path = './models/best_openvino_int8_model/best.xml'
    print(f"Quantized detection model will be saved to {int8_model_det_path}")
    serialize(quantized_det_model, str(int8_model_det_path))
    return

def test_int8_model():

    core = Core()
    quantized_det_model_path ='./models/best_openvino_int8_model/best.xml'
    quantized_det_model = core.read_model(model=quantized_det_model_path)
    quantized_det_compiled_model = core.compile_model(quantized_det_model, "CPU")
    label_map = {0: 'person', 1: 'face', 2: 'bicycle', 3: 'car', 4: 'motorcycle', 5: 'bus', 6: 'truck', 7: 'plate', 8: 'cat', 9: 'dog'}

    input_image = load_test_img()
    start_time = time.time()
    detections = yolo_compiled_detect(input_image, quantized_det_compiled_model)[0]
    end_time = time.time()
    elapsed_time = end_time - start_time
    print(f"Elapsed time: {elapsed_time:.6f} seconds")
    image_with_boxes = yolo_draw_results(detections, input_image, label_map)
    cv2.imshow('Image',image_with_boxes)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

    input_image = load_test_img()
    start_time = time.time()
    detections = yolo_compiled_detect(input_image, quantized_det_compiled_model)[0]
    end_time = time.time()
    elapsed_time = end_time - start_time
    print(f"Elapsed time: {elapsed_time:.6f} seconds")
    image_with_boxes = yolo_draw_results(detections, input_image, label_map)
    cv2.imshow('Image',image_with_boxes)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

    input_image = load_test_img()
    start_time = time.time()
    detections = yolo_compiled_detect(input_image, quantized_det_compiled_model)[0]
    end_time = time.time()
    elapsed_time = end_time - start_time
    print(f"Elapsed time: {elapsed_time:.6f} seconds")
    image_with_boxes = yolo_draw_results(detections, input_image, label_map)
    cv2.imshow('Image',image_with_boxes)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

####################################################################
####################### final video test ###########################
####################################################################

def main():
    parser = argparse.ArgumentParser(description="Script to convert a PyTorch model to ONNX format, test it, quantize to INT8, and test the INT8 model.")
    parser.add_argument('--path_pt_model', type=str, help='Path to the PyTorch model (.pt)')
    parser.add_argument('--convert', action='store_true', help='Convert the model to ONNX')
    parser.add_argument('--test_onnx', action='store_true', help='Test the ONNX model')
    parser.add_argument('--quantize', action='store_true', help='Quantize to INT8')
    parser.add_argument('--test_int8', action='store_true', help='Test the INT8 model')
    args = parser.parse_args()

    if args.convert:
        if args.path_pt_model:
            convert_pt_to_onnx(args.path_pt_model)
        else:
            print("Please provide the path to the PyTorch model using --path_pt_model argument.")
    
    if args.test_onnx:
        test_onnx_model()
    
    if args.quantize:
        quantizer_int_8()
    
    if args.test_int8:
        test_int8_model()

if __name__ == "__main__":
    main()