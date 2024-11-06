from ultralytics import YOLO

# Load a model
model = YOLO("yolov8n.pt")  # load a pretrained model (recommended for training)
# Use the model
results = model.train(data="./data.yaml", epochs=10)  # train the model
# Validate the model
results = model.val()  # evaluate model performance on the validation set