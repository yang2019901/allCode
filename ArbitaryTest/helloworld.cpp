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
    double x, y;
    while (cin >> x >> y)
    {
        cout << atan2(y, x) * 180 / 3.1415926535;
    }
    return 0;
}