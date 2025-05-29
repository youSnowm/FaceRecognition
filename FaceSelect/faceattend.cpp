#include "faceattend.h"
#include "ui_faceattend.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
FaceAttend::FaceAttend(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FaceAttend)
{
    ui->setupUi(this);
    //打开摄像头
    cap.open(0);//dev/video
    //启动定时器事件
    if (!cap.isOpened()) {
        // 如果摄像头无法打开，显示错误信息
        QMessageBox::critical(this, "错误", "无法打开摄像头");
        return;
    }
    //定时器每隔 100 毫秒触发一次 timerEvent(QTimerEvent *e) 函数
    startTimer(100);

    //导入级联分类器文件
    cascade.load("D:/python/opencv/Opencv4.5.1/opencv-build/install/etc/haarcascades/haarcascade_frontalface_alt2.xml");

    //QTcpSocket当断开连接disconnect，成功发送connect
    connect(&msocket,&QTcpSocket::disconnected,this,&FaceAttend::start_connect);
    connect(&msocket,&QTcpSocket::connected,this,&FaceAttend::stop_connect);
    //关联接收数据的槽函数
    connect(&msocket,&QTcpSocket::readyRead,this,&FaceAttend::recv_data);

    //定时器连接服务器
    connect(&mtimer,&QTimer::timeout,this,&FaceAttend::time_connect);
    //启动定时器
    mtimer.start(5000);//每五秒连接一次
    flag=0;
    //ui->widgetLb->hide();
    ui->widgetLb->hide();
}

FaceAttend::~FaceAttend()
{
    delete ui;
}

//使用定时器事件采集摄像头数据
void FaceAttend::timerEvent(QTimerEvent *e)
{
    //采集数据
    Mat srcImage;
    if(cap.grab())
    {
        cap.read(srcImage);//读取yizhen数据
    }

    //把图片大小设与显示窗口一样大
    cv::resize(srcImage,srcImage,Size(480,480));

    Mat grayImage;
    //转灰度图
    cvtColor(srcImage,grayImage,COLOR_BGR2GRAY);
    //检测人脸数据
    std::vector<Rect> faceRects;
    cascade.detectMultiScale(srcImage,faceRects);
    if(faceRects.size()>0 && flag>=0)
    {
        Rect rect= faceRects.at(0);//第一个人脸的矩形框
        //将图片的标签框跟人脸重合
        ui->headpicture->move(rect.x,rect.y);

        if(flag>2)
        {
            //baMat数据转换成QbyteArray,-->>编码成jpg格式
            std::vector<uchar> buf;
            //bool cv::imencode(const std::string& ext, cv::InputArray img, std::vector<uchar>& buf, const std::vector<int>& params = std::vector<int>());
            //ext-- jpg,png,bmp等格式,img-- cv::Mat格式的img,buf-- std::vector<uchar> bar 代表存储的缓冲区
            cv::imencode(".jpg",srcImage,buf);
            //(const char*)buf.data() 是将 uchar 类型的指针强制转换为 char 类型的指针。
            //这是因为 QByteArray 的构造函数需要一个 const char* 类型的参数。
            QByteArray byte((const char*)buf.data(),buf.size());

            //准备发送
            quint64 backsize=byte.size();//获取字节数组的大小
            QByteArray sendData;
            //将数据写入 QByteArray 对象 sendData
            QDataStream stream(&sendData,QIODevice::WriteOnly);
            stream.setVersion(QDataStream::Qt_5_14);
            //stream << data; 的操作时，这些数据被序列化并写入到 sendData 中。
            stream<<backsize<<byte;
            //发送
            msocket.write(sendData);
            flag=-2;
            faceMat = srcImage(rect);
            //保存
            imwrite("./face.jpg",faceMat);
        }
        flag++;
    }
    if(faceRects.size()==0)
    {
        //把人脸框移动到中心位置
        ui->headpicture->move(100,60);
        ui->widgetLb->hide();
        flag=0;
    }

    if(srcImage.data==nullptr) return;
    //把opencv里面的Mat格式数据（bgr）转Qt里面QImage(RGB)
    cvtColor(srcImage,srcImage,COLOR_BGR2RGB);
    QImage image(srcImage.data,srcImage.cols,srcImage.rows,srcImage.step1(),QImage::Format_RGB888);
    QPixmap map=QPixmap::fromImage(image);

    ui->videoLb->setPixmap(map);
}

//接收数据
void FaceAttend::recv_data()
{
//    QString msg=msocket.readAll();
//    qDebug()<<msg;
//    ui->lineEdit->setText(msg);
    QByteArray array=msocket.readAll();
    qDebug()<<array;
    //-------------Json解析------------------------
    QJsonParseError err;
    QJsonDocument doc=QJsonDocument::fromJson(array,&err);
    if(err.error!=QJsonParseError::NoError)
    {
        qDebug()<<"数据错误";
        return;
    }
    QJsonObject obj=doc.object();
    QString employeeID=obj.value("employeeID").toString();
    QString name=obj.value("name").toString();
    QString department=obj.value("department").toString();
    QString timestr=obj.value("time").toString();

    ui->numberEdit->setText(employeeID);
    ui->nameEdit->setText(name);
    ui->departmentEdit->setText(department);
    ui->timeEdit->setText(timestr);

     //通过样式来显示图片
    ui->label_headLb->setStyleSheet("border-radius:75px;border-image: url(./face.jpg);");
    ui->widgetLb->show();
}

void FaceAttend::time_connect()
{
    //连接服务器
    msocket.connectToHost("192.168.0.178",9999);
    qDebug()<<"正在连接服务器";
}

void FaceAttend::stop_connect()
{
    //停止定时器
    mtimer.stop();
    qDebug()<<"连接服务器成功";
}

//启动连接
void FaceAttend::start_connect()
{
    mtimer.start(5000);//启动定时器
    qDebug()<<"断开连接";
}
