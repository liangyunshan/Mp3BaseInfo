#include "mp3infowidget.h"
#include "ui_mp3infowidget.h"

#include <QTime>
#include <QString>
#include <QFileDialog>
#include <QDebug>

#include "tablemodel.h"
//#include "getmp3duration.h"
#include "ID3Prase/mw_id3_get.h"

Mp3InfoWidget::Mp3InfoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Mp3InfoWidget)
{
    ui->setupUi(this);
    ui->filePath->setReadOnly(true);
    dataModel = new TableModel(ui->ID3InfoTable);
    ui->ID3InfoTable->setModel(dataModel);
    dataModel->setTableView(ui->ID3InfoTable);

    connect(ui->chooseBtn,SIGNAL(clicked(bool)),this,SLOT(chooseTargetMp3File(bool)));
}

Mp3InfoWidget::~Mp3InfoWidget()
{
    delete ui;
}

void Mp3InfoWidget::praseID3ForMp3File(const QString &filePath)
{
    Json::Value id3Info;

    readID3InfoOfMp3File(id3Info,filePath.toStdString().c_str());
    int duration_seconds = getDurationOfMp3File(filePath.toStdString().c_str());
    int mins = duration_seconds/60;
    int secs = duration_seconds%60;
    QTime mp3Duration(0,mins,secs);

    id3Info["Path"] = filePath.toLocal8Bit().data();
    id3Info["Duration"] = mp3Duration.toString("mm:ss").toStdString().c_str();

    std::string fileName = GetPathOrURLShortName(filePath.toStdString());
    fileName = fileName.replace(fileName.length()-3,3,"jpeg");

    std::string coverPath = std::string("/Albums") + "/" + fileName;

    bool extratResult = extractAlbumImgofMp3File(filePath.toStdString().c_str(),coverPath.c_str());

    dataModel->appendData(id3Info);

//    qDebug()<<"id3Info"<<id3Info.toStyledString().c_str();
}

void Mp3InfoWidget::chooseTargetMp3File(bool)
{
    QString fileName = QFileDialog::getOpenFileName(this,"ChooseMp3","","*.mp3");
    if(!fileName.isEmpty())
    {
        ui->filePath->setText(fileName);
        praseID3ForMp3File(fileName);
    }
}
