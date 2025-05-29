//搭建服务器端
//接着实现客户端将人脸数据打包发送到服务器端
#include "attendserverwin.h"
#include "ui_attendserverwin.h"
#include <QThread>
#include <opencv.hpp>
#include <QSqlQuery>
#include <QSqlError>

AttendServerWin::AttendServerWin(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AttendServerWin)
{
    ui->setupUi(this);

    //qtcpServer当有客户端连接会发送newConnection
    connect(&mserver,&QTcpServer::newConnection,this,&AttendServerWin::accept_client);
    mserver.listen(QHostAddress::Any,9999);//启动服务器
    bsize=0;

    //给sql模型绑定表格
    model.setTable("employee");

    //创建线程
    QThread *thread=new QThread();
    //把QFaceObject对象移动到thread线程z中执行
    fobj.moveToThread(thread);
    //启动线程，通过信号出发线程
    thread->start();
    connect(this,&AttendServerWin::query,&fobj,&QFaceObject::face_query);
    //关联QFaceObject对象里面的send_face对象
    connect(&fobj,&QFaceObject::send_faceid,this,&AttendServerWin::recv_faceid);
}

AttendServerWin::~AttendServerWin()
{
    delete ui;
}

//接收客户端连接
void AttendServerWin::accept_client()
{

    //获取与客户端通信的套接字
    msocket = mserver.nextPendingConnection();

    //当客户端有数据到达会发送readlyRead信号
    connect(msocket,&QTcpSocket::readyRead,this,&AttendServerWin::read_data);
}

//读取客户端发送的数据
void AttendServerWin::read_data()
{
    QDataStream stream(msocket);//将套接字绑定到数据表
    stream.setVersion(QDataStream::Qt_5_14);
    //接收端按照相同顺序读取数据。首先读取 bsize（即发送端写入的第一个整数
    if(bsize==0)
    {
        if(msocket->bytesAvailable()<(qint64)sizeof(bsize)) return;
        //采集数据长度
        stream>>bsize;
    }
    if(msocket->bytesAvailable() < bsize)//说明数据还没发送完成,返回继续等待
    {
        return;
    }

    QByteArray data;
    stream>>data;
    bsize=0;
    if(data.size()==0)//没有读取到数据
    {
        return;
    }

    //显示图片
    QPixmap map;
    map.loadFromData(data,"jpg");
    map=map.scaled(ui->pictureLb->size());
    ui->pictureLb->setPixmap(map);

    //识别人脸,接收数据，解码imdecode
    cv::Mat faceImage;
    std::vector<uchar> decode;
    decode.resize(data.size());
    memcpy(decode.data(),data.data(),data.size());
    faceImage=cv::imdecode(decode,cv::IMREAD_COLOR);
 // int faceid=fobj.face_query(faceImage);//消耗资源较多
    emit query(faceImage);

}
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
        //setFilter过滤对象
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
