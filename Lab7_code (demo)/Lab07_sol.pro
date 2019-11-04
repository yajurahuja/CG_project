QT += core gui opengl
win32: CONFIG-=debug_and_release

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Lab07sol
TEMPLATE = app

DESTDIR = .

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
INCLUDEPATH += . src depends/glm
DEFINES += NDEBUG

OBJECTS_DIR=build
MOC_DIR=build
UI_DIR=build
RCC_DIR=build

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


CONFIG += c++11

# Input
HEADERS +=  ./src/mainwindow.h \
            ./src/openglwidget.h \
            ./src/stb_image.h

FORMS +=    ./src/mainwindow.ui

SOURCES +=  ./src/main.cpp \
            ./src/mainwindow.cpp \
            ./src/openglwidget.cpp

RESOURCES += \
    shaders.qrc

DISTFILES +=

# Platform specific customizations
win32: LIBS += opengl32.lib
macx{
    CONFIG-=app_bundle
}
