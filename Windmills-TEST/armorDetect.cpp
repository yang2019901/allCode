#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>

using namespace std;
using namespace cv;

const bool BLUE = false;
const bool RED = true;

// 对于wind.mp4 对中心标志R的识别效果很好
bool findMillCenter(Mat src, const bool ColorFlag, Point &center)
{
    if (src.empty())
    {
        printf("src is empty\n");
        return false;
    }
    vector<Mat> splited;
    split(src, splited);
    Mat temp;
    if (ColorFlag == BLUE)
    {
        subtract(splited[0], splited[2], temp);
    }
    else
    {
        subtract(splited[2], splited[0], temp);
    }
    threshold(temp, temp, 150, 255, THRESH_BINARY);
    dilate(temp, temp, Mat());
    dilate(temp, temp, Mat());
    vector<vector<Point>> contours;
    findContours(temp, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    for (int i = 0; i < contours.size(); i++)
    {
        double area = contourArea(contours[i]);
        Rect rectCon = boundingRect(contours[i]);
        float ratio = float(rectCon.height) / rectCon.width;
        float AreaError = fabs(area - rectCon.area()) / area;
        if (area < 2e-4 * src.cols * src.rows || AreaError > 0.5 || fabs(ratio - 1) > 0.1) // 面积小于全图的万分之二，和噪点一个数量级，就很难区分了，故标记为识别失败。
        {
            contours.erase(i + contours.begin());
            i--;
            continue;
        }
    }
    if (contours.empty())
        return false;
    Rect MarkR = boundingRect(contours[0]); //草率地取了第一个，实际上应该考虑多个轮廓符合条件的情形
    center.x = MarkR.x + MarkR.width / 2;
    center.y = MarkR.y + MarkR.height / 2;
    return true;
}

// trimRegion 对于wind1.jpg生效
void trimRegion(Mat src, Rect &region)
{
    region.x = max(0, region.x);
    region.y = max(0, region.y);
    region.width = min(region.width, src.cols - region.x);
    region.height = min(region.height, src.rows - region.y);
    return;
}

RotatedRect armorDetect(Mat src, const bool ColorFlag)
{
    if (src.empty())
    {
        printf("src is empty\n");
        return RotatedRect();
    }
    vector<Mat> splited;
    split(src, splited);
    Mat temp;
    medianBlur(src, temp, 3);
    if (ColorFlag == BLUE)
    {
        subtract(splited[0], splited[2], temp);
    }
    else
    {
        subtract(splited[2], splited[0], temp);
    }
    dilate(temp, temp, Mat());
    medianBlur(temp, temp, 3);
    threshold(temp, temp, 120, 255, THRESH_BINARY);
    Mat element = getStructuringElement(MORPH_RECT, Size(3,3));
    morphologyEx(temp, temp, MORPH_CLOSE, element, Point(0,0), 1);

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    Point2i center;
    // findContours only cost 1 ms in my machine (i5-9300H GTX1650)
    findContours(temp, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
    vector<int> contour(contours.size()); // all zeros
    for (size_t i = 0; i < contours.size(); i++)
        if (hierarchy[i][3] != -1)
            contour[hierarchy[i][3]]++;
    for (size_t i = 0; i < contour.size(); i++)
    {
        if (contour[i] == 1)
        {
            return minAreaRect(contours[hierarchy[i][2]]);
        }
    }
    return RotatedRect();
}

Rect centerRoi(Mat src, const bool ColorFlag, Point &center)
{
    if (src.empty())
    {
        return Rect(0, 0, src.cols, src.rows);
    }
    Point2f vertex[4];
    armorDetect(src, ColorFlag).points(vertex);
    int armlength = 0;
    for (const Point &ver : vertex)
    {
        armlength = MAX(armlength, sqrt(pow(ver.x - center.x, 2) + pow(ver.y - center.y, 2)) + 20);
        // explanation: add extra number to ensure the calculating error will be offset
    }
    Rect Roi(center.x - armlength, center.y - armlength, 2 * armlength, 2 * armlength);
    trimRegion(src, Roi);
    return Roi;
}

int main()
{
    VideoCapture cap("C:/Users/Lenovo/Desktop/VScode/Windmills-TEST/wind.mp4");
    Mat p1;
    // p1 = imread("C:/Users/Lenovo/Desktop/VScode/Windmills-TEST/wind4.jpg");
    while (true)
    {
        cap >> p1;
        if (p1.empty())
            break;
        resize(p1, p1, Size(640, 480));
        Point center;
        if (findMillCenter(p1, BLUE, center))
        { 
            circle(p1, center, 0, Scalar(0, 255, 255), 5);
            Rect Roi = centerRoi(p1, BLUE, center);
            rectangle(p1, Roi, Scalar(255,0,255),2);
        }
        imshow("marked frame", p1);
        if (waitKey(0) == 'q')
            break;
    }
    system("pause");
}