#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include "tablemodel.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void chooseMp3File(bool);

private:
    Ui::Widget *ui;

//    TableModel *model;
};

#endif // WIDGET_H
