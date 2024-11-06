# AI MODULES
# test.py Description:

The `test.py` script is designed to perform object detection using YOLOv8 and license plate recognition using LPRNet on a video file. It utilizes pre-trained models for both tasks to detect objects and recognize license plates in each frame of the video.

## Usage:

To use the script, follow these steps:

1. **Download Pre-trained Models:**

   Ensure you have the pre-trained YOLOv8 and LPRNet models available. You can obtain these models from their respective sources and place them in the appropriate directories.

   https://drive.google.com/file/d/1C1eBNns4Q6dHxxYb_XetgL7LATMdx1AM/view?usp=drive_link

2. **Install Dependencies:**
   Make sure you have all the required dependencies installed. check requirements.txt in plate_recognition and detector folder


3. **Run the Script:**
Execute the script `script.py` from the command line with the following arguments:
- `<YOLOv8_model_path>`: Path to the YOLOv8 model file.
- `<LPRNet_model_path>`: Path to the LPRNet model file.
- `<video_file_path>`: Path to the video file on which you want to perform object detection and license plate recognition.

```bash
python test.py --det_model ./detector/models/best_openvino_int8_model/best.xml --lprnet_model ./plate_recognition/LPRNet_Pytorch/weights/Final_LPRNet_model.pth --video_file ./data_video/video_5.mp4
```
4. **View Results:**
The script will display the processed video with bounding boxes around detected objects and recognized license plates. Press the 'q' key to exit the video playback.

# LPRNet_Pytorch plate_recognition
### Introduction
This project implements a License Plate Recognition (LPR) system using PyTorch. The system is designed to recognize and interpret license plate numbers from images. It utilizes the LPRNet architecture, which is specifically tailored for license plate recognition tasks.

### Project Structure
The project structure is organized as follows:
```
LPRNet_Pytorch/
├── data/
│   ├── final_test/              # Directory containing test data
│   ├── final_train/             # Directory containing training data
│   ├── load_data.py             # data loader class for pytorch
│   └── __pycache__/             # Cache directory for compiled Python files
│       └── load_data.cpython-38.pyc
├── model/
│   ├── LPRNet.py                # Python script containing LPRNet model definition
│   └── __pycache__/             # Cache directory for compiled Python files
│       ├── __init__.cpython-38.pyc
│       └── LPRNet.cpython-38.pyc
├── README.md                    # This file providing an overview of the project
├── requirements.txt             # Python dependencies for training LPRNet model
├── test_LPRNet.py               # Python script for testing LPRNet model
├── train_LPRNet.py              # Python script for training LPRNet model
└── weights/
    └── Final_LPRNet_model.pth   # Pre-trained weights for the LPRNet model
```
### Dataset 
https://drive.google.com/file/d/1RkHSedxo_aeURPRm_h4U7d3WfKrkZPHj/view?usp=drive_link

The dataset contains a total of 7355 images, which are split into two subsets:

- **Train Set**: Consists of 6564 images used for training the model.
- **Test Set**: Contains 791 images utilized for evaluating the trained model.

Each image in the dataset is named according to the following convention: `hash_label.jpeg`, where:
- `hash`: Represents a unique identifier or hash for the image.
- `label`: Denotes the label associated with the image.

For example, an image named `abc123_456-AB-25.jpeg` would have the hash `abc123` and the corresponding label `56-AB-25` (licence plate target).


### Usage

1. if you want to prepare your datasets, image size must be 94x24.
2. base on your datsets path modify the scripts its hyperparameters --train_img_dirs or --test_img_dirs.
3. adjust other hyperparameters if need.
4. run 'python train_LPRNet.py' or 'python test_LPRNet.py'.
5. if want to show testing result, add '--show true' or '--show 1' to run command.

- **Training**: To train the LPRNet model, run `train_LPRNet.py` script. Make sure to have training data in the `final_train/` directory.
  
  ```bash
  python train_LPRNet.py
  ```

- **Testing**:
To test the LPRNet model, run the `test_LPRNet.py` script. Make sure you have the test data available in the `final_test/` directory.

  ```bash
  python test_LPRNet.py
  ```
## Dependencies

check requirements.txt file for python dependencies

## Acknowledgments

- The LPRNet architecture was proposed by Tian et al.
- The implementation is based on PyTorch and follows the guidelines provided by the PyTorch community.


## Contribution

Contributions to the project are welcome. Please feel free to fork the repository, make changes, and submit pull requests.

For any suggestions, questions, or issues, please open an issue on the GitHub repository.

Enjoy License Plate Recognition with LPRNet and PyTorch!


# YOLOv8 detector

This repository contains code to train YOLOv8 on a custom dataset with 10 new classes. The repository also includes scripts to convert and export the trained model into ONNX format and quantize the model to 8-bit for better performance.

## Dataset
https://drive.google.com/file/d/1O6utG0vf9kQryIgJp5iINJt4sMdk8UXV/view?usp=drive_link

The dataset used for training YOLOv8 includes images and corresponding label files. The directory structure of the dataset is as follows:
```
├── data
│   ├── coco_bike.jpg
│   ├── coco_dataset_extended.zip
│   ├── train
│   │   ├── images
│   │   │   ├── 000000000036.jpg
│   │   │   ├── 000000000042.jpg
│       └── labels
│           ├── 000000000036.txt
│           ├── 000000000042.txt
│   └── valid
│       ├── images
│       │   ├── 000000000139.jpg
│       │   ├── 000000000724.jpg
│       └── labels
│           ├── 000000000139.txt
│           ├── 000000000724.txt
```


The data.yaml file contains configuration details for the dataset and classes. It specifies the paths for training, testing, and validation images, the number of classes (nc), and the class names (names).

```
path: data/
train: train/images
test: valid/images
val: valid/images

# Classes 
nc: 10 # replace based on your dataset's number of classes 

# Class names 
# replace all class names with your own classes' names 
names: ['person', 'face', 'bicycle', 'car', 'motorcycle', 'bus', 'truck', 'plate', 'cat', 'dog']
```

Basically this new data set is the coco dataset with two additional classes, face and licence plate.
## YOLOv8 Trainig 

To train the YOLOv8 model, run the following command:

```bash
python train.py
```
### YOLOv8 Training Results

The `yolo_res_training` folder contains various files and visualizations related to the training process of the YOLOv8 model. Here's a brief overview of the contents:

- **args.yaml**: Configuration file containing training arguments used during the YOLOv8 training process.

- **confusion_matrix_normalized.png**: Normalized confusion matrix visualizing the model's performance on the validation set.

- **confusion_matrix.png**: Confusion matrix visualizing the model's performance on the validation set.

- **F1_curve.png**: F1-score curve showing the model's F1-score over epochs during training.

- **labels_correlogram.jpg**: Correlogram visualizing the relationship between different classes in the dataset.

- **labels.jpg**: Image showing the distribution of different classes in the dataset.

- **P_curve.png**: Precision-recall curve illustrating the precision-recall trade-off of the model.

- **PR_curve.png**: Precision-recall curve illustrating the precision-recall trade-off of the model.

- **R_curve.png**: Recall curve showing the model's recall over epochs during training.

- **results.csv**: CSV file containing detailed results and metrics of the model's performance.

- **results.png**: Image summarizing the results and metrics of the model's performance.

- **train_batch2360820.jpg**, **train_batch2360821.jpg**, **train_batch2360822.jpg**: Example images from the training batches.

- **weights/best.pt**: Trained weights of the YOLOv8 model that achieved the best performance during training.

These files and visualizations provide insights into the training process and performance of the YOLOv8 model on the custom dataset.


## Performance Optimization Using ONNX and INT8 Quantization
This section discusses performance optimization techniques employed in this repository, specifically focusing on leveraging ONNX format and INT8 quantization.

- **ONNX Format**: The trained YOLOv8 model is converted into the ONNX format (`best.onnx`) using the `model_converter.py` script. ONNX provides a standardized format for representing deep learning models, allowing for interoperability across different frameworks and optimization opportunities.

- **INT8 Quantization**: Quantization is a technique used to reduce the precision of numerical values in a neural network model, leading to reduced memory footprint and faster inference time. In this repository, the ONNX model is further quantized to 8-bit integer precision (`best_openvino_int8_model/best.bin` and `best_openvino_int8_model/best.xml`) using the `model_converter.py` script. This quantized model is optimized for deployment on hardware platforms that support INT8 operations, such as Intel CPUs with AVX-512 support.

By leveraging ONNX format and INT8 quantization, the inference performance of the YOLOv8 model is significantly improved, making it suitable for deployment in resource-constrained environments or real-time applications.


To convert and export the trained model into ONNX format and quantized 8-bit model, use the model_converter.py script:
```bash
python model_converter.py --path_pt_model='./models/best.pt' --convert --test_onnx --quantize --test_int8
```

### Models directory


https://docs.openvino.ai/2022.3/notebooks/230-yolov8-optimization-with-output.html#instance-segmentation
```
├── models
│   ├── best.onnx
│   ├── best_openvino_int8_model
│   │   ├── best.bin
│   │   └── best.xml
│   ├── best_openvino_model
│   │   ├── best.bin
│   │   ├── best.xml
│   │   └── metadata.yaml
│   └── best.pt
```
### Result elapsed time (one image)
- pytorch model 
    - 0.234 second
- onnx model 
    - 0.077304 seconds
- quantized int_8 model 
    - 0.014374 seconds


## Dependencies
check requirements.txt file for python dependencies

## Acknowledgments

- The YOLOv8 architecture was proposed by Alexey Bochkovskiy, Chien-Yao Wang, and Hong-Yuan Mark Liao. The implementation in this repository is based on their work.
- The implementation of YOLOv8 follows the guidelines provided by the PyTorch community and is inspired by various open-source repositories and resources available online.

