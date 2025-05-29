#ifndef ATTENDSERVERWIN_H
#define ATTENDSERVERWIN_H

#include "qfaceobject.h"
#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QDateTime>
QT_BEGIN_NAMESPACE
namespace Ui { class AttendServerWin; }
QT_END_NAMESPACE

class AttendServerWin : public QMainWindow
{
    Q_OBJECT

public:
    AttendServerWin(QWidget *parent = nullptr);
    ~AttendServerWin();
signals:
    void query(cv::Mat& image);
protected slots:
    void read_data();
private slots:
    void accept_client();
    void recv_faceid(int64_t faceid);
private:
    Ui::AttendServerWin *ui;
    QTcpServer mserver;
    QTcpSocket *msocket;
    quint64 bsize;

    QFaceObject fobj;
    QSqlTableModel model;
};
#endif // ATTENDSERVERWIN_H
