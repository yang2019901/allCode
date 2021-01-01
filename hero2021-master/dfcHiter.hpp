/*
 * 这是 TJU Robomasters 上位机源码，未经管理层允许严禁传播给其他人（包括队内以及队外）
 *
 * dfcHiter类控制机器人进行打击操作
 */


#pragma once
/*
#ifndef _DFC_HPP_
#define _DFC_HPP_
#endif*/
#include "dfc.hpp"
#include "camview.hpp"
#include "serial.hpp"
#include "util.hpp"
#include "configurations.hpp"

using namespace cv;
using namespace std;

// 模块ID为1
class DfcHiter : public  ModuleBase{

public:
    double ShootAngleThresh = 1;
    Point2f shootOffAngle;

    DfcHiter(SerialManager *_serial ,CameraView *_camview ):ModuleBase(1)
    {
        serial = _serial;
        camview = _camview;
        SET_CONFIG_DOUBLE_VARIABLE(ShootAngleThresh,1)
    }

    void EnableModule()
    {
        isPosition = true;// 标定标志
        waitFire = false;// 等待打击完成标志
        stage = 0;// 打击阶段
        frameNumber = 0;// 等待云台转动到位的时间以及打击完成时间
    }

    // 被强制关闭，释放一些状态
    void DisableModule()
    {
        EnableModule();
    }

    void Update(ImageData &frame,float dtTime)
    {//处理大风车打击程序；
        Point2f target(0,0), center(0,0);
        bool flag = false;
        Mat src = frame.image;
        bigWindMill( src, target, center );//获得大风车的待打击位置坐标和中心R坐标
        if ( target.x != 0 && target.y != 0 && center.x != 0 && center.y != 0 ){
            flag = true;
        }
        if ( flag ){
            Point2f preTarget;//预测击打点的坐标
            double speed, times, rad;//定义风车的转速、延迟的时间、风车的半径
            preTarget = getPredicentPosition( target, center, speed, times, rad );
            double distance;
            Point2f ptzOffAngle = CalcDfcPTZAngle( center, distance);
            shootOffAngle = ptzOffAngle + CalculateGravityAngle( frame.ptzAngle.y+ptzOffAngle.y,frame.shootSpeed,distance );
        }
        

        
        if ( flag ) // found target
        {
            // 发送消息
            serial->SendFiringOrder(Length(shootOffAngle) < ShootAngleThresh ,true);
            serial->SendPTZAbsoluteAngle(shootOffAngle.x + frame.ptzAngle.x, shootOffAngle.y + frame.ptzAngle.y);
        }
    }



private:
    SerialManager *serial;
    CameraView *camview;
    int stage;
    bool isPosition;
    bool waitFire;
    int frameNumber;


    Point2f CalcDfcPTZAngle(Point2f center,float probDis)
    {
        probDis = 800;//假设从发射机构到大风车的距离有8m
        return camview->ScreenPointToPTZAngle(center,probDis,1);
    }
    
};


