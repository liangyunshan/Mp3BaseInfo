#include "widget.h"
#include "ui_widget.h"

#include <QFileDialog>
#include <QTime>
#include <QDebug>

#include "getmp3duration.h"
#include "ID3Prase/mw_id3_get.h"
#include "json/json.h"
#include "tablemodel.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    connect(ui->choose,SIGNAL(clicked(bool)),this,SLOT(chooseMp3File(bool)));

//    model = new TableModel(this);
//    ui->musicList->setModel(model);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::chooseMp3File(bool)
{
    QString mp3Path = QFileDialog::getOpenFileName(this);
    ui->mp3Path->clear();
    ui->mp3Path->setText(mp3Path);
    Json::Value id3Value;
    readID3InfoOfMp3File(id3Value,mp3Path.toStdString().c_str());
    int duration_seconds = getDurationOfMp3File(mp3Path.toStdString().c_str());
    int mins = duration_seconds/60;
    int secs = duration_seconds%60;
    QTime mp3Duration(0,mins,secs);


    qDebug()<<"id3Value"<<mp3Duration.toString("hh:mm:ss");

}
