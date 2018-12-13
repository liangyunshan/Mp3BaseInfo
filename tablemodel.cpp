#include "tablemodel.h"

TableModel::TableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

TableModel::~TableModel()
{

}

int TableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return datas.count();
}

int TableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 5;
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if( role == Qt::DisplayRole)
    {
        Json::Value musicInfo = datas.at(index.row());
        switch (index.column()) {
            case PATH:
                return musicInfo.get("Path",Json::Value("****")).asCString();

            case TITLE:
                return musicInfo.get("TIT2",Json::Value("****")).asCString();

            case ARTIST:
                return musicInfo.get("TPE1",Json::Value("****")).asCString();

            case ALBUM:
                return musicInfo.get("TALB",Json::Value("****")).asCString();

            case DURATION:
                return musicInfo.get("Duration",Json::Value("****")).asCString();

            default:
                return QVariant();
        }
    }
    else if(role == Qt::TextAlignmentRole)
    {
        switch (index.column()) {
        case TITLE:
        case ARTIST:
        case DURATION:
            return Qt::AlignCenter;
            break;
        default:
            break;
        }
    }
    else if(role == Qt::ToolTipRole)
    {
        return data(index,Qt::DisplayRole);
    }
    else if(role ==  Qt::DecorationRole)
    {
        if(index.column() == ALBUM)
        {
            return QIcon(":/album/img/DefaultAlbum.png");
        }
    }

    return QVariant();
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
            case PATH:
                return tr("Path");

            case TITLE:
                return tr("Title");

            case ARTIST:
                return tr("Signer");

            case ALBUM:
                return tr("Album");

            case DURATION:
                return tr("Duration");

            default:
                return QVariant();
        }
    }
    return QVariant();
}

Qt::ItemFlags TableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractTableModel::flags(index) |= Qt::ItemIsSelectable;
}

void TableModel::setTableView(QTableView *view)
{
    if(!view)
        return;
//    view->horizontalHeader()->setSectionResizeMode(PATH,QHeaderView::Interactive);
//    view->horizontalHeader()->setSectionResizeMode(TITLE,QHeaderView::ResizeToContents);
//    view->horizontalHeader()->setSectionResizeMode(ARTIST,QHeaderView::Interactive);
//    view->horizontalHeader()->setSectionResizeMode(ALBUM,QHeaderView::ResizeToContents);
//    view->horizontalHeader()->setSectionResizeMode(DURATION,QHeaderView::Fixed);

     view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    view->verticalHeader()->setVisible(false);


}

void TableModel::appendData(Json::Value data)
{
    beginResetModel();
    datas.append(data);
    endResetModel();
}
