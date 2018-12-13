#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QTableView>
#include <QHeaderView>

#include "json/json.h"

//! [0]
class TableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum MUSICARGUMENT
    {
        PATH,
        TITLE,
        ARTIST,
        ALBUM,
        DURATION
    };
    TableModel(QObject *parent = 0);
    ~TableModel();

    int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void setTableView(QTableView *view);
    void appendData(Json::Value data);

private:
    QList<Json::Value> datas;
};
//! [0]

#endif // TABLEMODEL_H
