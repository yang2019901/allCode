#pragma once

#include <iostream>
#include "opencv2/opencv.hpp"
#include <math.h>


using namespace std;
using namespace cv;
//获取点间距离
double getDistance(Point A,Point B)
{
    double dis;
    dis=pow((A.x-B.x),2)+pow((A.y-B.y),2);
    return sqrt(dis);
}
bool isTargetFLow( vector<Point> contourss){//检测是否是目标装甲
	        int area = contourArea(contourss);
			int perimeter = arcLength(contourss,1);
			if ( area < 2000 || area > 6000 ) return false;
			if ( area/perimeter >15 ||  area/perimeter < 5) return false;
			return true;
}
bool isCenterR( vector<Point> contourss ){//检测是否是中心R
	        int area = contourArea(contourss);
			int perimeter = arcLength(contourss,1);
			if ( area >800 || area < 160  ) return false;
			if ( area/perimeter >7 || area/perimeter < 2) return false;
			return true;
}
Point2f getzuobiao( vector<Point> contourss ,Mat img){//根据轮廓得到坐标
	    RotatedRect rectPoint = minAreaRect( contourss );
		//定义一个存储以上四个点的坐标的变量
		Point2f fourPoint2f[4];
		//将rectPoint变量中存储的坐标值放到 fourPoint的数组中
		rectPoint.points(fourPoint2f);
 
		//根据得到的四个点的坐标  绘制矩形
		for (int i = 0; i < 3; i++)
		{
			line(img, fourPoint2f[i], fourPoint2f[i + 1]
				, Scalar(180,255,255));
		}
		line(img, fourPoint2f[0], fourPoint2f[3], Scalar(180,255,255));
		Point2f target;
		target.x = fourPoint2f[0].x +fourPoint2f[1].x +fourPoint2f[2].x +fourPoint2f[3].x; 
		target.x = target.x/4;
		target.y = fourPoint2f[0].y +fourPoint2f[1].y +fourPoint2f[2].y +fourPoint2f[3].y; 
		target.y = target.y/4;

		cout << target <<endl;
		return target;

}


Point2f getPredicentPosition( Point2f a, Point2f r, double speed, double times ,double rad){
	// 此函数用于计算预测的击打点坐标
	// a 代表 目标装甲中心坐标
	// r 代表 旋转中心R坐标
	double w = speed/rad;
	double  angle = w * times; // 预测提前的角度值
	double PI = 3.1415;
	Point2f predict_point;
	predict_point.x =  (r.x + (a.x - r.x) * cos( (-1) * angle * PI / 180.0) - (r.y - a.y) * sin( (-1)*angle * PI / 180.0)) ;
    predict_point.y =  (r.y - (a.x - r.x ) * sin( (-1) *angle * PI / 180.0) - (r.y - a.y) * cos( (-1)*angle * PI / 180.0)) ;
    return predict_point;
	
    
}
void bigWindMill( Mat img, Point2f a, Point2f r){//处理大风车图片
	   
          //Mat img = imread("//home//longxuan//longxuan//wind//hello//1111.png");
		 // Mat img;
		  //VideoCapture cap;
		  //cap.open("/home/longxuan/longxuan/wind/Windmills/wind.mp4");
		  //cout << "11111111"<<endl;
		  int delay = 30;
		  if ( img.empty() ) return;
			    //resize(img,img,Size(img.cols*0.5,img.rows*0.5));
       
			  //图像预处理
		  //分割颜色通道
          vector<Mat> imgChannels;
          split(img,imgChannels);
          //获得目标颜色图像的二值图
         // RED
          //Mat midImage=imgChannels.at(2)-imgChannels.at(0);//识别红色

          Mat midImage=imgChannels.at(0)-imgChannels.at(2);//识别蓝色

		  threshold(midImage,midImage,100,255,CV_THRESH_BINARY);
        
		  //膨胀
		  //dilate(binary,midImage,Mat());//膨胀操作
		int structElementSize=1;
		Mat element=getStructuringElement(MORPH_RECT,Size(2*structElementSize+1,2*structElementSize+1),Point(structElementSize,structElementSize));
		dilate(midImage,midImage,element);

		//闭运算，消除扇叶上可能存在的小洞
		structElementSize=3;
		element=getStructuringElement(MORPH_RECT,Size(2*structElementSize+1,2*structElementSize+1),Point(structElementSize,structElementSize));
		morphologyEx(midImage,midImage, MORPH_CLOSE, element);
        Mat src = midImage;
	   
		//查找轮廓
		vector<vector<Point>> contours2;
		vector<vector<Point>> contours3;
        int target_2;
		int target_2_R;
		int target_3;
		vector<Vec4i> hierarchy2;
		vector<Vec4i> hierarchy3;
		findContours(midImage,contours2,hierarchy2,CV_RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
		findContours(src,contours3,hierarchy3,CV_RETR_TREE,CHAIN_APPROX_SIMPLE);

		RotatedRect rect_tmp2;
		bool findTarget=0;
        //cout << contours2.size()<<endl;
		int len = contours2.size();
		for ( int i =0; i <len; i++  ){
			int area = contourArea(contours2[i]);
			int perimeter = arcLength(contours2[i],1);
			//cout << area<< "  "<<i<<" "<<perimeter<<" "<< area/perimeter<<endl;
			
			if (  isTargetFLow( contours2[i]) ){
                      target_2 = i;
			}
			if ( isCenterR( contours2[i] ) ){
				     target_2_R = i;

			}
		}

		int Len = contours3.size();
		for ( int i = 0; i < Len; i++  ){
			if  ( contours3[i] == contours2[target_2] ){
				target_3 = i + 1;
			}
		}
		//cout << target_3<<endl;
		//cout << target_2_R <<"#####################"<<endl;
		drawContours( img, contours3,target_3,Scalar(255,0,255));
		drawContours( img,contours2,target_2_R,Scalar(255,255,255) );
		if ( contours3.size() != 0 && contours2.size() != 0 && target_3 != 0 && target_2_R != 0){
			a = getzuobiao( contours3[ target_3] ,img);
		    r = getzuobiao( contours2[ target_2_R ],img );
		    circle( img, r, 8,Scalar( 0,0,255) , 2  );
		    circle( img, a, 8,Scalar( 0,0,255) , 2  );
		}
		
		



		    //  imshow( "processed",midImage);;
		    //imshow( "bin", img);
			
			
		    if(delay>=0&&waitKey (delay)>='p')
                   waitKey(0);

		 
}