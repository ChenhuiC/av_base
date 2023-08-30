#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QThread>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class RecThread : public QThread
{
    Q_OBJECT
public:
    explicit RecThread(QObject *parent = nullptr);

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
    bool recStatue = false;
    RecThread *recThread;
private slots:

    void on_btnStart_clicked();

private:
    Ui::Widget *ui;
};


#endif // WIDGET_H
