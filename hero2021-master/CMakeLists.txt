cmake_minimum_required(VERSION 3.5)
project(RM2018_Adv)

find_package( OpenCV REQUIRED )
find_package( Threads )
add_definitions(-w)
set(CMAKE_CXX_STANDARD 11)
include_directories(C:\Users\Lenovo\Desktop\hero2021-master/include)
link_directories(C:\Users\Lenovo\Desktop\hero2021-master/darknet/)
add_executable(RM2018_Adv main.cpp configurations.hpp util.hpp serial.hpp camview.hpp 
                dnnManager.hpp autolead.hpp armorDetector.hpp armorTracker.hpp
               predictor.hpp armorHiter.hpp dfcHiter.hpp dfc.hpp custom_definations.hpp KMSolver.hpp trackerMedianFlow.hpp autoTracker.hpp)

target_link_libraries( RM2018_Adv darknet.so ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT} )
