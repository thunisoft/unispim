#include "mywordlibtableview.h"
#include "wordlibtableitemwidget.h"
#include <QHeaderView>
#include <QPushButton>
#include <QLineEdit>
#include <QPainter>
#include <QSharedPointer>
#include "myutils.h"
#include <cmath>



static void dfsOpenEditor(QTableView *table_view, QAbstractItemModel *model, const QModelIndex &root = QModelIndex())
{
    if (root.isValid())
        table_view->openPersistentEditor(root);
    int row_count = model->rowCount();
    int column_count = model->columnCount();
    for (int index = 0; index < row_count; ++index)
    {
        for(int column_index=0; column_index< column_count; ++column_index)
        {
            auto idx = model->index(index, column_index);
            if(idx.isValid())
            {
                table_view->openPersistentEditor(idx);
            }
        }

    }
}

MyTableView::MyTableView(QWidget *parent) : QTableView(parent)
{
    //setSelectionMode(QAbstractItemView::ExtendedSelection);
    setDropIndicatorShown(true);
    //隐藏行宽和列高
    horizontalHeader()->setVisible(false);
    verticalHeader()->setVisible(false);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setShowGrid(false);
    this->setEditTriggers(QAbstractItemView::EditTrigger::AllEditTriggers);
    this->setMouseTracking(true);
    connect(this, &MyTableView::entered, this, &MyTableView::openPersistentEditor);

}

MyTableView::~MyTableView() {}


void MyTableView::setModel(QAbstractItemModel* model)
{
    QTableView::setModel(model);
    QObject::connect(model, &QAbstractTableModel::modelReset, this, &MyTableView::openAllEditor);
    QObject::connect(model, &QAbstractTableModel::rowsInserted, this, &MyTableView::openAllEditor);
    openAllEditor();
}

void MyTableView::mousePressEvent(QMouseEvent *event)
{
    Super::mousePressEvent(event);
}



void MyTableView::openAllEditor()
{
    dfsOpenEditor(this, model());
    update();
}


Delegate::Delegate(QTableView *table_view, QObject *parent) : QStyledItemDelegate(parent)
{
    m_table_view = table_view;
}

Delegate::Delegate(QObject *parent):QStyledItemDelegate(parent)
{

}

Delegate::~Delegate()
{
}

void Delegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(index);
    //QStyledItemDelegate::paint(painter, option, index);
}

QSize Delegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
   return WordlibTableItemWidget().size();
}

QWidget *Delegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    int row_index = index.row();
    int column_index = index.column();
    row_index;column_index;
    TAB_WORDLIB_INFO* item_data = (TAB_WORDLIB_INFO*)(index.internalPointer());

    QString obj_name = m_table_view->objectName();

    if(item_data->wordlib_name.isEmpty() && obj_name.compare("user_table_view") == 0)
    {
        AddUserWordlibTableItem* add_btn_item = new AddUserWordlibTableItem(parent);
        connect(add_btn_item,&AddUserWordlibTableItem::import_user_wordlib_btn_clicked,this,&Delegate::import_user_wordlib_btn_clicked);
        return add_btn_item;
    }

     WordlibTableItemWidget* item = new WordlibTableItemWidget(parent);

     item->set_wordlib_info(item_data);

     connect(item,&WordlibTableItemWidget::wordlib_removed,this,&Delegate::wordlib_removed);
     connect(item,&WordlibTableItemWidget::wordlib_download,this,&Delegate::wordlid_download);
     connect(item,&WordlibTableItemWidget::wordlib_state_changed,this,&Delegate::wordlib_state_changed);
     return item;
}

void Delegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    Q_UNUSED(editor);
    Q_UNUSED(index);
}

void Delegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    Q_UNUSED(editor);
    Q_UNUSED(model);
    Q_UNUSED(index);
}

void Delegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
   static_cast<WordlibTableItemWidget*>(editor)->setGeometry(option.rect);
}


MyTableModel::MyTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

MyTableModel::~MyTableModel()
{
}


Qt::ItemFlags MyTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

int MyTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        int vector_size = m_wordlib_info_vector.size();
        return std::ceil(vector_size/3.0);
    }
}

int MyTableModel::columnCount(const QModelIndex& parent) const
{
    //固定显示三行
    Q_UNUSED(parent);
    return 3;
}

QModelIndex MyTableModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);

     if(row<0 || column<0 )
     {
         return QModelIndex();
     }
     int item_index = row*3 + column;
     if(m_wordlib_info_vector.size() > item_index)
     {
         return createIndex(row, column, m_wordlib_info_vector.at(item_index).data());
     }
     else
     {
         return QModelIndex();
     }
}

bool MyTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(value);
    Q_UNUSED(role);
    if (!index.isValid())
        return false;

    dataChanged(index,index);
    return true;
}

QVariant MyTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    return QVariant();
}

bool MyTableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    Q_UNUSED(section);
    Q_UNUSED(value);
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    return true;
}

QVariant MyTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();
     int row = index.row();
     int column = index.column();
     int item_index = row*3 + column;

     return (m_wordlib_info_vector.size() > item_index)? QVariant(m_wordlib_info_vector.at(item_index)->wordlib_name):QVariant();
}

void MyTableModel::set_data(const data_container& data)
try
{
    beginResetModel();  //刷新表格
    m_wordlib_info_vector.clear();
    m_wordlib_info_vector = data;
    endResetModel();
}
catch(std::exception& e)
{
   TOOLS::LogUtils::write_log_to_file(QString("when set data throw exception:%1").arg(e.what()));
}
