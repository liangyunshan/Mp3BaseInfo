#include "mp3infowidget.h"
#include "ui_mp3infowidget.h"

#include <QDir>
#include <QTime>
#include <QString>
#include <QFileDialog>
#include <QDebug>

#include "tablemodel.h"
#include "json/json.h"
#include "ID3Prase/mw_id3_get.h"

Mp3InfoWidget::Mp3InfoWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Mp3InfoWidget)
{
    ui->setupUi(this);

    createAlbumFolder();
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

/*!
 * @brief 解析mp3文件的id3信息，并获取时长与专辑封面
 * @param filePath mp3文件的实际路径
 */
void Mp3InfoWidget::praseID3ForMp3File(const QString &filePath)
{
    Json::Value id3Info;

    readID3InfoOfMp3File(id3Info,filePath.toStdString().c_str()); //调用封装了id3lib提供的api的接口获取id3信息
    int duration_seconds = getDurationOfMp3File(filePath.toStdString().c_str());    //调用封装的接口获取mp3时长
    int mins = duration_seconds/60;
    int secs = duration_seconds%60;
    QTime mp3Duration(0,mins,secs);

    id3Info["Path"] = filePath.toLocal8Bit().data();
    id3Info["Duration"] = mp3Duration.toString("mm:ss").toStdString().c_str();

    if(!albumPath.isEmpty())
    {
        std::string fileName = GetPathOrURLShortName(filePath.toStdString());
        fileName = fileName.replace(fileName.length()-3,3,"jpeg");

        std::string coverPath = albumPath.toStdString()+ "/" + fileName;
        bool extratResult = extractAlbumImgofMp3File(filePath.toStdString().c_str(),coverPath.c_str());     //获取mp3的专辑封面
        if(extratResult)
        {
            id3Info["AlbumPath"] = coverPath.c_str();
        }
    }

    dataModel->appendData(id3Info);
}

/*!
 *@brief 创建用于存放mp3专辑封面图片的文件家夹
 *@return 文件夹创建结果或者是否存在
 */
bool Mp3InfoWidget::createAlbumFolder()
{
    QString tempAlbumPath = QDir::currentPath() + "/"+QString("Albums");
    QDir tmpDir(tempAlbumPath);
    if(!tmpDir.exists())
    {
       bool result =  tmpDir.mkpath(tempAlbumPath);
       if(result)
       {
            albumPath = tempAlbumPath;
       }
       else
       {
           albumPath = "";
       }
       return result;
    }
    else
    {
        albumPath = tempAlbumPath;
        return true;
    }

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
