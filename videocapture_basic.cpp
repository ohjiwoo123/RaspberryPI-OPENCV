#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;

int main(int, char**)
{
    // 카메라에서 이미지 한 장을 저장하기 위한 이미지 객체 
    Mat frame;

    //--- INITIALIZE VIDEOCAPTURE
    // cap = fd
    VideoCapture cap;

    // open the default camera using default API
    // cap.open(0);
    // OR advance usage: select any API backend

    // 카메라 갯수가 증가하면 0부터 1씩 증가 
    int deviceID = 0;             // 0 = open default camera
    int apiID = cv::CAP_V4L2;      // 0 = autodetect default API
    // open selected camera using selected API

    // STEP 1. 카몌라 장치열기 
    cap.open(deviceID, apiID);
    // check if we succeeded
    if (!cap.isOpened()) {
        perror("ERROR! Unable to open camera\n");
        return -1;
    }

    //--- GRAB AND WRITE LOOP
    printf("Start grabbing\n");
    printf("Press any key to terminate\n");

    while(1)
    {
        // wait for a new frame from camera and store it into 'frame'
        // 카메라에서 매 프레임마다 읽기 
        cap.read(frame);
        // check if we succeeded
        if (frame.empty()) {
            perror("ERROR! blank frame grabbed\n");
            break;
        }

        // "Live" 라는 창을 썡썽하꼬 끄 창예 frame 이미지를 보여준다. 
        // show live and wait for a key with timeout long enough to show images
        // waitkey 숫자가 클수록 프레임이 끊긴다.
        imshow("Live", frame);
        if (waitKey(5) >= 0)
            break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
