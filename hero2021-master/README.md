# Hero2021

这一版程序包含了决赛阶段算法的完整框架，对于神符模块和自动引导模块只需要编写对应的class文件即可，调试代码可以写在main文件尾部的两个函数中;运行程序前修改configuration中的mainEntry使程序跳转到对应功能模块就可以调试运行了。
对于dsfHiter.hpp 和 autoleader.hpp 的修改有：
   继承 ModuleBase 时，使用 public 继承
   初始化ModuleBase时，autoleader修改了ModuleID
   Update函数修改了参数，帧信息数据结构ImageData包含了该帧图像以及该帧图像对应的电控信息，使用电控信息时应该从这里获取

调试时，注意选择对应你摄像头的分辨率，通过Configuration可以修改，而且需要先标定摄像头，修改程序入口为10可以进入标定程序，缺少标定文件可能会报错。摄像头标定如果不用标定板也可以在网上百度棋盘格的图片，自行数出格点的个数，量出边长，也可以完成标定。

使用宏SET_CONFIG_VARIABLE系列，可以快速指定通过config动态更新的变量，需要注意通过此方法设置需要保证config中变量名与设定的变量名完全一致
编译程序前，请修改custom_definations文件 修改路径和设备类型，使用jetson TX1/2选择DEVICE_TX，其他基本都可以选DEVICE_MANIFOLD，目前这个选择只会影响摄像头设备，这是因为TX有一个自带的摄像头

框架实现的一些功能模块（CameraView,ConfigurationVariables,SerialManager,...）的使用可以参照装甲相关程序

这一版程序串口还是秒算的串口 TX版本串口暂时不能使用...
Good luck!