#include <iostream>
#include "opencv2/opencv.hpp"
#include <math.h>


using namespace std;
using namespace cv;


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
        
        image.copyTo(binary);
        resize(image,image,Size(image.cols*0.5,binary.rows*0.5));
        resize(binary,binary,Size(binary.cols*0.5,binary.rows*0.5));

        //blue light discern:

        medianBlur(binary, binary, 3);
        // 2B-G-R 计算蓝色程度
        Mat grey(binary.rows, binary.cols, CV_8UC1);
        for (int i = 0; i < binary.rows; i++)
        {
            for (int j = 0; j < binary.cols; j++)
            {
                // 获得像素点的颜色
                Vec3b &pixel_color = binary.at<Vec3b>(i, j);
                // 计算绿色程度
                int rate = pixel_color[0] * 1.5 - 0.5 * pixel_color[1] - pixel_color[2];
                if (rate < 0)
                    rate = 0;
                if (rate > 255)
                    rate = 255;
                grey.at<uchar>(i, j) = rate;
            }
        }
        medianBlur(binary,binary,3);
        threshold(grey, binary, 120, 255, THRESH_BINARY);        //阈值要自己调
        imshow("blue discern", grey);
        Mat element = getStructuringElement(MORPH_RECT, Size(3,3));
        morphologyEx(binary,binary,MORPH_DILATE,element);

        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        Point2i center;
        findContours(binary, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE, Point(0,0));
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
                putText(image, "target", vertex[0], FONT_HERSHEY_SIMPLEX, 1.0, Scalar(255,255,0));
                try
                {
                    cv::circle(image, Point(center.x, center.y), 15, cv::Scalar(0, 0, 255), 4); //circle the target
                    Mat prediction = KF.predict();
                    Point predict_pt = Point((int)prediction.at<float>(0), (int)prediction.at<float>(1));
                    measurement.at<float>(0) = (float)center.x;
                    measurement.at<float>(1) = (float)center.y;
                    KF.correct(measurement);

                    circle(image, predict_pt, 3, Scalar(34, 255, 255), -1);

                    center.x = (int)prediction.at<float>(0);
                    center.y = (int)prediction.at<float>(1);
                }catch(exception e){
                    cout << "Kalman filter failed\n";
                }
            }
        }
        { 
        // for (size_t i = 0; i < contours.size(); i++)
        // {

        //     vector<Point> points;
        //     double area = contourArea(contours[i]);
        //     if (area < 50 || 1e4 < area) continue;
        //     drawContours(image, contours, static_cast<int>(i), Scalar(0), 2);

        //     points = contours[i];
        //     RotatedRect rrect = fitEllipse(points);
        //     cv::Point2f* vertices = new cv::Point2f[4];
        //     rrect.points(vertices);

        //     float aim = rrect.size.height/rrect.size.width;
        //     if(aim > 1.7 && aim < 2.6){
        //         for (int j = 0; j < 4; j++)
        //         {
        //             cv::line(binary, vertices[j], vertices[(j + 1) % 4], cv::Scalar(0, 255, 0),4);
        //         }
        //         float middle = 100000;

        //         for(size_t j = 1;j < contours.size();j++){

        //             vector<Point> pointsA;
        //             double area = contourArea(contours[j]);
        //             if (area < 50 || 1e4 < area) continue;

        //             pointsA = contours[j];

        //             RotatedRect rrectA = fitEllipse(pointsA);

        //             float aimA = rrectA.size.height/rrectA.size.width;

        //             if(aimA > 3.0){
        //             float distance = sqrt((rrect.center.x-rrectA.center.x)*(rrect.center.x-rrectA.center.x)+
        //                                   (rrect.center.y-rrectA.center.y)*(rrect.center.y-rrectA.center.y));

        //             if (middle > distance  )
        //                 middle = distance;
        //             }
        //         }
        //         if( middle > 60){                               //这个距离也要根据实际情况调,和图像尺寸和物体远近有关。
        //             cv::circle(binary,Point(rrect.center.x,rrect.center.y),15,cv::Scalar(0,0,255),4);  //circle the target 
        //             Mat prediction = KF.predict();
        //             Point predict_pt = Point((int)prediction.at<float>(0), (int)prediction.at<float>(1));

        //             measurement.at<float>(0) = (float)rrect.center.x;
        //             measurement.at<float>(1) = (float)rrect.center.y;
        //             KF.correct(measurement);

        //             circle(binary, predict_pt, 3, Scalar(34, 255, 255), -1);

        //             rrect.center.x = (int)prediction.at<float>(0);
        //             rrect.center.y = (int)prediction.at<float>(1);

        //         }
        //     }
        // }
        }

        imshow("frame",binary);
        imshow("Original", image);
        if(waitKey(30) == 'q')
            break;
    }
}