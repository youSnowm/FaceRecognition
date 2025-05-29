#include "qfaceobject.h"
#include <QDebug>
QFaceObject::QFaceObject(QObject *parent) : QObject(parent)
{
    //初始化
    seeta::ModelSetting FDmode("D:/csd/qt/qt-projrct/AiProject/SeetaFace/bin/model/fd_2_00.dat",seeta::ModelSetting::CPU,0);
    seeta::ModelSetting PDmode("D:/csd/qt/qt-projrct/AiProject/SeetaFace/bin/model/pd_2_00_pts5.dat",seeta::ModelSetting::CPU,0);
    seeta::ModelSetting FRmode("D:/csd/qt/qt-projrct/AiProject/SeetaFace/bin/model/fr_2_10.dat",seeta::ModelSetting::CPU,0);
    this->fengineptr=new seeta::FaceEngine(FDmode,PDmode,FRmode);

    //导入已有的人脸数据库
    this->fengineptr->Load("./face.db");
}
QFaceObject::~QFaceObject()
{
    delete fengineptr;
}

int64_t QFaceObject::face_register(cv::Mat &faceImage)
{
    //把opencv的Mat数据转为seetaface的数据
    SeetaImageData sImage;
    sImage.data=faceImage.data;
    sImage.width=faceImage.cols;
    sImage.height=faceImage.rows;
    sImage.channels=faceImage.channels();

    int64_t faceID=this->fengineptr->Register(sImage);//注册返回一个人脸id
    if(faceID>=0)
    {
        fengineptr->Save("./face.db");
    }
    return faceID;
}

int QFaceObject::face_query(cv::Mat &faceImage)
{
    //把opencv的Mat数据转为seetaface的数据
    SeetaImageData sImage;
    sImage.data=faceImage.data;
    sImage.width=faceImage.cols;
    sImage.height=faceImage.rows;
    sImage.channels=faceImage.channels();
    float similarity=0;
    int64_t faceid = fengineptr->Query(sImage,&similarity);//运算时间比较长
    if(similarity>0.85)
    {
        emit send_faceid(faceid);
    }
    else {
        emit send_faceid(-1);
    }
    return faceid;
}
