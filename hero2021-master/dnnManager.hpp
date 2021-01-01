/*
 * 这是 TJU Robomasters 上位机源码，未经管理层允许严禁传播给其他人（包括队内以及队外）
 *
 * DnnManager 对深度学习模型进行管理，对接 Darknet
 * 在这里编辑代码改动、功能改进说明：
 * convert pb model trained by tensorflow to weights model, use darknet read model
 * the model need be optimized
 */

#pragma once

// 是否启用darknet，启用深度神经网络相关功能;如开启CMakeList中应该添加对darknet库的支持
#define ENABLE_DARKNET

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include "util.hpp"
#include "KMSolver.hpp"

#ifdef ENABLE_DARKNET

#include "darknet.h"

#endif
using namespace std;
using namespace cv;

/*
 * 深度学习对装甲兵种分类
 * 输入图像为中间图样的二值化图像，输入大小28*28
 * 输出为下面几种类别之一
 */
typedef int ArmorDetailType;
#define ARMOR_TYPE_UNKNOWN 0   // 不能确定装甲类型
#define ARMOR_INFAN 1     // 装甲为步兵
#define ARMOR_HERO 2      // 装甲为英雄
#define ARMOR_ENGIN 3     // 装甲为工程

class DnnManager {
public:
    /*
     * 构造函数，参数为三个模型的路径
     * mnist_cfg 小能量机关模型框架路径
     * mnist_model 小能量机关模型的路径
     * fire_cfg 大能量机关模型框架路径
     * fire_model 大能量机关模型的路径
     * armor_cfg 装甲类型识别的模型框架路径
     * armor_model 装甲类型识别的模型路径
     * 路径最好通过头文件util.hpp中的FILEDIR(fileName)得到
     */
    DnnManager(char* mnist_cfg, char* mnist_weights, char* fire_cfg, char* fire_weights, char* armor_cfg, char* armor_weights)
    {
#ifdef ENABLE_DARKNET
        mnist_net = load_network(mnist_cfg, mnist_weights, 0);
        fire_net = load_network(fire_cfg, fire_weights, 0);
        armor_net = load_network(armor_cfg, armor_weights, 0);
#endif
    }
    // 单个格子分类
    void ClassifyDsfCell(Mat image,int &result,float &confidence, bool isSmallEnergy = true)
    {
        int r[1];float c[1];
        ClassifyDsfCells(vector<Mat>({ image }), r, c, isSmallEnergy);
        result = r[0];
        confidence = c[0];
    }

    // 九宫格分类
    void ClassifyDsfCells(vector<Mat> images, int result[], float confidence[], bool isSmallEnergy = true)
    {
        target = isSmallEnergy ? 0 : 1;
        RunSmallNet(images, result, confidence);
    }

    // 单个装甲分类
    void ClassifyArmor(Mat image,ArmorDetailType &result,float &confidence)
    {
        int r[1];float c[1];
        ClassifyArmors(vector<Mat>({ image }), r, c);
        result = r[0];
        confidence = c[0];
    }

    // 多装甲同时分类
    void ClassifyArmors(vector<Mat> images, ArmorDetailType result[], float confidence[])
    {
        target = 2;
        RunSmallNet(images, result, confidence);
    }

private:
#ifdef ENABLE_DARKNET
    network *mnist_net;
    network *fire_net;
    network *armor_net;
#endif

    KMSolver KM;
    int target;// target 0--mnist 1--fire 2--armor

    void RunSmallNet( vector<Mat> &images, int result[], float confidence[])
    {
#ifdef ENABLE_DARKNET
        data d={0};// struct defined in darknet use to process batch
        matrix X = make_matrix(images.size(),28*28*1);//struct defined in darknet use to save image datas
        image img;// struct defined in darknet use to save image data
        // put images in inputs
        for(int n=0;n<images.size();n++)
        {
            img = make_image(28, 28, 1);
            for(int i=0; i<28; i++)
            {
                uchar* pixel = images[n].ptr<uchar>(i);
                for(int j=0; j<28; j++)
                {

                    img.data[i*28+j] = pixel[j]/255.;
//                    img.data[28*28 + i*28+j] = pixel[j][1]/255.;
//                    img.data[2*28*28 + i*28+j] = pixel[j][0]/255.;
                }
            }
            X.vals[n] = img.data;
        }
        d.X = X;
        // predict result
        matrix p;
        int class_num = (target == 0 || target == 1)?10:6;
        Mat pred = Mat(images.size(),class_num,CV_32FC1);
        if(target == 0)
            p = network_predict_data(mnist_net, d); 
        if(target == 1)
            p = network_predict_data(fire_net, d); 
        if(target == 2)
            p = network_predict_data(armor_net, d); 
        for(int i=0; i<pred.rows; i++)
        {
            float* data = pred.ptr<float>(i);
            for(int j=0; j<pred.cols; j++)
            {
                data[j] = p.vals[i][j];
                //cout<<data[j]<<endl;
            }

        }
        free_matrix(p);
        free_data(d);
        if(target == 0 || target == 1)
            DealPredAsWhole(pred, result, confidence, 1);
        else
            DealPredForEach(pred, result, confidence);
#else
        
        for(int n=0;n<images.size();n++)
        {
            result[n] = 0;
            confidence[n] = 0.2;
        }
#endif
    }
    void DealPredForEach(Mat &pred, int result[], float confidence[])
    {
        FOREACH(i, pred.rows)
        {
            float curconf = 0, t;
            FOREACH(j, pred.cols)
            {
                t = pred.at<float>(i, j);
                if (t > curconf)
                {
                    curconf = t;
                    result[i] = j;
                }
            }
            confidence[i] = curconf;
        }
    }

    void DealPredAsWhole(Mat &pred, int result[], float confidence[],int startCol = 0)
    {
        Rect rc(1,0,9,9);
        pred = pred(rc);
        KM.BestMatch(pred);
        FOREACH(i, pred.rows)
        {
            result[i] = KM.match[i]+startCol;
            confidence[i] = pred.at<float>(i,result[i]);
        }
    }
};
