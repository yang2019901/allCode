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

const bool DEG = false;
const bool RAD = true;

const int CONSTMODE = 0;    // 小能量机关模式
const int SINMODE = 1;      // 大能量机关模式
const int CONSTSPEED = 60;  // 小能量机关模式下，机关旋转的角速度。单位: deg/s   

const int UNKNOWN = 0;
const int COUNTERCLOCKWISE = 1;
const int CLOCKWISE = -1;

#ifndef PI
#define PI 3.1415927
#endif

#ifndef RAD2DEG
#define RAD2DEG(rad) ((rad) * 180 / PI) 
#endif

#ifndef DEG2RAD
#define DEG2RAD(deg) ((deg) * PI / 180)
#endif

#define TIMING true
#define DISTANCE(p1, p2) sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2))

// NOTE: ROI算法的前提是相机参考系与大符参考系完全相对静止，这就要求不能使用安装在枪管上的相机，不然图片会随之而动，导致ROI设置失败
// TODO: 增加斜侧向打符的功能，主要用于干扰对方（想法：仿射变换 or 透视变换）
class MillHiter
{
protected:
    Rect _roi;                  // 矩形区域 ROI （如果已经找到的话）
    Point2f _centerR;           // 中心点 R 的坐标 （如果已经找到的话）
    vector<Point2f> _sampleR;   // 取“聚点”作为大风车中心点的坐标
    vector<float> _angle;       // 记录前几帧扇叶的角度，以便计算扇叶的旋转方向
    bool _roiAvail;             // ROI是否有效（找到）
    bool _centerRAvail;         // 中心点R坐标是否有效（找到）
    bool _colorFlag;            // 要击打的能量机关的颜色
    int _spinDir;               // 能量机关旋转的方向

public:
    /* 取消默认构造函数，如果没有colorFlag（也即不输入打哪种颜色的装甲），就报错，终止运行 */
    // MillHiter() : _roiAvail(false), _centerRAvail(false), _colorFlag(BLUE), _spinDir(UNKNOWN){};
    MillHiter(bool colorFlag) : _colorFlag(colorFlag), _roiAvail(false), _centerRAvail(false), _spinDir(UNKNOWN){};
    MillHiter(Rect roi, Point2f centerR, bool colorFlag): _roi(roi), _centerR(centerR), _roiAvail(true), _centerRAvail(true), _colorFlag(colorFlag), _spinDir(UNKNOWN){};

    bool targetDetect(Mat roi, RotatedRect& target, int mode);
    bool targetLock(Mat src, Point2f& aim, int mode=0, int SampleSize=10, double DistanceErr=7.0, double nearbyPercentage=0.8);
    bool findMillCenter(Mat src, Point2f &center) const;
    double getAngle(const Point2f& center, const Point2f& pos, bool unit = RAD) const;
    double getAngularSpeed(const Point2f& center, const Point2f& nowPos, const Point2f& lastPos, time_t dT, bool unit = RAD) const;
    void trimRegion(Mat src, Rect &region) const;

    // to be added and tested:
    // predict prePos's position in dt ms
    bool predictIn(const Point2f& prePos, Point2f& postPos, double dt, int mode = CONSTMODE);

    // 与实现roi设置有关的函数
    RotatedRect armorDetect(Mat src) const;
    Rect centerRoi(Mat src, const Point2f &center);
};

// returned angle ranging from -PI to PI (aka from -180 deg to 180 deg)
float formatAngle(float angle, bool unit)
{
    if (unit == DEG)
    {
        if (fabs(angle) < 180)  return angle;
        else if (fabs(angle + 360) < 180) return angle + 360;
        else if (fabs(angle - 360) < 180) return angle - 360;
        return angle;
    }
    else if (unit == RAD)
    {
        if (fabs(angle) < PI)  return angle;
        else if (fabs(angle + 2 * PI) < PI) return angle + 2 * PI;
        else if (fabs(angle - 2 * PI) < PI) return angle - 2 * PI;
        return angle;
    }
    return angle;
}

// 对于wind.mp4 对中心标志R的识别效果很好
bool MillHiter::findMillCenter(Mat src, Point2f &center) const
{
    if (src.empty())
    {
        printf("src received by 'findMillCenter()' is empty\n");
        return false;
    }
    vector<Mat> splited;
    split(src, splited);
    Mat temp;

    /////////// time cost: 0.3 ms //////////
    if (this->_colorFlag == BLUE)
    {
        subtract(splited[0], splited[2], temp);
    }
    else
    {
        subtract(splited[2], splited[0], temp);
    }
    ////////////////////////////////////////

    threshold(temp, temp, 150, 255, THRESH_BINARY);
    dilate(temp, temp, Mat());
    dilate(temp, temp, Mat());
    vector<vector<Point>> contours;
    findContours(temp, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    for (int i = 0; i < contours.size(); i++)
    {
        double area = contourArea(contours[i]);
        Rect rectCon = boundingRect(contours[i]);
        double ratio = float(rectCon.height) / rectCon.width;
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
void MillHiter::trimRegion(Mat src, Rect &region) const
{
    region.x = max(0, region.x);
    region.y = max(0, region.y);
    region.width = min(region.width, src.cols - region.x);
    region.height = min(region.height, src.rows - region.y);
    return;
}

// 对于wind.mp4 绝大多数帧识别良好，除了用手遮挡的那几帧。同时需要注意，它不能区分已击打和未击打的装甲板
// 策略：Accuracy first!
RotatedRect MillHiter::armorDetect(Mat src) const
{
    if (src.empty())
    {
        printf("src received by 'armorDetect()' is empty\n");
        return RotatedRect();
    }
    vector<Mat> splited;
    split(src, splited);
    Mat temp;
    medianBlur(src, temp, 3);
    if (this->_colorFlag == BLUE)
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
    morphologyEx(temp, temp, MORPH_CLOSE, element, Point2f(0,0), 1);

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    Point2f center;
    // findContours only cost 1 ms in my machine (i5-9300H GTX1650)
    findContours(temp, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point2f(0, 0));
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
Rect MillHiter::centerRoi(Mat src, const Point2f &center)
{
    if (src.empty())
    {
        printf("src received by 'centerRoi()' is empty\n");
        return Rect(0, 0, src.cols, src.rows);
    }
    Point2f vertex[4];
    this->armorDetect(src).points(vertex);
    double armlength = 0.0;
    for (const Point2f &ver : vertex)
    {
        armlength = MAX(armlength, sqrt(pow(ver.x - center.x, 2) + pow(ver.y - center.y, 2)) + 20);
        // explanation: add extra number to ensure the calculating error will be offset
    }
    Rect Roi( int(center.x - armlength), int(center.y - armlength), int(2 * armlength), int(2 * armlength));
    trimRegion(src, Roi);
    return Roi;
}

// 策略：Efficiency first!
// mode=0:使用内嵌矩形的方式进行识别。注：预处理图片使用红蓝通道相减，因此ColorFlag的值很重要。（部分）特点：矩形区域中心点略有偏移
// mode=1:使用面积比，距离的方式进行识别。注：预处理图片使用亮度（装甲的灯条亮度显著高于周围环境），与ColorFlag的值无关。 （部分）特点：矩形区域中心点几乎无偏移
bool MillHiter::targetDetect(Mat roi, RotatedRect& target, int mode)
{
    if (roi.empty())
    {
        printf("roi received by 'targetDetect()' is empty\n");
        return false;
    }
    if (mode == 0)
    {
        Mat gray;
        Mat splited[3];
        split(roi, splited);

        if (this->_colorFlag == BLUE)  subtract(splited[0], splited[2], gray);
        else                           subtract(splited[2], splited[0], gray);
        // 通道相减法对 wind.mp4 效果极佳!

        Mat element = getStructuringElement(MORPH_RECT, Size(3,3));
        morphologyEx(gray, gray, MORPH_CLOSE, element, Point(0,0), 1); // 注意：MORPH_CLOSE迭代次数过多会导致矩形选区的偏移（将"1"改为"2"已经产生明显误差了）
        threshold(gray, gray, 80, 255, THRESH_BINARY);                 // TODO: 灰度的阈值可能要调
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
                int targetNum = hierarchy[i][2];   
                if (contourArea(contours[targetNum]) <= 25)
                    continue;
                // else: presume "target" is truely what we are looking for
                target = minAreaRect(contours[targetNum]);
                return true;
            }
        }
        return false;
    }
    else if (mode == 1)
    {
        Mat gray;
        cvtColor(roi, gray, COLOR_BGR2GRAY);           // around 1ms sometimes 1.5ms
        threshold(gray, gray, 80, 255, THRESH_BINARY); // no time   // TODO: 灰度的阈值可能要调
        // auto end = steady_clock::now();

        dilate(gray, gray, Mat()); // sometimes 1ms about these two operations
        dilate(gray, gray, Mat());

        floodFill(gray, Point2f(5, 50), Scalar(255), 0, FLOODFILL_FIXED_RANGE); // around 1ms

        threshold(gray, gray, 80, 255, THRESH_BINARY_INV);
        vector<vector<Point>> contours;
        findContours(gray, contours, RETR_LIST, CHAIN_APPROX_NONE); // sometimes 1ms

        // the "for" loop overall costs 1 ms
        for (size_t i = 0; i < contours.size(); i++)
        {
            vector<Point> points;
            double area = contourArea(contours[i]);
            if (area < 50 || 1e4 < area)
                continue;
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
                    if (area < 50 || 1e4 < area)            // TODO：长宽比可能也需要调
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
                if (middle > 60)   // middle是该轮廓包围矩形的中心与其他轮廓包围矩形的中心之间的最小距离，用于区分待击打矩形和已击打矩形  // TODO：middle可能也要调
                {
                    target = rrect;
                    return true;
                }
            }
        }
        return false;
    }

    // else: set mode wrong 
    else
    {
        printf("no such mode");
        return false;
    }
}

// 该ROI设置算法对相机参考系与大符参考系的相对静止有严重依赖！！！
// 对于wind.mp4，ROI设置好后，有一定的优化效果，设置前则每帧耗时5-6ms
bool MillHiter::targetLock(Mat src, Point2f& aim, int mode, int SampleSize, double DistanceErr, double nearbyPercentage)
{
    // preprocess:
    if (src.empty())
    {
        printf("src received by 'targetLock()' is empty\n");
        return false;
    }

    bool noR = !this->_centerRAvail;
    bool noRoi = !this->_roiAvail;
    if (!noRoi) rectangle(src, _roi, Scalar(0,255,255),2);
    
    else if (noRoi && noR)      // 最初始的状态
    {
        Point2f centerNow;
        if (this->findMillCenter(src, centerNow))  // 此时findMillCenter 找到了这帧的 R，把该组数据压入 _sampleR中作为有效数据
        {
            this->_sampleR.push_back(centerNow);
            if (_sampleR.size() < SampleSize)   
            {
                this->_roi = Rect(0, 0, src.cols, src.rows);
            }
            else   // 如果_sample.size() 已经达到SampleSize（取了一个不大不小的值 10 作为样本容量的默认值），就开始拟合中心。策略：把偏差过大的点舍去，取“聚点”
            {
                for (int i = 0; i < SampleSize; i++)
                {
                    int neighborNum = 1;        // 该点和它本身距离为0，足够小，故临近点个数初始化为1，子循环遍历从第i+1个开始    
                    for (int j = i + 1; j < SampleSize; j++)
                        if (DISTANCE(this->_sampleR[i], this->_sampleR[j]) < DistanceErr)         // 注意：这里的误差是需要调节的，此处草率地预设为 7.0
                            neighborNum++;
                    if (double(neighborNum) / SampleSize >= nearbyPercentage)       // 有超过nearbyPercentage的点聚集在该点的周围，可以认为该点就是“聚点”
                    {
                        this->_centerRAvail = true;
                        this->_centerR = this->_sampleR[i];
                        noR = false;
                        break;
                    }
                    if (i == SampleSize)    // 遍历至此，还不退出，这说明没有点是聚点，所以清空所有样本数据，重测
                    {
                        this->_sampleR.clear();
                        this->_roi = Rect(0, 0, src.cols, src.rows);
                    }
                }
            }
        }
        else    // 说明 findMillCenter 失败了，此时不改变 _sampleR，将roi设置为全图（即在全图中寻找） 
        {
            this->_roi = Rect(0, 0, src.cols, src.rows);
        }
    }

    if (noRoi && !noR)  // 不加else是因为可能在此帧拟合出来了R点，如果加了，在这帧，这个分支就会被跳过
    {
        this->_roi = centerRoi(src, this->_centerR);              // centerRoi一般会成功，但也有可能失败
        if (this->_roi.width == src.cols && this->_roi.height == src.rows)   // 如果出现_roi区域和全图的大小一样，说明这次寻找失败了，设置_roiAvail为false，下次再找
        {
            this->_roiAvail = false;
        }
        else
        {
            this->_roiAvail = true;
        }
    }

    Mat roi = Mat(src, this->_roi);
    RotatedRect target;
    if (!this->targetDetect(roi, target, mode))
        return false;
    aim.x = target.center.x + this->_roi.x;
    aim.y = target.center.y + this->_roi.y;
    return true;
}

// return degree angle (ranging from -90 to +270)
inline double MillHiter::getAngle(const Point2f& center, const Point2f& pos, bool unit) const
{
    if (unit == RAD) return atan2(pos.y - center.y, pos.x - center.x);
    else             return RAD2DEG(atan2(pos.y - center.y, pos.x - center.x));
} 

// calculate mean angular speed. when "dT" is marginal, it can be used as instant angular velocity
// if returned value is plus, the mill rotates counter-clockwise. if minus, it rotates clockwise.
double MillHiter::getAngularSpeed(const Point2f& center, const Point2f& nowPos, const Point2f& lastPos, time_t dT, bool unit) const
{
    double AngularSpeed;
    double dAngle = this->getAngle(center, nowPos, DEG) - this->getAngle(center, lastPos, DEG);
    // Presumption with reason: because dT is small, dAngle must be small(smaller than 180 degree)
    if (fabs(dAngle) < 180)
        AngularSpeed = dAngle / dT;
    else
    {
        if (fabs(dAngle + 360) < 180)
            AngularSpeed = (dAngle + 360) / dT;
        else AngularSpeed = (dAngle - 360) / dT;
    } 
    if (unit == DEG) return AngularSpeed;
    else             return DEG2RAD(AngularSpeed);
}
 
bool MillHiter::predictIn(const Point2f& prePos, Point2f& postPos, double dt, int mode)
{
    /*  dt: ms  CONSTSPEED: deg/s  */
    if (mode == CONSTMODE)
    {
        /* put prediction code of constant-speed-motion here */
        if (!this->_centerRAvail) { /* printf("center R is needed to predict\n"); */  return false; }
        else 
        {
            double dAngle = CONSTSPEED * (dt / 1000);  
            dAngle = DEG2RAD(dAngle);
            postPos.x = this->_centerR.x + (prePos.x - this->_centerR.x) * cos(dAngle) - (prePos.y - this->_centerR.y) * sin(dAngle);
            postPos.y = this->_centerR.y + (prePos.x - this->_centerR.x) * sin(dAngle) + (prePos.y - this->_centerR.y) * cos(dAngle);
            return true;
        }
    }
    else if(mode == SINMODE)
    {
        /* put prediction code of sine-motion here  */
        return false;
    }   
    else 
    {
        cout << "wrong mode\n";
        return false;
    }
}

int main()
{
    VideoCapture cap("C:/Users/Lenovo/Desktop/VScode/Windmills-TEST/wind.mp4");
    Mat frame;
    Point2f vertices[4];
    MillHiter hit(BLUE);
    float lastAngle;
 
    for (int i = 0;; i++)
    {
        cap >> frame;
        if (frame.empty())
            break;
        // 计时开始，读图在另一个线程，只算处理图片的时间
        auto start = steady_clock::now();

        resize(frame, frame, Size(640, 480));  

        Point2f prePos; 
        Point2f postPos;
        if (!hit.targetLock(frame, prePos, 1, 10))
            continue;

        circle(frame, prePos, 3, Scalar(0, 0, 255), -1);

        if(hit.predictIn(prePos, postPos, 200, CONSTMODE))
            circle(frame, postPos, 2, Scalar(0, 255, 0), -1);

        // 计时结束
        auto end = steady_clock::now();
        auto duration = end - start;
        printf("time cost: %lf ms\n", (double)duration.count() / 1e6);

        imshow("marked frame", frame);
        if (waitKey(10) == 'q')
            break;

    }
    system("pause");
}