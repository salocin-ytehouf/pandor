import pika, json
from bson import ObjectId
import base64

def upload(f, fs, channel, access):
    #push data in mongo db modify to handle two images 
    print('inside util storage endpoint')
    print('#####################################################', flush=True)
    try:
        print('before fs.put()', flush=True)
        metadata = f["metadata"]
        image_data_binary = base64.b64decode(f["data"])
        fid = fs.put(image_data_binary, **metadata)
        print('after fs.put()', flush=True)
    except Exception as err:
        print(err, flush=True)
        return "internal server error mongodb, image not uploaded retry", 500

    message = {
        "image_fid": str(fid),
        "worker_id": access["username"],
        "ts": metadata["timestamp"],
        "camera_id":metadata["camera_id"],
        "is_door":metadata["is_door"],
        "nb_attempt":0
    }

    try:
        print('before channel.basic_publish()', flush=True)
        channel.basic_publish(
            exchange="",
            routing_key="image",
            body=json.dumps(message),
            properties=pika.BasicProperties(
                delivery_mode=pika.spec.PERSISTENT_DELIVERY_MODE
            ),
        )
        print('after channel.basic_publish()', flush=True)
    except Exception as err:
        print(err, flush=True)
        fs.delete(fid)
        return "internal server error rabbitmq, image not uploaded retry", 500
