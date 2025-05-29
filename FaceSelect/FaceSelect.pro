QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    faceattend.cpp

HEADERS += \
    faceattend.h

FORMS += \
    faceattend.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    image.qrc
#添加opencv，seetface库
INCLUDEPATH += D:\python\opencv\Opencv4.5.1\opencv-build\install\include
INCLUDEPATH += D:\python\opencv\Opencv4.5.1\opencv-build\install\include\opencv2
INCLUDEPATH += D:\csd\qt\qt-projrct\AiProject\build_SeetFace2\install\include
INCLUDEPATH += D:\csd\qt\qt-projrct\AiProject\build_SeetFace2\install\include\seeta


LIBS += -L D:\python\opencv\Opencv4.5.1\opencv-build\install\x64\mingw\lib\libopencv_*.a
LIBS+=D:\csd\qt\qt-projrct\AiProject\build_SeetFace2\install\lib\libSeeta*
