#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
#include <chrono>

using namespace std;
using namespace cv;
using namespace chrono;

const bool BLUE = false;
const bool RED = true;

#define TIMING true

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

// 对于wind.mp4 绝大多数帧识别良好，除了用手遮挡的那几帧。同时需要注意，它不能区分已击打和未击打的装甲板
// 策略：Accuracy first!
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

// 对于wind.mp4 roi卡范围的效果很好
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


// 策略：Efficiency first!
// mode=0:使用内嵌矩形的方式进行识别
// mode=1:使用面积比，距离的方式进行识别(to be added)
RotatedRect targetDetect(Mat roi, const bool ColorFlag, int mode)
{
    if (roi.empty())
    {
        printf("roi is empty\n");
        return RotatedRect();
    }
    if (mode == 0)
    {
        Mat gray;
        Mat splited[3];
        split(roi, splited);
        if (ColorFlag == BLUE)
        {
            subtract(splited[0], splited[2], gray);
        }
        else 
        {
            subtract(splited[2], splited[0], gray);
        }
        // 通道相减法对 wind.mp4 效果极佳!

        Mat element = getStructuringElement(MORPH_RECT, Size(3,3));
        morphologyEx(gray, gray, MORPH_CLOSE, element, Point(0,0), 1); // 注意：MORPH_CLOSE迭代次数过多会导致矩形选区的偏移（将"1"改为"2"已经产生明显误差了）
        threshold(gray, gray, 80, 255, THRESH_BINARY);
        // presume all contours have been closed so far 

        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(gray, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0,0));
        vector<int> contour(contours.size()); // all zeros
        for (size_t i = 0; i < contours.size(); i++)
            if (hierarchy[i][3] != -1)
                contour[hierarchy[i][3]]++;
        for (size_t i = 0; i < contour.size(); i++)
        {
            if (contour[i] == 1 || contour[i] == 2)
            {
                int target = hierarchy[i][2];   
                if (contourArea(contours[target]) <= 25)
                    continue;
                // else: presume "target" is truely what we are looking for
                return minAreaRect(contours[target]);
            }
        }
        return RotatedRect();
    }
    else if (mode == 1)
    {
        Mat gray;
        cvtColor(roi, gray, COLOR_BGR2GRAY);           // around 1ms sometimes 1.5ms
        threshold(gray, gray, 80, 255, THRESH_BINARY); // no time   //阈值要自己调
        // auto end = system_clock::now();

        dilate(gray, gray, Mat()); // sometimes 1ms about these two operations
        dilate(gray, gray, Mat());

        floodFill(gray, Point(5, 50), Scalar(255), 0, FLOODFILL_FIXED_RANGE); // around 1ms

        threshold(gray, gray, 80, 255, THRESH_BINARY_INV);
        imshow("mode2 gray", gray);
        vector<vector<Point>> contours;
        findContours(gray, contours, RETR_LIST, CHAIN_APPROX_NONE); // sometimes 1ms

        // the "for" loop overall costs 1 ms
        for (size_t i = 0; i < contours.size(); i++)
        {

            vector<Point> points;
            double area = contourArea(contours[i]);
            if (area < 50 || 1e4 < area)
                continue;
            // drawContours(gray, contours, static_cast<int>(i), Scalar(0), 2);
            points = contours[i];
            RotatedRect rrect = fitEllipse(points);
            Point2f *vertices = new Point2f[4];
            rrect.points(vertices);

            float aim = rrect.size.height / rrect.size.width;
            if (aim > 1.7 && aim < 2.6)
            {
                float middle = 100000;
                for (size_t j = 1; j < contours.size(); j++)
                {

                    vector<Point> pointsA;
                    double area = contourArea(contours[j]);
                    if (area < 50 || 1e4 < area)
                        continue;

                    pointsA = contours[j];

                    RotatedRect rrectA = fitEllipse(pointsA);

                    float aimA = rrectA.size.height / rrectA.size.width;

                    if (aimA > 3.0)
                    {
                        float distance = sqrt((rrect.center.x - rrectA.center.x) * (rrect.center.x - rrectA.center.x) +
                                              (rrect.center.y - rrectA.center.y) * (rrect.center.y - rrectA.center.y));

                        if (middle > distance)
                            middle = distance;
                    }
                }
                if (middle > 60)
                    return rrect;
            }
        }
        return RotatedRect();
    }

    // else: set mode wrong 
    else
    {
        printf("no such mode");
        return RotatedRect();
    }
}

int main()
{
    VideoCapture cap("C:/Users/Lenovo/Desktop/VScode/Windmills-TEST/wind.mp4");
    Mat p1;
    // p1 = imread("C:/Users/Lenovo/Desktop/VScode/Windmills-TEST/wind4.jpg");
    Point2f vertices[4];
    
    while (true)
    {
        cap >> p1;
        auto start = system_clock::now();
        if (p1.empty())
            break;
        resize(p1, p1, Size(640, 480));
        Point center;
        Rect Roi(0,0,p1.cols,p1.rows);
        if (findMillCenter(p1, BLUE, center))
        { 
            circle(p1, center, 0, Scalar(0, 255, 255), 5);
            Roi = centerRoi(p1, BLUE, center);
            rectangle(p1, Roi, Scalar(255,0,255),2);
        }

        Mat roi = Mat(p1, Roi);
        targetDetect(roi, BLUE, 0).points(vertices);
        for (int i = 0; i < 4; i++)
        {
            line(roi, vertices[i], vertices[(i+1)%4], Scalar(0,255,255),2);
        }

        auto end = system_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        printf("time cost: %lf ms\n", duration.count() / 1000.0);
        imshow("marked frame", p1);
        if (waitKey(0) == 'q')
            break;
    }
    system("pause");
}