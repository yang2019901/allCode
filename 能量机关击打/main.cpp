#include <iostream>
#include <cmath>
#include <assert.h>

using namespace std;

#include "SinePredictor.hpp"
using namespace Sine;
#include "random.hpp"

const Sys_time frame_gap = 20 * 1000;            // 帧之间的间隔 20ms
double angle_0 = M_PI_2;            // 大风车角度初始值


/* 梯度下降的相关信息 */
/* 1. 可以确定梯度下降具有较大的修正效果，修正效果很大程度上取决于后面theta观测值的准确度（误差0.1rad仍具有较大的修正效果） */
/* 2. 只利用alpha同时修正init_theta与t_hat的误差是不够的，会顾此失彼。同时修正两个参数，可以保证效果比不修正时好 */
/* 3. 利用卡尔曼滤波（线性）配合梯度下降，每一帧都对方程进行一次修正 */


/* 获取均值0，方差var的正态分布噪声 */
const double get_noise(const double var)
{
    return random_normal(0, var);
}


// 模拟
void simulate()
{
    SinePredictor predictor;
    predictor.init_filter_variance(0.1, 0.01);
    double total_error = 0;             // 总共的误差绝对值的和
    int cnt = 0;                        // 总共的修正次数

    int frame_count = 0;

    Sys_time sys_time = 100;
    

    for (; sys_time < 100 + 500 * frame_gap; sys_time += frame_gap)
    {
        frame_count++;
        double delta_t = get_delta_t(sys_time, 100);
        /* 生成带误差的角度 */
        double angle = t2theta(angle_0, delta_t) + get_noise(0.1);
        // cout << "this angle: " << angle << endl;
        /* 前30帧用于训练 */
        if (frame_count <= 30)
            predictor.feed_angle(angle, sys_time);
        /* 如果100帧还是无法初始化角度，就报bug */
        else if (!predictor.is_ok())
        {
            cout << "something was wrong..." << endl;
            break;
        }
        else
        {
            if (frame_count == 200)
            {
                cout << "change target" << endl;
                angle_0 -= 0.5;
                predictor.change_target(sys_time, t2theta(angle_0, (sys_time - 100) / 1e+6));
                continue;
            }
            const double angle_pred = predictor.predict(sys_time, frame_gap / 1e+6);
            /* 记录误差 */
            angle = t2theta(angle_0, delta_t + frame_gap / 1e+6);
            cout << "predict: " << angle_pred << endl;
            cout << "real : " << angle << endl;
            cout << "this time error: " << abs(angle_pred - angle) << endl << endl;
            total_error += abs(angle_pred - angle);
            cnt++;
            // getchar();
        }
    }

    cout << "prediction time: " << cnt << endl;
    cout << "average error: " << total_error / cnt << " rad" << endl;
    cout << predictor.get_alpha() << endl; /* 可以看到是一个比较小的数 */
    cout << predictor.get_beta() << endl;
}

int main()
{
    simulate();
    return 0;
}