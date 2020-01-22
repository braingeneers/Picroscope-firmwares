import time
import datetime
import glob
import argparse
import io
import json

import boto3
from botocore.errorfactory import ClientError

from PIL import Image, ImageDraw
from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient

__RECORDING__ = False
__BASE__ = "rcurrie/archive"
__UUID__ = ""


def init(name):
    client = AWSIoTMQTTClient("arn:aws:iot:us-west-2:443872533066:thing/{}".format(name))
    client.configureEndpoint("ahp00abmtph4i-ats.iot.us-west-2.amazonaws.com", 8883)
    client.configureCredentials("certs/AmazonRootCA1.pem",
                                glob.glob("certs/{}/*-private.pem.key".format(name))[0],
                                glob.glob("certs/{}/*-certificate.pem.crt".format(name))[0])
    client.configureOfflinePublishQueueing(-1)  # Infinite offline Publish queueing
    client.configureDrainingFrequency(2)  # Draining: 2 Hz
    client.configureConnectDisconnectTimeout(10)  # 10 sec
    client.configureMQTTOperationTimeout(5)  # 5 sec
    return client


def callback(client, userdata, message):
    global __RECORDING__
    global __UUID__
    print(message.topic, message.payload)
    if message.topic.endswith("/start"):
        __RECORDING__ = True
        __UUID__ = json.loads(message.payload.decode())["uuid"]
    elif message.topic.endswith("/stop"):
        __RECORDING__ = False


def generate_and_upload_image(text, bucket, key):
    """ Generate an image with text in it and upload to S3 publicly accessible """
    image = Image.new('RGB', (320, 240), color=(73, 109, 137))
    d = ImageDraw.Draw(image)
    d.text((10, 10), text, fill=(255, 255, 0))

    in_mem_file = io.BytesIO()
    image.save(in_mem_file, format="JPEG")
    in_mem_file.seek(0)

    bucket.upload_fileobj(in_mem_file, key,
                          ExtraArgs={"ContentType": "image/jpeg", "ACL": "public-read"})


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Demo braingeneers processing daemon")
    parser.add_argument("-n", "--name", required=True, help="Thing name")
    args = parser.parse_args()

    session = boto3.session.Session()
    s3 = session.resource(
        "s3", endpoint_url="https://s3.nautilus.optiputer.net")
    bucket = s3.Bucket("braingeneers")
    json.load_s3 = lambda f: json.load(bucket.Object(key=f).get()["Body"])
    json.dump_s3 = lambda obj, f: bucket.Object(key=f).put(
        Body=json.dumps(obj), ACL='public-read')

    client = init(args.name)
    client.connect()
    client.subscribe("{}/#".format(args.name), 1, callback)
    print("Thing {} and listening for events...".format(args.name))

    while True:
        try:
            time.sleep(5)
            if __RECORDING__:
                timestamp = datetime.datetime.now().isoformat()
                print("Snap @ {} to {} UUID".format(timestamp, __UUID__))

                try:
                    manifest = json.load_s3("{}/{}/images/manifest.json".format(__BASE__, __UUID__))
                except ClientError:
                    print("Initializing manifest")
                    manifest = {
                       "uuid": __UUID__,
                       "timestamp": timestamp,
                       "num_stacks": 4,
                       "stack_size": 2,
                       "captures": []
                    }
                    pass

                # Upload a set of images
                for stack in range(manifest["num_stacks"]):
                    for z in range(manifest["stack_size"]):
                        text = "{} {} {}".format(timestamp, stack, z)
                        key = "{}/{}/images/{}/{}/{}.jpg".format(
                            __BASE__, __UUID__, timestamp, stack, z)
                        generate_and_upload_image(text, bucket, key)
                        print("Uploaded", text, key)

                # Add this capture to manifest and update
                manifest["captures"].append(timestamp)
                json.dump_s3(manifest, "{}/{}/images/manifest.json".format(__BASE__, __UUID__))

        except KeyboardInterrupt:
            break

    client.disconnect()
    print("Stopped listening to events and disconnected.")

