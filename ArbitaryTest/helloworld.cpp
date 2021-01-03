#include <iostream>
#include <algorithm>
#include <vector> 
// #include <chrono>
#include <opencv2/opencv.hpp>

using namespace std;
// using namespace chrono;
using namespace cv;


int main()
{
    Mat img;
    int a = 100000;
    img = imread("C:/Users/Lenovo/Pictures/Saved Pictures/geometricalPicture2.jpg");
    if (!img.empty())
    {
        resize(img, img, Size(960,540));
        imshow("this is a test image", img);
        a = waitKey(0);
        destroyAllWindows();
    }
    else 
        printf("cannot open the picture!\n");
    printf("%d\n", a);
    printf("%f\n", abs(a/1000.0));
    system("pause");
    return 0;

    // auto t1 = system_clock::now();
    // for (int i = 0; i < 100000;)
    //     i += 1;
    // auto t2 = system_clock::now();
    // auto duration = duration_cast<microseconds>(t2-t1);
    // printf("use time %d us", duration.count());
    // return 0;
}