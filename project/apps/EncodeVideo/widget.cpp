#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include "rec_video.h"

RecThread::RecThread(/* args */)
{
}

RecThread::~RecThread()
{
}

void RecThread::run()
{
    qDebug() << "current thread ID: " << QThread::currentThreadId();
    rec_video();

}



Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    recThread = new RecThread();

}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_btnStart_clicked()
{
    recStatue = !recStatue;
    if(recStatue){
        ui->btnStart->setText("Stop");
        set_status(REC_START);
        recThread->start();
    }else{
        ui->btnStart->setText("Start");
        set_status(REC_STOP);
    }
}
