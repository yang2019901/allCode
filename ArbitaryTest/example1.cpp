// #include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
// #include <cassert>
#include <chrono>

/*
    use <chrono> lib to do timing work
*/


using namespace std;
using namespace chrono;
using namespace cv;

const double PI = 3.14159265354;
// draw polygon according to given points set
void drawPolygon(Mat img, vector<Point>& poly)
{
    int I = poly.size();
    for (int i = 0; i < I; i++)
    {
        line(img, poly[i], poly[(i + 1) % I], Scalar(0,255,0), 2, 2);
    }
}

void drawPolygon(Mat src)
{
    int T;
    printf("number of edges: ");
    scanf("%d", &T);
    Point p;
    vector<Point> poly;
    printf("please enter %d sets of coordinates\n", T);
    for (int i = 0; i < T; i++)
    {
        printf(">>> ");
        cin >> p.x >> p.y;
        poly.push_back(p);
    }
    drawPolygon(src, poly);
    imshow("picture with polygen", src);
    waitKey(0);
}

void drawRotatedRect(Mat src, RotatedRect rotatedRect, Scalar scalar=Scalar(0,255,0), int thickness=2)
{
    Point2f pts[4];
    rotatedRect.points(pts);
    // for (int i = 0; i < 4; i++)
    // {
    //     line(src, pts[i], pts[(i + 1) % 4], scalar, thickness);
    // }
    vector<vector<Point>> contours(1);
    for (const Point2f& p : pts)
        contours[0].push_back(p);
    drawContours(src, contours, -1, scalar, thickness);
}

//void filterNoise(Mat& src, int thresholdValue, Size size)
//{
//
//    Mat srcGray, mask;
//    cvtColor(src, srcGray, COLOR_BGR2GRAY);
//    threshold(srcGray, mask, thresholdValue, 255, THRESH_BINARY);
//    Mat opstruct = getStructuringElement(MORPH_RECT, size);
//    dilate(mask, mask, opstruct);
//    erode(mask, mask, opstruct);
//    src = mask.clone();
//    // imshow("mask", mask);
//    imshow("src", src);
//    waitKey(0);
//}

//src is a binary matrix
void markEllipse(Mat src, vector<vector<Point>>& contours, double errorRatio=0.05,Scalar sca = Scalar(0,255,0), int thickness=1)
{
    int T = contours.size();
    double area = 0.0;
    double perimeter = 0.0;
    for (int i = 0; i < T; i++)
    {
        area = contourArea(contours[i]);
        perimeter = arcLength(contours[i], true);
        RotatedRect rect = minAreaRect(contours[i]);
        double width = rect.size.width;
        double height = rect.size.height;
        double a, b;
        a = max(width, height)/2;
        b = min(width, height)/2;
        double areaExpected = PI * a * b;
        //double periExpected = PI * (3*(a + b)/2 - sqrt(a * b));
        double periExpected = PI * sqrt(2 * (a * a + b * b));
        if (fabs((areaExpected-area) / areaExpected) < errorRatio && fabs(periExpected-perimeter) / periExpected < errorRatio)
            ellipse(src, rect, Scalar(0,0,255), 2);
    }
    return ;
}

void Test1(Mat img)
{
    imshow("原图", img);
    Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
    Mat dstImage;
    erode(img, dstImage, element);
    Mat dstImage1;
    blur(img, dstImage1, Size(7, 7));
    imshow("均值滤波", dstImage1);
    waitKey(0);
    destroyAllWindows();
    drawPolygon(img);
    return ;
}
void Test2(Mat src)
{
    Mat img, mask;
    vector<vector<Point>> contours;
    cvtColor(src, img, COLOR_BGR2GRAY);
    threshold(img, img, 200, 255, THRESH_BINARY);
    Mat opstruct = getStructuringElement(MORPH_RECT, Size(3, 3));
    dilate(img, img, opstruct);
    erode(img, img, opstruct);
    erode(img, img, opstruct);
    dilate(img, img, opstruct);
    imshow("imgafter dilate&erode", img);
    auto start = system_clock::now();
    findContours(img, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
    auto end = system_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    printf("find contours use %lld us", duration.count());
    int T = contours.size();
    cvtColor(img, img, COLOR_GRAY2BGR);
    img.copyTo(mask);
    drawContours(mask, contours, -1, Scalar(0, 255, 0), 1);
    imshow("mask&contours(original)", mask);
    // for (int i = 0; i < T; i++)
    // {
    //     Rect rect = boundingRect(contours[i]);
    //     rectangle(img, rect, Scalar(0, 255, 0), 1);
    // }

    // for (int i = 0; i < T; i++)
    // {
    //     RotatedRect rect = minAreaRect(contours[i]);
    //     drawRotatedRect(img, rect, Scalar(0, 255, 0), 2);
    // }
    
    // for (int i = 0; i < T; i++)        
    // {
    //     // 椭圆拟合要求contours[i]的点的数目多于4
    //     if (contours[i].size() <= 4)
    //         continue;
    //     RotatedRect rect = fitEllipse(contours[i]);
    //     ellipse(img, rect, Scalar(255,0,0), 2);
    // }
    
    // vector<vector<Point>> polyContours;
    // for (int i = 0; i < T; i++)
    // {
    //     vector<Point> poly;
    //     approxPolyDP(contours[i], poly, 3, true);
    //     polyContours.push_back(poly);
    // }
    // drawContours(img, polyContours, -1, Scalar(255,0,0), 1);
    markEllipse(img, contours, 0.1);
    imshow("img&contours", img);
    waitKey(0);
    destroyAllWindows();
    return;
}

void Test3(Mat src)
{   
    // 图片透视变换 perspective transform:
    vector<Point2f>src_coners(4);
    src_coners[0] = Point2f(13, 134);
    src_coners[1] = Point2f(271, 24);
    src_coners[2] = Point2f(180, 411);
    src_coners[3] = Point2f(398, 362);
    circle(src, src_coners[0], 3, Scalar(0, 0, 255), 3, 8);
    circle(src, src_coners[1], 3, Scalar(0, 0, 255), 3, 8);
    circle(src, src_coners[2], 3, Scalar(0, 0, 255), 3, 8);
    circle(src, src_coners[3], 3, Scalar(0, 0, 255), 3, 8);
    vector<Point2f>dst_coners(4);
    dst_coners[0] = Point2f(0, 300);
    dst_coners[1] = Point2f(0, 0);
    dst_coners[2] = Point2f(400, 300);
    dst_coners[3] = Point2f(400, 0);
    Mat warpMatrix = getPerspectiveTransform(src_coners, dst_coners); // 得到坐标系变换矩阵
    Mat dst;
    warpPerspective(src, dst, warpMatrix, dst.size(), INTER_LINEAR, BORDER_CONSTANT);  // 根据坐标系变换矩阵实现变换
    imshow("original picture", src);
    imshow("warp picture", dst);
    waitKey();
}

int main()
{
    string photoPath = "C:\\Users\\Lenovo\\Pictures\\Saved Pictures\\1.jpg";
    // LenovoWallPaper.jpg 
    // geometricalPicture2.jpg
    Mat img = imread(photoPath);
    resize(img, img, Size(960, 540));
    Test2(img);
    system("pause");
    return 0; 
}