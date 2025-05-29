#ifndef REGISTERWIN_H
#define REGISTERWIN_H

#include <QWidget>
#include <opencv.hpp>
namespace Ui {
class Registerwin;
}

class Registerwin : public QWidget
{
    Q_OBJECT

public:
    explicit Registerwin(QWidget *parent = nullptr);
    ~Registerwin();
    void timerEvent(QTimerEvent *e);

private slots:
    void on_resetBt_clicked();

    void on_addpicBt_clicked();

    void on_registerBt_clicked();

    void on_videoBt_clicked();

    void on_cameraBt_clicked();

private:
    Ui::Registerwin *ui;
    int timerid;
    cv::VideoCapture cap;
    cv::Mat image;
};

#endif // REGISTERWIN_H
