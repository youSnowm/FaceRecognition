# FaceRecognition
基于Qt的人脸识别考勤系统
##                                      Arm+Qt+Opencv


### 第一步：创建客户端项目---faceSelect

1.先实现考勤页面设计，然后显示摄像头数据

1.选择MainWidget，创建文件FaceSelect。会生成.cpp和.pro等文件。

2.faceattend.ui

进入ui界面，先实时显示由摄像头传输的图片(一帧传)，拖拽QWidget，并添加QLabel控件，图片就显示在QLabel上。
左边界面如下

具体实现如下

代码文件：

faceattend.h和faceattend.cpp

faceattend.h：

```c++
#include <opencv.hpp>
#include <QMessageBox>
#include <QDebug>

//添加命名空间，cv::VideoCapture
using namespace cv;
using namespace std;

	//计时器事件--显示摄像头数据
public:	
    void timerEvent(QTimerEvent *e);
private:  
    //定义一个摄像头
    VideoCapture cap;
```

faceattend.cpp：添加代码

主函数：

```c++
ui->setupUi(this);
    //先打开摄像头
    cap.open(0);
    if(!cap.isOpened())
    {
        QMessageBox::critical(this,"错误","无法打开摄像头");
        return;
    }
    //按1s启动计时器事件
    startTimer(100);
```

timerEvent:

```c++
//计时器事件
void FaceAttend::timerEvent(QTimerEvent *e)
{
    //采集数据
    Mat srcImage;
    //尝试抓取一帧
    if(cap.grab())
    {
        cap.read(srcImage);
    }
    if(srcImage.data==nullptr) return;
    //把opencv中的mat格式数据bgr,转换成qt中的image数据RGB
    cvtColor(srcImage,srcImage,COLOR_BGR2RGB);
    //需要将 OpenCV 图像显示在 Qt 的控件上时,使用 QImage 类来将 OpenCV 的 Mat 对象转换为 Qt 中的图像对象
    //srcImage.data--是图像像素的起始地址,cols列数，rows行数，step1在不同行之间移动指针。RGB888代表三通道
    QImage image(srcImage.data,srcImage.cols,srcImage.rows,srcImage.step1(),QImage::Format_RGB888);
    QPixmap map=QPixmap::fromImage(image);
    ui->videoLb->setPixmap(map);
}
```

**2.检测人脸并实时显示，检测框实时跟踪**



代码文件：
**faceattend.h和faceattend.cpp**

faceattend.h：

```c++
    //级联分类器----检测人脸
    cv::CascadeClassifier cascade;
```

faceattend.cpp

主函数:

```c++
//导入级联分类器文件,识别人脸
    cascade.load("D:/python/opencv/Opencv4.5.1/opencv-build/install/etc/haarcascades/haarcascade_frontalface_alt2.xml");

```

timerEvent:

```c++
 //使用CasecadeClassifier方法时要转为灰度图
    Mat grayImage;
    cvtColor(srcImage,grayImage,COLOR_BGR2GRAY);
    //检测人脸
    std::vector<Rect> faceRects;
    cascade.detectMultiScale(srcImage,faceRects);
    if(faceRects.size()>0)
    {
        Rect rect = faceRects.at(0);//创建第一个人脸的矩形框
        //移动人脸框
        ui->headpicture->move(rect.x,rect.y);
    }
    else
    {
        //把人脸框移动到中心位置
        ui->headpicture->move(100,60);
    }
```



**3.在客户端建立连接**

.pro文件：QT       += core gui network

主题思想：创建网络套接字msocket，和定时器mtimer

创建三个槽函数，分别代表连接，停止，开始

```
timer_connect();//连接
void stop_connect();//停止
void start_connect();//开始
```


具体实现如下

代码文件：

.pro文件：QT       += core gui network

**faceattend.h和faceattend.cpp**

faceattend.h：

```c++
private slots:
    void timer_connect();//连接
    void stop_connect();//停止
    void start_connect();//开始    
private:    
    //创建网络套接字，定时器
    QTcpSocket msocket;
    QTimer mtimer;
```

faceattend.cpp

主函数:

```c++
 //QTcpSocket当断开时disconnect，
    		      connect(&msocket,&QTcpSocket::disconnected,this,&FaceAttend::start_connect);
    connect(&msocket,&QTcpSocket::connected,this,&FaceAttend::stop_connect);

    //定时器连接服务器,mtimer-定时器，定时时间到就自动连接
    connect(&mtimer,&QTimer::timeout,this,&FaceAttend::timer_connect);
    mtimer.start(5000);//5s启动一次，知道连接成功
```



```c++
//连接服务器
void FaceAttend::timer_connect()
{
    msocket.connectToHost("192.168.0.178",9999);
    qDebug()<<"正在连接服务器";
}
//停止服务器
void FaceAttend::stop_connect()
{
    mtimer.stop();
}
//启动服务器
void FaceAttend::start_connect()
{
    mtimer.start(5000);
}

```
**接着跳转到AttendanceServer服务端**

### AttendanceServer服务端
**4.客户端和服务端建立连接**

代码文件：

**attendserver.h和attendserver.cpp**

attendserver.h：

```c++
private slots:
    void accept_client();
    void read_data();
    
 private:
    QTcpServer mserver;
    QTcpSocket *msocket;   
    
```

attendserver.cpp

主函数:

```
//qtcpServer当有客户端连会发送newconnect
    connect(&mserver,&QTcpServer::newConnection,this,&AttendServer::accept_client);
    mserver.listen(QHostAddress::Any,9999);//监听，启动服务器
```



```c++
void AttendServer::accept_client()
{
    //获取与客户端通信的套接字
    msocket = mserver.nextPendingConnection();

    //当客户端有数据到达会发送readyRead
    connect(msocket,&QTcpSocket::readyRead,this,&AttendServer::read_data);
}

//读取客户端发送的数据
void AttendServer::read_data()
{
    //读取所有数据
    QString msg=msocket->readAll();
    qDebug()<<msg;
}
```

**5.服务端显示图像**

1.在客户端将图像数据打包，使用stream,QByteArray方法

faceattend.cpp

```c++
//把Mat数据转换为QbyteArray,编码成jpg格式
        std::vector<uchar> buf;
        cv::imencode(".jpg",srcImage,buf);
        QByteArray byte((const char*)buf.data(),buf.size());
        //准备发送
        quint64 backsize=byte.size();
        QByteArray sendData;
        QDataStream stream(&sendData,QIODevice::WriteOnly);
        stream.setVersion(QDataStream::Qt_5_14);
        stream<<backsize<<byte;
        //发送
        msocket.write(sendData);
```

attendserver.h

```c++
private:
quint64 bsize;
```

attendserver.cpp

主函数:

```
bsize=0;
```

```c++
//读取客户端发送的数据
void AttendServer::read_data()
{
   QDataStream stream(msocket);//把套接字绑定到数据流stream上
   stream.setVersion(QDataStream::Qt_5_14);
	
   if(bsize==0)
   {
       if(msocket->bytesAvailable()<(qint64)sizeof(bsize)) return;
       //采集数据长度
       stream>>bsize;
   }
   if(msocket->bytesAvailable()<bsize)
   {
       return;
   }
   QByteArray data;
   stream>>data;
   bsize=0;
   if(data.size()==0)
   {
       return;
   }
   //显示图片
   QPixmap map;
   map.loadFromData(data,"jpg");//loadFromData从二进制数据解析图像
   map=map.scaled(ui->pictureLb->size());//scaled是缩放
   ui->pictureLb->setPixmap(map);
}
```

**6.实现人脸模块的封装注册登录**
1.新建类--------QFaceObject类

qfaceobject.h

```c++
#include <seeta/FaceEngine.h>
#include <opencv.hpp>
```

```c++
private slots:
//返回注册id
    int64_t face_register(cv::Mat& faceImage);//整型数
//负责查询输入人脸在数据库的匹配,并根据相似度进行反馈。
    int face_query(cv::Mat& faceImage);
signals:
    void send_faceid(int64_t faceid);
private:
    seeta::FaceEngine *fengineptr;
```

qfaceobject.cpp

主函数

```c++
 //初始化
    seeta::ModelSetting FDmode("D:/csd/qt/qt-projrct/AiProject/SeetaFace/bin/model/fd_2_00.dat",seeta::ModelSetting::CPU,0);
    seeta::ModelSetting PDmode("D:/csd/qt/qt-projrct/AiProject/SeetaFace/bin/model/pd_2_00_pts5.dat",seeta::ModelSetting::CPU,0);
    seeta::ModelSetting FRmode("D:/csd/qt/qt-projrct/AiProject/SeetaFace/bin/model/fr_2_10.dat",seeta::ModelSetting::CPU,0);
    this->fengineptr=new seeta::FaceEngine(FDmode,PDmode,FRmode);

    //导入已有的人脸数据库
    this->fengineptr->Load("./face.db");
```

----------------------------------

```c++
int64_t QFaceObject::face_register(cv::Mat &faceImage)
{
    // 把 OpenCV 的 Mat 格式转换为 SeetaFace 的 SeetaImageData 格式
    SeetaImageData sImage;
    sImage.data = faceImage.data;         // 指向图像数据的指针
    sImage.width = faceImage.cols;        // 图像的宽度（列数）
    sImage.height = faceImage.rows;       // 图像的高度（行数）
    sImage.channels = faceImage.channels(); // 图像的通道数（例如，彩色图像为 3）

    // 使用 SeetaFace 引擎进行人脸注册，返回注册的人脸 ID
    int64_t faceID = this->fengineptr->Register(sImage);
    if (faceID >= 0)                      // 如果返回的人脸 ID 有效
    {
        // 将注册信息保存到数据库文件中
        fengineptr->Save("./face.db");
    }
    return faceID;                        // 返回人脸 ID
}

int QFaceObject::face_query(cv::Mat &faceImage)
{
    // 把 OpenCV 的 Mat 格式转换为 SeetaFace 的 SeetaImageData 格式
    SeetaImageData sImage;
    sImage.data = faceImage.data;         // 指向图像数据的指针
    sImage.width = faceImage.cols;        // 图像的宽度（列数）
    sImage.height = faceImage.rows;       // 图像的高度（行数）
    sImage.channels = faceImage.channels(); // 图像的通道数

    float similarity = 0;                 // 用于存储查询结果的相似度
    // 使用 SeetaFace 引擎查询人脸，返回匹配的人脸 ID，同时更新相似度
    int64_t faceid = fengineptr->Query(sImage, &similarity);
    return faceid;                        // 返回查询到的人脸 ID
}

```

**7.数据库和表格创建**

AttendanceServer

main.cpp

```c++
 //连接数据库
    QSqlDatabase db=QSqlDatabase::addDatabase("QSQLITE");
    //设置数据库名称
    db.setDatabaseName("server.db");
    //打开数据库
    if(!db.open())
    {
        qDebug()<<db.lastError().text();
        return -1;
    }
    //创建员工信息表格
    QString createsql = "create table if not exists employee(employeeID integer primary key autoincrement,name varchar(256), sex varchar(32),"
                            "birthday text, address text, phone text, faceID integer unique, headfile text)";
    QSqlQuery query;
    if(!query.exec(createsql))
    {
        qDebug()<<query.lastError().text();
        return -1;
    }

    //考勤表格
        createsql = "create table if not exists attendance(attendaceID integer primary key autoincrement, employeeID integer,"
                    "attendaceTime TimeStamp NOT NULL DEFAULT(datetime('now','localtime')))";
        if(!query.exec(createsql))
        {
            qDebug()<<query.lastError().text();
            return -1;
        }
```

**8.注册页面设计**


**9.员工个人数据注册并导入到数据库,采集头像数据**


五个按钮的槽函数，和定时器事件

registerwin.h

```c++
public:
    void timerEvent(QTimerEvent *e);
private slots:
    void on_resetBt_clicked();
    void on_addpicBt_clicked();
    void on_registerBt_clicked();
    void on_videoBt_clicked();
    void on_cameraBt_clicked();  
private:
    int timerid;
    cv::VideoCapture cap;
    cv::Mat image;
```

registerwin.cpp

```c++
//实时显示摄像头数据
void Registerwin::timerEvent(QTimerEvent *e)
{
    //获取摄像头数据并且显示
    if(cap.isOpened())
    {
        cap>>image;
        if(image.data==nullptr) return;
    }

    //Mat转QImage
    cv::Mat rgbImage;
    cv::cvtColor(image,rgbImage,cv::COLOR_BGR2RGB);
    QImage qImage(rgbImage.data,rgbImage.cols,rgbImage.rows,rgbImage.step1(),QImage::Format_RGB888);
    //在qt界面上显示
    QPixmap map=QPixmap::fromImage(qImage);
    map=map.scaledToWidth(ui->headpicLb->width());
    ui->headpicLb->setPixmap(map);

}

void Registerwin::on_resetBt_clicked()
{
    //清空数据
    ui->nameEdit->clear();
    ui->birthdayEdit->setDate(QDate::currentDate());//设置当前日期
    ui->addressEdit->clear();
    ui->phoneEdit->clear();
    ui->picFileEdit->clear();
}

//添加图像
void Registerwin::on_addpicBt_clicked()
{
    //通过文件对话框 选中图片路径
    QString filepath=QFileDialog::getOpenFileName(this);
    ui->picFileEdit->setText(filepath);

    //显示图片
    QPixmap map(filepath);
    map=map.scaledToWidth(ui->headpicLb->width());
    ui->headpicLb->setPixmap(map);
}

//注册
void Registerwin::on_registerBt_clicked()
{
    //1,通过照片，结合faceObject模块得到faceID
    QFaceObject faceobj;
    cv::Mat image=cv::imread(ui->picFileEdit->text().toUtf8().data());
    int faceID=faceobj.face_register(image);
    qDebug()<<faceID;

    //把头像保存到一个固定路径下
    QString headfile=QString("./data/%1.jpg").arg(QString(ui->nameEdit->text().toUtf8().toBase64()));
    cv::imwrite(headfile.toUtf8().data(),image);

    //2,把个人信息存储到数据库employee
    QSqlTableModel model;
    model.setTable("employee");//设置表名
    QSqlRecord record=model.record();
    //设置记录数据
    record.setValue("name",ui->nameEdit->text());
    record.setValue("sex",ui->mrb->isChecked()?"男":"女");
    record.setValue("birthday",ui->birthdayEdit->text());
    record.setValue("address",ui->addressEdit->text());
    record.setValue("phone",ui->phoneEdit->text());
    record.setValue("faceID",faceID);
    //头像路径
    record.setValue("headfile",headfile);
    //把记录插入到数据表格中
    bool ret=model.insertRecord(0,record);
    //3,提示注册成功
    if(ret)
    {
        QMessageBox::information(this,"注册提示","注册成功");
        //提交
        model.submitAll();
    }
    else {
         QMessageBox::critical(this,"注册提示","注册失败");
    }
}

//打开摄像头
void Registerwin::on_videoBt_clicked()
{
    if(ui->videoBt->text()=="打开摄像头")
    {
        //打开摄像头
        if(cap.open(0))
        {
            ui->videoBt->setText("关闭摄像头");
            //启动定时器事件
            timerid=startTimer(100);
        }
    }
    else
    {
        killTimer(timerid);//关闭定时器事件
        ui->videoBt->setText("打开摄像头");
        //关闭摄像头
        cap.release();
    }
}
//员工信息注册模块头像采集
//拍照
void Registerwin::on_cameraBt_clicked()
{
    //保存数据
    //把头像保存到一个固定路径下
    QString headfile=QString("./data/%1.jpg").arg(QString(ui->nameEdit->text().toUtf8().toBase64()));
    ui->picFileEdit->setText(headfile);
    cv::imwrite(headfile.toUtf8().data(),image);
    killTimer(timerid);//关闭定时器事件
    ui->videoBt->setText("打开摄像头");
    //关闭摄像头
    cap.release();
}

```

**10.接收客户端人脸数据，识别人脸id,使用线程来处理QThread**

attendserverwin.h

```c++
signals:
    void query(cv::Mat& image);
private slots:
    void recv_faceid(int64_t faceid);
QFaceObject fobj;
```

attendserverwin.cpp

主函数：

```c++
//创建线程
    QThread *thread=new QThread();
    //把QFaceObject对象移动到thread线程z中执行
    fobj.moveToThread(thread);
    //启动线程
    thread->start();
    connect(this,&AttendServerWin::query,&fobj,&QFaceObject::face_query);
    //关联QFaceObject对象里面的send_face对象
   connect(&fobj,&QFaceObject::send_faceid,this,&AttendServerWin::recv_faceid);
}
```

```c++
//读取客户端发送的数据
void AttendServerWin::read_data()
{
	//识别人脸
    cv::Mat faceImage;
    std::vector<uchar> decode;

    decode.resize(data.size());
    memcpy(decode.data(),data.data(),data.size());
    //字节流数据转换成Mat格式
    faceImage=cv::imdecode(decode,cv::IMREAD_COLOR);
 	emit query(faceImage);
}

```

**11.识别人脸id查询个人信息**

通过人脸id得到员工信息，读取工号，姓名，部门，时间。

attendserverwin.h-发送数据

```c++
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>

QSqlTableModel model;
```

attendserverwin.cpp

主函数：

```
//给sql模型绑定表格
    model.setTable("employee");
```

```c++
void AttendServerWin::recv_faceid(int64_t faceid)
{
    //    qDebug()<<faceid;
        //从数据库中c查询faceid对应个人信息
        //给模型设置过滤器
        qDebug()<<"识别到人脸id:"<<faceid;
        if(faceid<0)
        {
            QString sdms=QString("{\"employeeID\":\" \",\"name\":\"\",\"department\"\"\":,\"time\":\"\"}");
            msocket->write(sdms.toUtf8());//把打包好的数据发送到客户端
            return;
        }
        model.setFilter(QString("faceID=%1").arg(faceid));
        //查询
        model.select();
        //判断是否查询到数据
        if(model.rowCount()==1)
        {
            //工号，姓名，部门，时间
            QSqlRecord record=model.record(0);
            QString sdms=QString("{\"employeeID\":\"%1\",\"name\":\"%2\",\"department\":\"软件\",\"time\":\"%3\"}")
                    .arg(record.value("employeeID").toString())
                    .arg(record.value("name").toString())
                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            qDebug()<<sdms;
            //把数据写入数据库---考勤表
            QString insertSql = QString("insert into attendance(employeeID) values('%1')").arg(record.value("employeeID").toString());
            QSqlQuery query;
            if(!query.exec(insertSql))
            {
                QString sdms = QString("{\"employeeID\":\" \",\"name\":\"\",\"department\":\"\",\"time\":\"\"}");
                msocket->write(sdms.toUtf8());//把打包好的数据发送给客户端
                qDebug()<<query.lastError().text();
                return ;
            }else
            {
                msocket->write(sdms.toUtf8());//把打包好的数据发送给客户端
            }
        }
}
```

faceattend.h，接收数据

```c++
private slots:
    void recv_data();
```

faceattend.cpp，接收数据

```c++
void FaceAttend::recv_data()
{
    //先接收，显示一下
   QString msg=msocket.readAll();
   qDebug()<<msg;
   ui->lineEdit->setText(msg);

 }
    
```

