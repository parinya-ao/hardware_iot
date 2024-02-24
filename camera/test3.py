import requests

img_data = requests.get("http://10.2.6.215/capture").content
with open('img3.jpg', 'wb') as handler:
    handler.write(img_data)
