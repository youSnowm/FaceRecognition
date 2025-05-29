#ifndef FACEATTEND_H
#define FACEATTEND_H

#include <QMainWindow>
#include <opencv.hpp>
#include <QMessageBox>
#include <QTcpSocket>
#include <QTimer>
#include <QDebug>
using namespace cv;
using namespace std;
QT_BEGIN_NAMESPACE
namespace Ui { class FaceAttend; }
QT_END_NAMESPACE

class FaceAttend : public QMainWindow
{
    Q_OBJECT

public:
    FaceAttend(QWidget *parent = nullptr);
    ~FaceAttend();

    //定时器事件
    void timerEvent(QTimerEvent *e);

private slots:
    void time_connect();
    void stop_connect();
    void start_connect();

    void recv_data();
private:
    Ui::FaceAttend *ui;

    //摄像头
    VideoCapture cap;

    //hear--级联分类器--检测人脸
    cv::CascadeClassifier cascade;

    //创建网络套接字，定时器
    QTcpSocket msocket;
    QTimer mtimer;

    //标志是否是同一个人脸进入到识别区域
    int flag;

    //保存人类的数据
    cv::Mat faceMat;
};
#endif // FACEATTEND_H
