import requests
import cv2
import time

def gstreamer_pipeline(
    sensor_id=0,
    capture_width=1920,
    capture_height=1080,
    display_width=960,
    display_height=540,
    framerate=30,
    flip_method=0,
):
    return (
        "nvarguscamerasrc sensor-id=%d !"
        "video/x-raw(memory:NVMM), width=(int)%d, height=(int)%d, framerate=(fraction)%d/1 ! "
        "nvvidconv flip-method=%d ! "
        "video/x-raw, width=(int)%d, height=(int)%d, format=(string)BGRx ! "
        "videoconvert ! "
        "video/x-raw, format=(string)BGR ! appsink"
        % (
            sensor_id,
            capture_width,
            capture_height,
            framerate,
            flip_method,
            display_width,
            display_height,
        )
    )

cap = cv2.VideoCapture(0, cv2.CAP_DSHOW)
# cap = cv2.VideoCapture(gstreamer_pipeline(flip_method=0, framerate=30), cv2.CAP_GSTREAMER)

while True:
    success, img = cap.read()
    if success:    
        # cv2.imshow("OUTPUT", img)
        time.sleep(0.005)
        _, imdata = cv2.imencode('.JPG', img)

        print('.', end='', flush=True)

        r = requests.put('http://45.79.77.67:5000/upload', data=imdata.tobytes())

        print(r)

    if cv2.waitKey(5) == 27:  # 5ms = 200 frames per second (1000ms/5ms) 
        break

cv2.destroyAllWindows()
cap.release()