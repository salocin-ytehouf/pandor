FROM python:3.10-slim-bullseye

RUN apt-get update \
  && apt-get install -y --no-install-recommends --no-install-suggests \
  build-essential nano intel-opencl-icd libgl1-mesa-glx libglib2.0-0 libjpeg-dev libpng-dev libtiff-dev libavcodec-dev libavformat-dev libswscale-dev libv4l-dev libgtk-3-dev libatlas-base-dev gfortran \
  && pip install --no-cache-dir --upgrade pip

WORKDIR /app
COPY ./requirements.txt /app
RUN pip install wheel setuptools
RUN pip install  --no-cache-dir --requirement /app/requirements.txt
RUN pip install pika
RUN pip install pymongo
COPY . /app

CMD ["python3", "consumer.py"]