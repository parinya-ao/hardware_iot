import cv2

imagePath = r'img1.png'
cascPath = "haarcascade_frontalface_default.xml"

faceCascade = cv2.CascadeClassifier(cascPath)

image = cv2.imread(imagePath)

gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

faces = faceCascade.detectMultiScale(gray)

x, y, w, h = faces[0]

cv2.rectangle(image, (x, y), (x+w, y+h), (0, 255, 0), 2)