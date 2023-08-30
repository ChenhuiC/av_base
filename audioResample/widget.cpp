#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include "arecord.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    recThread = new RecThread(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_btnStart_clicked()
{
    recStatue = !recStatue;
    if(recStatue){
        ui->btnStart->setText("停止录制");
        /* 创建录制线程 */
        recThread->start();
    }else{
        ui->btnStart->setText("开始录制");
        set_status(REC_STOP);
    }
//    rec_audio();
}

RecThread::RecThread(QObject *parent)
{

}

void RecThread::run()
{
    rec_audio();
}
