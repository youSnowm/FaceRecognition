#include "attendserverwin.h"
#include "selectwin.h"
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <opencv.hpp>
#include "registerwin.h"
#include <QMessageBox>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //信号处理的时候要定义数据
    qRegisterMetaType<cv::Mat>("cv::Mat&");
    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<int64_t>("int64_t");
//    Registerwin ww;
//    ww.show();
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

    //考勤表格,autoincrement自动递增
        createsql = "create table if not exists attendance(attendaceID integer primary key autoincrement, employeeID integer,"
                    "attendaceTime TimeStamp NOT NULL DEFAULT(datetime('now','localtime')))";
        if(!query.exec(createsql))
        {
            qDebug()<<query.lastError().text();
            return -1;
        }

    AttendServerWin w;
    w.show();
    return a.exec();
}
