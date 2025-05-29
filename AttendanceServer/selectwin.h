#ifndef SELECTWIN_H
#define SELECTWIN_H

#include <QWidget>
#include <QSqlTableModel>
namespace Ui {
class selectwin;
}

class selectwin : public QWidget
{
    Q_OBJECT

public:
    explicit selectwin(QWidget *parent = nullptr);
    ~selectwin();

private slots:
    void on_selectBt_clicked();
private:
    Ui::selectwin *ui;
    QSqlTableModel *model;
};

#endif // SELECTWIN_H
