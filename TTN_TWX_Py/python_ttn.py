import time
import ttn
import requests
import json

app_id = "dht_temp_hum"
access_key =  "ttn-account-v2.hEUmyFWtEIvZNSxqg03YC8WxNPP-u08UMoKdzYSxUUY"

twx_temperature = 0
twx_humidity = 0

def uplink_callback(msg, client):
  print("Received uplink from ", msg.dev_id)
  print(msg.payload_fields)
  print(msg.payload_fields.Temperature)
  print(msg.app_id)
  twx_temperature = msg.payload_fields.Temperature
  twx_humidity = msg.payload_fields.Humidity
  url = 'http://192.168.225.104:8080/Thingworx'
  headers = { 'Content-Type': 'application/json', 'appKey': '44d70cce-3986-4139-ae57-39dbf36a19f4' }
  payload = { 'TemperatureData': twx_temperature, 'HumidityData': twx_humidity }
  response = requests.put(url + '/Things/LORAThing/Services/getTemperatureData?', headers=headers, json=payload, verify=False)
  print(response)




handler = ttn.HandlerClient(app_id, access_key)

# using mqtt client
mqtt_client = handler.data()
mqtt_client.set_uplink_callback(uplink_callback)
mqtt_client.connect()
time.sleep(60)
mqtt_client.close()

# using application manager client
app_client =  handler.application()
my_app = app_client.get()
print(my_app)
my_devices = app_client.devices()
print(my_devices)
