#ifndef QFACEOBJECT_H
#define QFACEOBJECT_H

#include <QObject>
#include <seeta/FaceEngine.h>
#include <opencv.hpp>
//人脸数据存储，人脸检测，人脸识别
class QFaceObject : public QObject
{
    Q_OBJECT
public:
    explicit QFaceObject(QObject *parent = nullptr);
    ~QFaceObject();

public slots:
    //人脸注册
    int64_t face_register(cv::Mat& faceImage);//整型数
    //负责查询输入人脸在数据库的匹配，人脸查询
    int face_query(cv::Mat &faceImage);
signals:
    void send_faceid(int64_t faceid);
private:
    seeta::FaceEngine *fengineptr;
};

#endif // QFACEOBJECT_H
