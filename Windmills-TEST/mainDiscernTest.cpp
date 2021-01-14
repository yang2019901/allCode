/////////////////////////
///     DESERTED    /////
/////////////////////////

// high time cost. optimization failed!

#include <iostream>
#include "opencv2/opencv.hpp"
#include <math.h>
#include <chrono>

using namespace std;
using namespace cv;
using namespace chrono;


int main()
{
    VideoCapture cap("C:/Users/Lenovo/Desktop/VScode/Windmills-TEST/wind.mp4");
    Mat image,binary;
    int stateNum = 4;       
    int measureNum = 2;
    KalmanFilter KF(stateNum, measureNum, 0);
    //Mat processNoise(stateNum, 1, CV_32F);
    Mat measurement = Mat::zeros(measureNum, 1, CV_32F);
    KF.transitionMatrix = (Mat_<float>(stateNum, stateNum) << 1, 0, 1, 0,//A 状态转移矩阵
        0, 1, 0, 1,
        0, 0, 1, 0,
        0, 0, 0, 1);
    //这里没有设置控制矩阵B，默认为零
    setIdentity(KF.measurementMatrix);      //H=[1,0,0,0;0,1,0,0] 测量矩阵
    setIdentity(KF.processNoiseCov, Scalar::all(1e-5));         //Q高斯白噪声，单位阵
    setIdentity(KF.measurementNoiseCov, Scalar::all(1e-1));     //R高斯白噪声，单位阵
    setIdentity(KF.errorCovPost, Scalar::all(1));               //P后验误差估计协方差矩阵，初始化为单位阵
    randn(KF.statePost, Scalar::all(0), Scalar::all(0.1));      //初始化状态为随机值

    for(;;)
    {
        cap.read(image);
        // Test time cost:
        
        auto start = steady_clock::now();

        image.copyTo(binary);
        resize(image,image,Size(image.cols*0.5,binary.rows*0.5));
        resize(binary,binary,Size(binary.cols*0.5,binary.rows*0.5));

        //blue light discern:

        medianBlur(binary, binary, 3);
        // // 2B-G-R 计算蓝色程度

        // NOTE: Damn it! the double loops cost 3-4 ms in my machine(i5-9300H GTX1650)! Quit using it!
        Mat grey(binary.rows, binary.cols, CV_8UC1);
        for (int i = 0; i < binary.rows; i++)
        {
            for (int j = 0; j < binary.cols; j++)
            {
                // 获得像素点的颜色
                Vec3b &pixel_color = binary.at<Vec3b>(i, j);
                // 计算绿色程度
                int rate = pixel_color[0] * 2 - pixel_color[1] - pixel_color[2];
                if (rate < 0)
                    rate = 0;
                if (rate > 255)
                    rate = 255;
                grey.at<uchar>(i, j) = rate;
            }
        }
        medianBlur(grey,grey,3);
        // Mat hsvImage;
        // cvtColor(image, hsvImage, COLOR_BGR2HSV);
        // inRange(hsvImage,Scalar(100, 150,150),Scalar(124,255,255),binary);
        auto end = steady_clock::now();
        threshold(grey, binary, 150, 255, THRESH_BINARY);        //阈值要自己调
        imshow("blue discern", binary);
        Mat element = getStructuringElement(MORPH_RECT, Size(3,3));
        // morphologyEx(binary,binary,MORPH_CLOSE,element);
        dilate(binary,binary, element);

        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        Point2i center;
        // findContours only cost 1 ms in my machine (i5-9300H GTX1650)
        findContours(binary, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0,0));
        vector<int> contour(contours.size());   // all zeros 
        for (size_t i = 0; i < contours.size(); i++)
            if (hierarchy[i][3] != -1)
                contour[hierarchy[i][3]]++; 
        for (size_t i = 0; i < contour.size();i++)
        {
            if (contour[i] == 1)
            {
                int num = hierarchy[i][2];
                RotatedRect box = minAreaRect(contours[num]);
                Point2f vertex[4];
                box.points(vertex);
                for (int i = 0; i < 4; i++)
                    line(image, vertex[i], vertex[(i+1)%4], Scalar(0,255,0),2, LINE_AA);
                center = (vertex[0] + vertex[2]) / 2;
                // putText(image, "target", vertex[0], FONT_HERSHEY_SIMPLEX, 1.0, Scalar(255,255,0));
                try
                {
                    cv::circle(image, Point(center.x, center.y), 15, cv::Scalar(0, 0, 255), 4); //circle the target
                    Mat prediction = KF.predict();
                    Point predict_pt = Point((int)prediction.at<float>(0), (int)prediction.at<float>(1));
                    measurement.at<float>(0) = (float)center.x;
                    measurement.at<float>(1) = (float)center.y;
                    KF.correct(measurement);

                    circle(image, predict_pt, 3, Scalar(34, 255, 255), -1);  //yellow dot indicates the predicted position

                    center.x = (int)prediction.at<float>(0);
                    center.y = (int)prediction.at<float>(1);
                }catch(exception e){
                    cout << "Kalman filter failed\n";
                }
            }
        }
        
        auto duration = duration_cast<microseconds>(end - start);
        printf("time cost: %lf ms\n", duration.count()/1000.0);
        imshow("frame",binary);
        imshow("Original", image);
        if(waitKey(0) == 'q')
            break;
    }
}