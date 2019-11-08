# CG_project
Stochastic Motion Textures

Compilation Instructions -
-Install OpenCV on your platform.
-Include the following path and libraries in your project :
  For Windows : (https://wiki.qt.io/How_to_setup_Qt_and_openCV_on_Windows)
  
    INCLUDEPATH += C:\opencv-build\install\include

    LIBS += C:\opencv-build\bin\libopencv_core411.dll
    LIBS += C:\opencv-build\bin\libopencv_highgui411.dll
    LIBS += C:\opencv-build\bin\libopencv_imgcodecs411.dll
    LIBS += C:\opencv-build\bin\libopencv_imgproc411.dll
    LIBS += C:\opencv-build\bin\libopencv_features2d411.dll
    LIBS += C:\opencv-build\bin\libopencv_calib3d411.dll
    LIBS += C:\opencv-build\bin\libopencv_photo411.dll
        
    
  For Mac :
  
    LIBS += -L/usr/local/bin \
     -lopencv_core \
     -lopencv_imgproc \
     -lopencv_features2d\
     -lopencv_highgui \
     -lopencv_imgcodecs \
     -lopencv_highgui \
     -lopencv_calib3d \
     -lopencv_photo
     
    mac: CONFIG += MAC_CONFIG

    MAC_CONFIG {
        QMAKE_CXXFLAGS = -std=c++11 -stdlib=libc++ -mmacosx-version-min=10.7
        QMAKE_LFLAGS = -std=c++11 -stdlib=libc++ -mmacosx-version-min=10.7

Usage Instructions (Segmentation and Matting) -
Select a rectangular area around the object you want to segment\n" <<
Hotkeys :

ESC - quit the program and proceed.
r - restore the original image
n - next iteration

left mouse button - set rectangle

CTRL+left mouse button - set GC_BGD pixels
SHIFT+left mouse button - set GC_FGD pixels

CTRL+right mouse button - set GC_PR_BGD pixels
SHIFT+right mouse button - set GC_PR_FGD pixels
