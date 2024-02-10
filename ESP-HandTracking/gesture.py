import requests
import numpy as np
import cv2
import mediapipe as mp
import time
import HandTrackingModule as htm


cap = cv2.VideoCapture('http://192.168.137.170:8080/video')

pTime = 0
cTime = 0
detector = htm.handDetector()
while True:
    #img = cv2.imdecode(np.frombuffer(requests.get(url="http://192.168.137.170:8080/shot.jpg").content , np.uint8), cv2.IMREAD_COLOR)
    ret , img = cap.read()
    img = detector.findHands(img, draw=True )
    lmList = detector.findPosition(img, draw=False)
    #if len(lmList) != 0:
        #print(lmList[4])

    cTime = time.time()
    fps = 1 / (cTime - pTime)
    pTime = cTime

    cv2.putText(img, str(int(fps)), (10, 70), cv2.FONT_HERSHEY_PLAIN, 3,(255, 0, 255), 3)

    cv2.imshow("Image", img)
    if(cv2.waitKey(1) == 27):
        exit()