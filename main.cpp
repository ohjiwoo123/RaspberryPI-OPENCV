#include "opencv4/opencv2/opencv.hpp"
// #include <iostream>
#include <stdio.h>

using namespace cv;

int main()
{
    printf("Hello OpenCV\n");
    // 이미지를 저장하기 위한 MAT Class 객체 선언
    Mat img;

    // image 불러오는 함수 (1이면 칼라 0이면 흑백)
    img = imread("lenna.bmp",1);
    
    // 이미지 파일을 읽었는데 Mat img가 비어 있다면
    if (img.empty())
    {
        perror("Image load failed!\n");
        return -1;
    } 

    // 새로운 창을 띄운다. 창의 이름이 image 
    namedWindow("image");
    // image 창에 img 이미지를 보여준다
    imshow("image", img);

    // 괄호 안에 아무런 도 없으면 아무키나 입력 받기 전까지 대기 
    waitKey();

    return 0;
}