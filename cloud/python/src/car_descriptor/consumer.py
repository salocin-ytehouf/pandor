import pika, sys, os, time
from pymongo import MongoClient
import gridfs
from car import descriptor

def main():
    client = MongoClient("host.minikube.internal", 27017)
    db_image = client.images

    # gridfs
    fs_images = gridfs.GridFS(db_image)

    # rabbitmq connection
    connection = pika.BlockingConnection(pika.ConnectionParameters(host="rabbitmq", heartbeat=3600))
    channel = connection.channel()

    #load lprnet 
    cuda_lprnet = False
    path_model_lprnet = './model/LFinal_LPRNet_model.pth'
    CHARS = ['京', '沪', '津', '渝', '冀', '晋', '蒙', '辽', '吉', '黑',
            '苏', '浙', '皖', '闽', '赣', '鲁', '豫', '鄂', '湘', '粤',
            '桂', '琼', '川', '贵', '云', '藏', '陕', '甘', '青', '宁',
            '新',
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K',
            'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
            'W', 'X', 'Y', 'Z', 'I', 'O', '-'
            ]
    lprnet = descriptor.load_lprnet(path_model_lprnet, cuda_lprnet, CHARS)

    def callback(ch, method, properties, body):
        err = descriptor.start(body, client, lprnet, cuda_lprnet)
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

