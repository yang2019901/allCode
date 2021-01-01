/*
 * 这是 TJU Robomasters 上位机源码，未经管理层允许严禁传播给其他人（包括队内以及队外）
 *
 * 自动引导模块，在补给站附近可以识别视觉特征，并自动引导至补给站
 * 在这里编辑代码改动、功能改进说明：
 *
 */

#pragma once

#include "util.hpp"
#include "serial.hpp"
#include "configurations.hpp"
#include "camview.hpp"
#include "dnnManager.hpp"

class AutoLeader : public ModuleBase
{
public:
    AutoLeader(SerialManager *_serial,CameraView *_camview) : ModuleBase(4)
    {
        serialManager = _serial;
        cameraView = _camview;
    }

    // 刚刚被启用，Update前初始化控制流程
    void EnableModule()
    {

    }

    // 被强制关闭，释放一些状态
    void DisableModule()
    {

    }

    // 更新控制指令，dtTime为两次更新之间的间隔时间
    void  Update(ImageData &frame, float dtTime)
    {

    }

private:
    SerialManager *serialManager;
    CameraView *cameraView;
};
