from plate_recognition.LPRNet_Pytorch.src.lprnet_utils import *
from detector.src.yolo_utils import *

import numpy as np
import cv2

import argparse


def test_video(quantized_det_model_path, path_model_lprnet, video_file):
    # Load YOLOv8 model
    core = Core()
    quantized_det_model = core.read_model(model=quantized_det_model_path)
    quantized_det_compiled_model = core.compile_model(quantized_det_model, "CPU")
    label_map = {0: 'person', 1: 'face', 2: 'bicycle', 3: 'car', 4: 'motorcycle', 5: 'bus', 6: 'truck', 7: 'plate', 8: 'cat', 9: 'dog'}

    # Load LPRNet model
    cuda_lprnet = False
    lprnet = lprnet_load(path_model_lprnet, cuda_lprnet, CHARS)

    # Open the video file
    video_capture = cv2.VideoCapture(video_file)

    if not video_capture.isOpened():
        print("Error: Could not open video file.")
        exit()

    while True:
        ret, frame = video_capture.read()
        for i in range(0, 10):
            ret, frame = video_capture.read()
        if not ret:
            break

        rgb_frame = frame[:, :, ::-1]
    
        # Run YOLOv8 object detection model
        detections = yolo_detect(rgb_frame, quantized_det_compiled_model)
        
        # Filter the results to get only license plate detections
        plate_detections = yolo_filter_class(detections[0], 7)

        # Run LPRNet license plate recognition on the detected plates
        final = lprnet_process_batch(frame, plate_detections, lprnet, cuda_lprnet, debug=True)
        image_with_boxes = yolo_draw_results(detections[0], np.asarray(frame), label_map)
        cv2.imshow('Video', image_with_boxes)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    video_capture.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Test YOLOv8 and LPRNet models on a video file.")
    parser.add_argument("--det_model", required=True, help="Path to the YOLOv8 model file")
    parser.add_argument("--lprnet_model", required=True, help="Path to the LPRNet model file")
    parser.add_argument("--video_file", required=True, help="Path to the video file")
    args = parser.parse_args()

    test_video(args.det_model, args.lprnet_model, args.video_file)


