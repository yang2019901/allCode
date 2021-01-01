/*
 * 这是 TJU Robomasters 上位机源码，未经管理层允许严禁传播给其他人（包括队内以及队外）
 *
 *     Program enter point. The program is splited to two threads: one reading camera and prepare data for algorithm
 * the other thread deal with the latest frame. The purpose is that, if the operations are serialized excuted, CPU
 * resources are not completely used, as GPU acceleration will make most of calculations on GPU and CPU spend much
 * time waiting for GPU to finish. So using that waiting time, the other thread loading the next image.
 * Feature modifications:
 *    2018.6.8  Finished the framework of main function, added multi-thread
 *    2018.6.12 Updated image_data structure
 *    2018.6.13 重新增加了图像获取线程
 */


#include "custom_definations.hpp"
#include <iostream>
#include <opencv2/opencv.hpp>
#include "armorDetector.hpp"
#include "serial.hpp"
#include <thread>
#include "camview.hpp"         
#include <unistd.h>
#include "armorTracker.hpp"
#include "armorHiter.hpp"
#include "dfcHiter.hpp"//dfc
#include "dnnManager.hpp"
#include "autolead.hpp"
#include "autoTracker.hpp"
using namespace cv;
using namespace std;


void ProcessFullFunction(ImageData &frame);
void ProcessAlgorithmFunction(ImageData &frame);
void ArmorDetectDebug(ImageData &frame);
void DSFDetectDebug(ImageData &frame);
void AutoLeadDebug(ImageData &frame);
void AutoBlockDebug(ImageData &frame);
double lastt = getTickCount(), curt = lastt,checkpoint_time = lastt,dtTime = 0;
int fps  = 0,frames = 0;

void capture_init(VideoCapture &capture)
{
    //设置帧缓存
    capture.set(CV_CAP_PROP_BUFFERSIZE,3);
    //设置分辨率
    switch (ConfigurationVariables::GetInt("resolutionType",0))
    {
    case 0:
        capture.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
        capture.set(CV_CAP_PROP_FRAME_WIDTH, 640);
        break;
    case 1 :
        capture.set(CV_CAP_PROP_FRAME_HEIGHT, 720);
        capture.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
    case 2:
        capture.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
        capture.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
    default:
        break;
    }  
    capture.set(CV_CAP_PROP_FOURCC, CV_FOURCC('M', 'J', 'P', 'G') );    //设置图片压缩格式
    capture.set(CV_CAP_PROP_EXPOSURE, ConfigurationVariables::GetInt("exposure",30)*0.0002);
    //设置曝光度之前需要在qv4l2手动添加手动曝光 0-1 映射到qv4l2的1-5000
}

// returns delta time
void UpdateFPS()
{
    frames++;
    curt = (double)getTickCount();
    //getTickFrequency 返回时钟频率
    //如果当前时钟减去上次输出时钟计数器为运行频率，则统计当前frames
    if (curt - checkpoint_time >= getTickFrequency()) 
    {
        checkpoint_time = curt;
        fps = frames;
        frames = 0;
        cout <<"fps:"<< fps << endl;
    }
    dtTime = (curt - lastt) / getTickFrequency();
    lastt = curt;  
}

void DisplayFPS(Mat &m)
{
    char fpsstr[30];
    sprintf(fpsstr,"fps:%d",fps);
    putText(m,fpsstr,Point(20,20),FONT_HERSHEY_SIMPLEX,1.0,Scalar(0,0,0),2);

}

//----global class objects

// camera thread variables

#if defined DEVICE_TX
    VideoCapture rmcap("/dev/video0");
#elif defined DEVICE_MANIFOLD
    VideoCapture rmcap("/dev/video0");
#endif

bool threadContinueFlag = true;
char waitedKey = 0;
bool onpause = false;
Lock<ImageData> frameData;//the framedata includes the data of every image
// modules definations
SerialManager *serial_ptr = NULL;
CameraView *camview_ptr = NULL;
ArmorBaseDetector *armor_detector_ptr = NULL;
ArmorTrackerBase *armor_tracker_ptr = NULL;
ArmorHiter *armor_hiter_ptr = NULL;
DfcHiter *dfc_hiter_ptr = NULL;
DnnManager *dnn_manager_ptr = NULL;
AutoLeader *autoleader_ptr = NULL;
AutoBlockTracker *autoBlock_ptr = NULL;


// 图像收集线程
void ImageCollectThread()
{
    SerialPort serialPort(ConfigurationVariables::GetInt("Port",1));
    SerialManager serialManager(&serialPort);
    serial_ptr = &serialManager;

    int frameIndex = 0;
    Mat img;
    while(threadContinueFlag)
    {
        try{
            // Get Image
            rmcap >> img;
            if (ConfigurationVariables::resolutionType == 0)
                resize(img,img,Size(960,720));
            // 更新当前帧的电控参数
            serialManager.UpdateReadData();
            frameData.Lock();   //frameData 加锁，防止其他线程在更改变量时调用 将串口下位机发送的指令赋值给framedata
            //{
                frameData.variable.ptzSpeed = ElectronicControlParams::PTZSpeed;
                frameData.variable.ptzAngle = ElectronicControlParams::PTZAngle;
                frameData.variable.worldPosition = ElectronicControlParams::worldPosition;
                frameData.variable.shootSpeed = ElectronicControlParams::shotSpeed;
                // 告知处理线程，图像准备完成
                frameData.variable.image = img;
                frameData.variable.index = frameIndex++;
                // DEBUG_DISPLAY(frameData.variable.image);
            //}
            frameData.Unlock(); //数据更改完成，解锁

        }
        catch(...)
        {
            cout << "Error in Collect." << endl;
        }
    }
}
//图像处理线程
void ImageProcessThread()
{
    // 初始化各模块
    char camparams[100] ;
    // //format output to camparams
    //注意如果更改了源码保存位置一定修改FILEDIR路径
    sprintf(camparams,FILEDIR(camparams_%d.xml),ConfigurationVariables::resolutionType);
    CameraView camview(camparams);// load the parameters of the camera
    camview_ptr = &camview;

    PolyMatchArmorDetector armor_detector;
    armor_detector_ptr = (ArmorBaseDetector*)(&armor_detector);
    armor_detector_ptr->SetTargetArmor(2 - ElectronicControlParams::teamInfo);

    LinearPredictor linear_predictor; 

    DnnManager dnnManager(FILEDIR(model/mnist.cfg), FILEDIR(model/mnist.weights), FILEDIR(model/fire.cfg),
                          FILEDIR(model/fire.weights), FILEDIR(model/armor.cfg), FILEDIR(model/armor.weights)); // file names
    dnn_manager_ptr = &dnnManager;

    PredictPIDArmorTracker predict_pid_tracker(camview_ptr,armor_detector_ptr,dnn_manager_ptr,&linear_predictor);
    armor_tracker_ptr = &predict_pid_tracker;

    // 等待串口类被初始化
    while(!serial_ptr);
    
#if defined ROBOT_SENTINEL
    SentinelArmorHiter sentinelHiter(serial_ptr,armor_tracker_ptr);
    armor_hiter_ptr = &sentinelHiter;
#elif defined ROBOT_INFANCY
    InfancyArmorHiter infancyHiter(serial_ptr,armor_tracker_ptr);
    armor_hiter_ptr = &infancyHiter;
#endif
    // 神符打击和自动引导模块的定义..定制需求添加在这里

    DfcHiter dfcHiter( serial_ptr, camview_ptr);
    dfc_hiter_ptr = &dfcHiter;
     
    AutoLeader autoleader(serial_ptr,camview_ptr);
    autoleader_ptr = &autoleader;

    AutoBlockTracker autoBlock(serial_ptr);
    autoBlock_ptr = &autoBlock;

    //初始化serialManager中的模块列表
    //压入3个模块 装甲识别 0； 大风车 1； 自动 2；
    serial_ptr->RegisterModule(armor_hiter_ptr);
    serial_ptr->RegisterModule(dfc_hiter_ptr);
    serial_ptr->RegisterModule(autoleader_ptr);

    int lastIndex = 0;
    ImageData processFrame;
    while(threadContinueFlag)
    {
        if (waitedKey == 'p')
        {
            waitedKey = 0;
            onpause = !onpause;
        }
        if (onpause) continue;
        try
        {
            // 当前图像获取线程还不能提供最新图像
            if (lastIndex >= frameData.variable.index) continue;
//LOG(A)
            // 拷贝获取线程得到的图像，防止内存错误
            frameData.Lock();
                frameData.variable.copyTo(processFrame);
            frameData.Unlock();
//LOG(B)
            lastIndex = processFrame.index;
            if (processFrame.image.empty()) break;
            UpdateFPS();
            // 处理该帧数据
            if (ConfigurationVariables::MainEntry < 4)
                ProcessFullFunction(processFrame);
            else 
                ProcessAlgorithmFunction(processFrame);

//LOG(C)
            DisplayFPS(processFrame.image);
            if(DEBUG_MODE)
                DEBUG_DISPLAY(processFrame.image);
            waitedKey = 0;

        }
        catch(...)
        {
            cout << "Error in process." << endl;
        }
        // 串口发送当前帧数据
        serial_ptr->FlushData();
//LOG(D)
        // 每秒更新一次configuration
        if(frames == 0 && ConfigurationVariables::KeepUpdateConfiguration)
        {
            // reloading configuration variables
            ConfigurationVariables::ReadConfiguration(false);
            if (!ConfigurationVariables::Loaded) cout << "Error when update configuration variables." << endl;
        }
    }
}

void ImageDisplayThread()
{
    while(threadContinueFlag)
    {
        if (ConfigurationVariables::DebugMode)
        {
            try{
                if (!onpause || DebugDisplayManager::imgs.size())
                    DebugDisplayManager::DisplayAll();
                int c = waitKey(1);
                if (c != -1) waitedKey = (char)c;
                if (waitedKey == 'x') threadContinueFlag = false;
            }catch(...){
                cout << "Error in Display." << endl;
            }
        }
	else
	    sleep(1);
    }
}

int main() {
//     // init configuration
    ConfigurationVariables::ReadConfiguration(true);//读取Configuration
    if (!ConfigurationVariables::Loaded)
        cout << "Load Configuration Failed. Using default values." << endl;
    else
        cout << "Configuration Loaded Successfully." << endl;
    ElectronicControlParams::teamInfo = ConfigurationVariables::GetInt("StartArmorType",2);

//     // init camera capture
    capture_init(rmcap);

//     // calibration process differs
    if (ConfigurationVariables::MainEntry == 10)//相机标定程序
    {
        char camparams[100] ;
        sprintf(camparams,FILEDIR(camparams_%d.xml),ConfigurationVariables::resolutionType);
        CameraView::CalibrateCameraProcess(camparams,rmcap); // write the parameters of the camera into the camparas.xml
        return 0;
    }
//     // init modules
//     // init threads
//creat the multi-thread mode run the imagecollect and process and display
    thread proc_thread(ImageProcessThread);
    thread display_thread(ImageDisplayThread);
    thread collect_thread(ImageCollectThread);

    collect_thread.join();
    proc_thread.join();
    display_thread.join();

//     return 0;
}



void ProcessFullFunction(ImageData &frame)
{
    // 调试模块的模式下 强制打开模块
    switch(ConfigurationVariables::MainEntry)
    {
    case 1: serial_ptr->EnableModule(2);break;
    case 2: serial_ptr->EnableModule(1);break;
    case 3: serial_ptr->EnableModule(4);break;
    }
    serial_ptr->UpdateCurModule(frame,dtTime);
}

void ProcessAlgorithmFunction(ImageData &frame)
{
    switch(ConfigurationVariables::MainEntry){
        case 4:  // armor detect
            ArmorDetectDebug(frame);
            break;
        case 5:
            DSFDetectDebug(frame);
            break;
        case 6:
            AutoLeadDebug(frame);
            break;
        case 7:
            AutoBlockDebug(frame);
        break;
    }
}

void ArmorDetectDebug(ImageData &frame)
{
    // test detector:
    //armor_detector_ptr->DetectArmors(frame.image);
    // test tracker
    Point2f res = armor_tracker_ptr->UpdateFrame(frame,dtTime) * 0.3 + frame.ptzAngle;
    serial_ptr->SendPTZAbsoluteAngle(res.x,res.y);
    // test serial
    //serial_ptr->SendSingleOrder(222);
//    cout << "PTZ Angle " << frame.ptzAngle << endl;
//    static float x = 0;
//    if (waitedKey == 'a') x-=0.5f;
//    if (waitedKey == 'd') x += 0.5f;
//    serial_ptr->SendPTZAbsoluteAngle(x,0);
    /*
    static int exposureTime = ConfigurationVariables::GetInt("exposureTime",64);
    if (waitedKey == 'd')
    {
        exposureTime += 4;
        cout << exposureTime << endl;
        SetCameraExposure((false,exposureTime));
    }
    else if (waitedKey == 'a')
    {
        exposureTime -= 4;
        cout << exposureTime << endl;
        SetCameraExposure((false,exposureTime));
    }*/
}

void DSFDetectDebug(ImageData &frame)
{

    static int flag = 0;
    static int angle_yaw = 0;
    static int angle_p = 0;
/*
    if(flag == 1)
    {
        dsf_reco_ptr->GetDsfPosition(frame);
        angle_yaw = frame.ptzAngle.x;
        angle_p = frame.ptzAngle.y;
    }
    if(flag == 2)
        dsf_reco_ptr->UpdateFrame(frame);
*/
//cout<<frame.shootSpeed<<endl;

//    flag=2;
    if(waitedKey == 'r') 
        if(flag != 1) flag = 1;
        else flag = 2;

/*
    if(waitedKey == 'a') 
    {
        angle_yaw += 1; 
        serial_ptr->SendPTZAbsoluteAngle(angle_yaw, angle_p);
    }
    if(waitedKey == 'b') 
    {
        angle_yaw -= 1; 
        serial_ptr->SendPTZAbsoluteAngle(angle_yaw, angle_p);
    }
    if(waitedKey == 'c') 
    {
        angle_p += 1; 
        serial_ptr->SendPTZAbsoluteAngle(angle_yaw, angle_p);
    }
    if(waitedKey == 'd') 
    {
       angle_p -= 1; 
       serial_ptr->SendPTZAbsoluteAngle(angle_yaw, angle_p);
    } 
*/
}

void AutoLeadDebug(ImageData &frame)
{

}

void AutoBlockDebug(ImageData &frame)
{
    static bool started = false;
    if (waitedKey == 's' && !started)
    {
        cout << "started " << endl;
        autoBlock_ptr->EnableModule();
        started = true;
    }
    else if (started && waitedKey == 'e')
    {
        autoBlock_ptr->DisableModule();
        started = false;
    }

    if (started)
    {
        autoBlock_ptr->Update(frame,dtTime);
    }
}
