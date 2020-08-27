#import paho-mqtt , pymongo ,dynpython <--srv use
import paho.mqtt.client as mqtt
import pymongo
from pymongo import MongoClient
#MongoDB MongoClient connect
#EXAMPLE ID : geumbi PASSWORD: rodutls179 , INPUT YOUR ID,PS 
cluster=pymongo.MongoClient("mongodb+srv://geumbi:rodutls179@cluster0.wveeu.mongodb.net/<dbname>?retryWrites=true&w=majority")
db=cluster["test"]
collection=db["test"]

#CALL BACK -> Broker Connect
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("connected OK")
    else:
        print("Bad connection Returned code=", rc)

#CALL BACK -> Broker Disconnect
def on_disconnect(client, userdata, flags, rc=0):
    print(str(rc))

# Topic Subscribe 
def on_subscribe(client, userdata, mid, granted_qos):
    print("subscribed: " + str(mid) + " " + str(granted_qos))

#ArrivedMessege insert to MongoDB 
def on_message(client, userdata, msg):
    #print messege
    print(str(msg.payload.decode("utf-8")))
    post={"_id":192,"cpu_usage":str(msg.payload.decode("utf-8"))}
   #INSERT IN DB
    collection.insert_one(post)

# Create New Client
client = mqtt.Client()

# CALL BACK FUC SETTING
client.on_connect = on_connect
client.on_disconnect = on_disconnect
client.on_subscribe = on_subscribe
client.on_message = on_message

# address : localhost, port: 1883  connect
#INPUT YOUR BROKER IP AD PROT NUM
client.connect('192.168.0.29', 1883)

# INPUT Subscribe Topic
client.subscribe('mon/cpu', 1)
client.loop_forever()
