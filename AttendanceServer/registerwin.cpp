
#include "registerwin.h"
#include "ui_registerwin.h"
#include <QFileDialog>
#include <qfaceobject.h>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QMessageBox>
#include <QDebug>
Registerwin::Registerwin(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Registerwin)
{
    ui->setupUi(this);
}

Registerwin::~Registerwin()
{
    delete ui;
}

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


