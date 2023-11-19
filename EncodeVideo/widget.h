#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QThread>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class RecThread : public QThread
{
private:
    /* data */
public:
    RecThread(/* args */);
    ~RecThread();
protected:
    void run();

};



class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
    bool recStatue = false;
    RecThread *recThread;

private slots:
    void on_btnStart_clicked();

};
#endif // WIDGET_H
