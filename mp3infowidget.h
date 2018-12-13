#ifndef MP3INFOWIDGET_H
#define MP3INFOWIDGET_H

#include <QWidget>

class TableModel;

namespace Ui {
class Mp3InfoWidget;
}

class Mp3InfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit Mp3InfoWidget(QWidget *parent = 0);
    ~Mp3InfoWidget();

private:
    void praseID3ForMp3File(const QString &filePath);

private slots:
    void chooseTargetMp3File(bool);

private:
    Ui::Mp3InfoWidget *ui;
    TableModel *dataModel;
};

#endif // MP3INFOWIDGET_H
