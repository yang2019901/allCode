
#include "serial.hpp"
#include "trackerMedianFlow.hpp"
#include "util.hpp"
#include "configurations.hpp"
#include "opencv2/opencv.hpp"
using namespace cv;

class AutoBlockTracker : public ModuleBase
{
public:
    PID follow_pid;

    AutoBlockTracker(SerialManager *_serial):ModuleBase(8)
    {
        this->serial = _serial;

        SET_CONFIG_DOUBLE_VARIABLE(follow_pid.kp,1);
        SET_CONFIG_DOUBLE_VARIABLE(follow_pid.ki,0);
        SET_CONFIG_DOUBLE_VARIABLE(follow_pid.kd,0);
        follow_pid.SetLimitParams(1,0.4);
    }

    void EnableModule()
    {
        tracker = TrackerMedianFlow::create();
        helloWorld = true;
    }

    void DisableModule()
    {

    }

    void Update(ImageData &frame,float dtTime)
    {
        if (helloWorld)
        {
            helloWorld = false;
            boundRect.x = frame.image.cols / 2 - 40;
            boundRect.y = frame.image.rows / 2 - 40;
            boundRect.width = 80;
            boundRect.height = 80;
            //cout << "Started " << boundRect << endl;
            tracker->init(frame.image,boundRect);
        }
        else
        {
            //cout << boundRect << endl;
            if (tracker->update(frame.image,boundRect))
            {
                Point2d center(boundRect.x + boundRect.width / 2,boundRect.y + boundRect.height / 2);
                double rate = center.x / frame.image.cols * 2 -1;
                rate = follow_pid.calc(rate);
                serial->SendMotionControl(rate,0,0);
            }
        }
        if (DEBUG_MODE)
        {
            rectangle(frame.image,boundRect,Scalar(255,0,255),2);
        }
    }

protected:
    SerialManager *serial;
    bool helloWorld = false;
    Ptr<TrackerMedianFlow> tracker;

    Rect2d boundRect;
};
