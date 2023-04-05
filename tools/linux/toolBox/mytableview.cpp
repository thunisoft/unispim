#include "mytableview.h"
#include <QHeaderView>
#include <QPushButton>
#include <QLineEdit>
#include <QTimer>
#include <QPainter>
#include <QSharedPointer>
#include "toolbtn.h"
#include "utils.h"


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
    setDropIndicatorShown(true);
    //隐藏行宽和列高
    horizontalHeader()->setVisible(false);
    verticalHeader()->setVisible(false);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setShowGrid(false);
    setAutoScroll(false);
    this->setEditTriggers(QAbstractItemView::EditTrigger::AllEditTriggers);
    this->setMouseTracking(true);
    resizeRowsToContents();
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

    QStyleOptionViewItem itemOption(option);
    if(itemOption.state&QStyle::State_HasFocus)
    {
        itemOption.state=itemOption.state^QStyle::State_HasFocus;
    }
    QStyledItemDelegate::paint(painter,itemOption,index);
}

QSize Delegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return ToolBtn().sizeHint();
}

void Delegate::destroyEditor(QWidget *editor, const QModelIndex &index) const
{
    if(editor == NULL || !index.isValid())
    {
        return;
    }

    if(dynamic_cast<ToolBtn*>(editor) != 0)
    {
        delete dynamic_cast<ToolBtn*>(editor);
    }
}

QWidget *Delegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    if(!index.isValid())
    {
        return NULL;
    }
    int row_index = index.row();
    int column_index = index.column();
    row_index;column_index;
    TOOL_ADDON_INFO* item_data = (TOOL_ADDON_INFO*)(index.internalPointer());
    if(item_data->addon_id.isEmpty())
    {
        //通过固定的ID来确定最后一个添加图片的按钮
        return NULL;
    }
    if(item_data->addon_status == "deprecated")
    {
        return NULL;
    }

    ToolBtn* item = new ToolBtn(parent);
    item->set_tool_info(*item_data);
    item->setProperty("type","toolbtn");
    connect(item,SIGNAL(tool_has_update()),this,SIGNAL(toolbox_hasupdate()));
    connect(item,SIGNAL(addon_removed(QString)),this,SIGNAL(addon_removed(QString)));
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
    static_cast<ToolBtn*>(editor)->setGeometry(option.rect);
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
        //动态行数在windows7下 resetmodel 的时候会崩溃
        //这里设置成默认三行,在一定事件段内够用了
        int vector_size = m_addon_vector.size();
        //return ceil(vector_size/4);
        return 3;
    }
}

int MyTableModel::columnCount(const QModelIndex& parent) const
{
    //固定显示四列
    Q_UNUSED(parent);
    return 4;
}

QModelIndex MyTableModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    if(row<0 || column<0 )
    {
        return QModelIndex();
    }
    int item_index = row*4 + column;
    if(m_addon_vector.size() > item_index)
    {
        return createIndex(row, column, m_addon_vector.at(item_index).data());
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
     int item_index = row*4 + column;

     return (m_addon_vector.size() > item_index)? QVariant(m_addon_vector.at(item_index)->addon_displayname):QVariant();
}

void MyTableModel::set_data(const data_container& data)
try
{
#ifdef _WIN32
    //beginResetModel();
    m_addon_vector.clear();
    m_addon_vector = data;
    //endResetModel();
#else
    beginResetModel();
    m_addon_vector.clear();
    m_addon_vector = data;
    endResetModel();
#endif
}
catch(std::exception& e)
{
   Utils::write_log_tofile(QString("when set data throw exception:%1").arg(e.what()));
}

void MyTableModel::refresh_data(const data_container& data)
try
{
#ifdef _WIN32
    beginResetModel();
    m_addon_vector.clear();
    m_addon_vector = data;
    endResetModel();
#else
    beginResetModel();
    m_addon_vector.clear();
    m_addon_vector = data;
    endResetModel();
#endif
}
catch(std::exception& e)
{
   Utils::write_log_tofile(QString("when set data throw exception:%1").arg(e.what()));
}
