# README.md

##### 一、文件结构：

lightTracker/lightTracker.cpp		---->	lightTracker程序的源文件（written in C++），lightTracker.exe缺失时可重新编译

lightTracker/lightTracker.exe		---->	lightTracker程序的可执行文件，双击即可运行

lightTracker/params.txt				 ---->	lightTracker程序的参数文件，存放透镜的参数信息（规格，位置）

lightTracker/params_guide.txt	 ---->	填写params.txt的指导文件，请按照本文件的数据格式来填写params.txt

lightTracker/Readme.md		   	---->    即本文件

##### 二、程序功能：

平行光光线追迹：计算平行光在（多个）球面镜的近轴区发生折射的像方截距和像方孔径角，进而得到像方焦距

##### 三、注意事项：

1. 按照params_guide.txt 的格式来填写数据（注意第一行仅有一个数据，这是因为没有“与第0个透镜的距离”）。
2. 注意单位是mm

##### 四、编译指南：

如果需要重新编译lightTracker.cpp，尽量使用命令行指令：

`g++ lightTracker.cpp -o lightTracker.exe`

这是因为IDE的路径设置可能会影响lightTracker.cpp中相对路径

##### 五、最终解决方案

如果仍然无法编译，可以自行搜索“c++命令行编译”或参考博文“https://blog.csdn.net/weixin_45676049/article/details/108018850”

如果有任何疑问、问题、意见或建议，**欢迎私戳作者qq:1308592371** 

本程序因你的使用而精彩！ :)

附注：本程序的完备文件托管于github(https://github.com/yang2019901/allCode/tree/main/lightTracker)