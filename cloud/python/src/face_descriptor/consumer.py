import pika, sys, os, time
from pymongo import MongoClient
import gridfs
from face import descriptor

def main():
    client = MongoClient("host.minikube.internal", 27017)
    db_image = client.images

    # gridfs
    fs_images = gridfs.GridFS(db_image)

    # rabbitmq connection
    connection = pika.BlockingConnection(pika.ConnectionParameters(host="rabbitmq", heartbeat=3600))
    channel = connection.channel()

    def callback(ch, method, properties, body):
        #err = to_mp3.start(body, fs_videos, fs_mp3s, ch)
        err = descriptor.start(body, client)
        if err:
            ch.basic_nack(delivery_tag=method.delivery_tag)
        else:
            ch.basic_ack(delivery_tag=method.delivery_tag)

    channel.basic_consume(
        queue=os.environ.get("IMAGE_QUEUE"), on_message_callback=callback #changer variable nom queue
    )

    print("Waiting for messages. To exit press CTRL+C")

    channel.start_consuming()

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("Interrupted",flush=True)
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)

