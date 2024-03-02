import requests
import cv2
from time import sleep 

# from umqtt.simple import MQTTClient

imagePath = r'img3.jpg'
cascPath = "haarcascade_frontalface_default.xml"

token = 'z6kZjpcYQj3xiwnPU5hr4DFBvVmY3tPAFloQieY3DUT'
payload = {'message' : 'found someone'
          ,'notificationDisabled' : False}

while True :
    img_data = requests.get("http://10.2.6.215/capture").content
    with open('img3.jpg', 'wb') as handler:
        handler.write(img_data)
    image = cv2.imread(imagePath)
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    faceCascade = cv2.CascadeClassifier(cascPath)
    faces = faceCascade.detectMultiScale(gray)
    print(f'There are {len(faces)} faces found.')
    if len(faces)>=1:
        r = requests.post('https://notify-api.line.me/api/notify'
                , headers={'Authorization' : 'Bearer {}'.format(token)}
                , params = payload
                ,files = {'imageFile': open(imagePath, 'rb')})
    sleep(1)