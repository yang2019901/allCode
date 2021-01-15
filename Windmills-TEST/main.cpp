//////////////////////////////////////////////////////////////////
//// Important: try best not to use copying function.
////    Instead, initialze Mat object by calling it 
////    as "dstImage" of function like "cvtColor",etc.
//////////////////////////////////////////////////////////////////


// TODO 结合该实例中的kalman预测（defective!只能跟随而不能预测），理解kalman各个参数的含义和初始化方法
#include <iostream>
#include <opencv2/opencv.hpp>
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
    setIdentity(KF.measurementMatrix);//H=[1,0,0,0;0,1,0,0] 测量矩阵
    setIdentity(KF.processNoiseCov, Scalar::all(1e-5));//Q高斯白噪声，单位阵
    setIdentity(KF.measurementNoiseCov, Scalar::all(1e-1));//R高斯白噪声，单位阵
    setIdentity(KF.errorCovPost, Scalar::all(1));//P后验误差估计协方差矩阵，初始化为单位阵
    randn(KF.statePost, Scalar::all(0), Scalar::all(0.1));//初始化状态为随机值

    for(;;)
    {
        cap.read(image);
        Mat binary;

        auto start = steady_clock::now();
        resize(image,image,Size(640,480));   // costs 2ms
        cvtColor(image,binary,COLOR_BGR2GRAY);      // around 1ms sometimes 1.5ms
        threshold(binary,binary, 80, 255, THRESH_BINARY);  // no time   //阈值要自己调
        // auto end = steady_clock::now();

        dilate(binary,binary,Mat());   // sometimes 1ms about these two operations
        dilate(binary,binary,Mat());

        floodFill(binary,Point(5,50),Scalar(255),0,FLOODFILL_FIXED_RANGE);// around 1ms

        threshold(binary, binary, 80, 255, THRESH_BINARY_INV);
        vector<vector<Point>> contours;
        findContours(binary, contours, RETR_LIST, CHAIN_APPROX_NONE); // sometimes 1ms
        
        // the "for" loop overall costs 1 ms
        for (size_t i = 0; i < contours.size(); i++){

            vector<Point> points;
            double area = contourArea(contours[i]);
            if (area < 50 || 1e4 < area) continue;
            drawContours(binary, contours, static_cast<int>(i), Scalar(0), 2);

            points = contours[i];
        
            RotatedRect rrect = fitEllipse(points);
            cv::Point2f* vertices = new cv::Point2f[4];
            rrect.points(vertices);

            float aim = rrect.size.height/rrect.size.width;
            if(aim > 1.7 && aim < 2.6){
                for (int j = 0; j < 4; j++)
                {
                    cv::line(image, vertices[j], vertices[(j + 1) % 4], cv::Scalar(0, 255, 0),4);
                }
                float middle = 100000;

                for(size_t j = 1;j < contours.size();j++){

                    vector<Point> pointsA;
                    double area = contourArea(contours[j]);
                    if (area < 50 || 1e4 < area) continue;

                    pointsA = contours[j];

                    RotatedRect rrectA = fitEllipse(pointsA);

                    float aimA = rrectA.size.height/rrectA.size.width;

                    if(aimA > 3.0){
                    float distance = sqrt((rrect.center.x-rrectA.center.x)*(rrect.center.x-rrectA.center.x)+
                                          (rrect.center.y-rrectA.center.y)*(rrect.center.y-rrectA.center.y));

                    if (middle > distance  )
                        middle = distance;
                    }
                }
                if( middle > 60){                               //这个距离也要根据实际情况调,和图像尺寸和物体远近有关。
                    cv::circle(image,Point(rrect.center.x,rrect.center.y),15,cv::Scalar(0,0,255),4);
                    Mat prediction = KF.predict();
                    Point predict_pt = Point((int)prediction.at<float>(0), (int)prediction.at<float>(1));

                    measurement.at<float>(0) = (float)rrect.center.x;
                    measurement.at<float>(1) = (float)rrect.center.y;
                    KF.correct(measurement);

                    circle(image, predict_pt, 3, Scalar(34, 255, 255), -1);

                    rrect.center.x = (int)prediction.at<float>(0);
                    rrect.center.y = (int)prediction.at<float>(1);

                }
            }
        }

        auto end = steady_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        printf("time cost: %lf ms\n", duration.count() / 1000.0);

        imshow("frame",image);
        imshow("Original", binary);
        if (waitKey(10) == 'q')
            break;
    }
    system("pause");
}
