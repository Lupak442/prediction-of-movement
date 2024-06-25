from ultralytics import YOLO
from ultralytics.yolo.v8.detect.predict import  DetectionPredictor
import numpy as np
import matplotlib.pyplot as plt
import cv2
from matplotlib.animation import FuncAnimation
import math
import serial
import cv2
import time

ser = serial.Serial(port='COM5', baudrate=9600, stopbits=1, bytesize=8, timeout=0.1)
ser.isOpen()
data='1'
count=0

model = YOLO("yolov8n-face.pt")

cap = cv2.VideoCapture(1)
#C:C:/Users/kysgk/Downloads/OpenCV_Python_2ndEdition_Examples/data/run3.mp4
#lst = [0, 0, 0, 0]
lst = [0 for ii in range(4)]
lst_x = []
lst_y = []
lst_w = []
lst_h = []
lst_x_y=[]
jin_x = 0
i = 0
x_i = 0
x_x = 0
y_y = 0
last_x=[0,0,0,0,0]
last_y=[0,0,0,0,0]
stop = 0
frame_index=0
points=[]
x = np.array([0, 0, 0, 0, 0])
y = np.array([0, 0, 0, 0, 0])
a = np.array([0, 0, 0, 0, 0])
b = np.array([0, 0, 0, 0, 0])

def do_something_with_coords(lst_x, lst_y):
    print(lst_x[0:5])
    print(lst_y[0:5])
    err_x = x[0] - x[1]
    err_y = y[0] - y[1]
    if (-12 < err_x < 12) and (-12 < err_y < 12):
        print("멈춤 상태")
        stop = 1

    np.put(x,[0,1,2,3,4],lst_x)
    np.put(y, [0, 1, 2, 3, 4], lst_y)
    coefficients = np.polyfit(x, y, 2, rcond=0.01)
    p = np.poly1d(coefficients)

    x_range = np.arange(0, 1000, 1)
    y_values = np.polyval(coefficients, x_range)
    for i in range(len(x_range)):
        x_x = int(x_range[i])
        y_y = int(y_values[i])
        cv2.circle(annotated_frame, (x_x, y_y), 5, (255, 0, 0), -1)
        #if y_y > 635 and y_y <640:
        #    jin_x = x_x
        #    print(jin_x)





    #xp = np.linspace(0, 800, 10)
    #plt.figure(figsize=(10, 10))
    #plt.plot(x, y, 'o', xp, p(xp), '-')
    #plt.show()

    # animation = FuncAnimation(plt, (lst_x,lst_y), fargs=(x, y), interval=1)
    # plt.show()

    #cv2.line(annotated_frame, (0,0), (100,100), (0, 0, 255), 10)
    #cv2.line(annotated_frame, (300,300), (500,500), (0, 255, 0), 10)
    #cv2.line(annotated_frame, (200,200), (600,200), (125, 0, 0), 10)
    #cv2.line(annotated_frame, (int(lst_x[3]), int(lst_y[3])), (int(lst_x[4]), int(lst_y[4])), (0, 0, 0), 10)


# Loop through the video frames
while cap.isOpened():
    # Read a frame from the video
    success, frame = cap.read()

    if success:

        # Run YOLOv8 inference on the frame
        results = model.predict(source=frame, classes=0, conf=0.5)

        # Visualize the results on the frame
        annotated_frame = results[0].plot()  # results 중에서 결과값에서 human을 찾아서 사용


        # Display the annotated frame

        # print(box.xywh)
        if frame_index == 2:

            try:  # 아니면 새롭게 클래스 1개 사람만 학습시키는것이 나을수도 있음
                boxes = results[0].boxes  # results[0] 첫번째 로 인식된 객체
                box = boxes[0]  # 출력용 사용하려면 변수에 저장
                lst = box.xyxy


                    # lst = 0
                    # lst_x = 0
                    # lst_y = 0

            except IndexError:
                print('the array is empty')

            if ((lst[0][3].item() + lst[0][1].item()) // 2) < 400:
                lst_x.append((lst[0][2].item() + lst[0][0].item()) // 2)
                lst_y.append((lst[0][3].item() + lst[0][1].item()) // 2)
                # if ((lst[0][3].item() + lst[0][1].item()) // 2) < 400:

                if len(lst_y) > 5:
                    lst_x = lst_x[-5:]
                    lst_y = lst_y[-5:]
                    plt.close()

                    do_something_with_coords(lst_x, lst_y)

                # elif ((lst[0][3].item() + lst[0][1].item()) // 2) > 300:
            elif 400<=((lst[0][3].item() + lst[0][1].item()) // 2) <500:
                last_x = lst_x
                last_y = lst_y
                np.put(a, [0, 1, 2, 3, 4], last_x)
                np.put(b, [0, 1, 2, 3, 4], last_y)
                print(a)
                coefficients = np.polyfit(a, b, 2, rcond=0.01)
                p_l = np.poly1d(coefficients)
                print("2차 다항식: ", p_l)
                # print("예측 좌표값: ", 500 ,p(500))
                # print("예측 좌표값: ", 800, p(800))

                x_range_l = np.arange(-1000, 1000, 1)
                y_values_l = np.polyval(coefficients, x_range_l)
                for ii in range(len(x_range_l)):
                    x_x_l = int(x_range_l[ii])
                    y_y_l = int(y_values_l[ii])
                    cv2.circle(annotated_frame, (x_x_l, y_y_l), 5, (0, 0, 255), -1)
                    if 600<y_y_l<640:
                        print('jin_x: ', x_x_l, y_y_l)

                        if 0<x_x_l<300:
                            print('문 열기')

                            ser.write(data.encode())


            else :
                last_x = 0
                last_y = 0


            frame_index = 0

        else:
            frame_index += 1

            # Break the loop if 'q' is pressed
        cv2.imshow("YOLOv8 Inference", annotated_frame)


        if cv2.waitKey(1) & 0xFF == ord("q"):
            break
    else:
        # Break the loop if the end of the video is reached
        break

# Release the video capture object and close the display window
cap.release()
ser.close()
#ser.close()
cv2.destroyAllWindows()

