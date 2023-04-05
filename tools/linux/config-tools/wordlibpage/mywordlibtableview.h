#ifndef MYWORDLIBTABLEVIEW_H
#define MYWORDLIBTABLEVIEW_H

#include <QTableView>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QAbstractItemModel>
#include <QObject>
#include <QSharedPointer>
#include <QItemDelegate>
#include "adduserwordlibtableitem.h"

enum WORDLIB_STATE
{
    CHECKED,
    UN_CHECKED,
    CLOUD_WORDLIB
};

struct TAB_WORDLIB_INFO
{
    QString wordlib_name;
    int wordlib_item_count;
    QString autor_name;
    QString file_path;
    QString file_name;
    QString update_time;
    WORDLIB_STATE current_state;
    bool can_be_edit;
    bool is_item_selected_in_ui;
    QString download_url;
    bool is_official_wordlib;
    TAB_WORDLIB_INFO()
    {
        wordlib_name = "";
        wordlib_item_count = 0;
        autor_name = "";
        file_path = "";
        file_name = "";
        update_time = "";
        current_state= CLOUD_WORDLIB;
        can_be_edit = false;
        is_item_selected_in_ui = false;
        download_url = "";
        is_official_wordlib = false;
    }
};

class MyTableView : public QTableView
{
    Q_OBJECT
public:
    using Super = QTableView;
    MyTableView(QWidget *parent = nullptr);
    ~MyTableView();
    void setModel(QAbstractItemModel *model) override;

public slots:
    //递归打开每一个Item的编辑控件
    void openAllEditor();

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

class MyTableModel : public QAbstractTableModel
{
    Q_OBJECT

  using data_container =    QVector<QSharedPointer<TAB_WORDLIB_INFO>>;
  using wordlib_info_ptr = QSharedPointer<TAB_WORDLIB_INFO>;
public:
    MyTableModel(QObject* parent = nullptr);
    ~MyTableModel();

public:
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    void set_data(const data_container& data);   //设置表格数据
public:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;  //返回行数
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;   //返回列数
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    //父节点parent的row行column列的数据
    QModelIndex index(int row, int column,
                          const QModelIndex &parent = QModelIndex()) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    //设置标题头
    bool setHeaderData(int section, Qt::Orientation orientation,
                          const QVariant &value, int role = Qt::EditRole) override;

private:
    data_container m_wordlib_info_vector;

};

class Delegate  : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit Delegate(QTableView *table_view, QObject *parent = nullptr);
    explicit Delegate(QObject *parent = nullptr);
    ~Delegate();
    //绘制事件
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    //获取委托控件的尺寸
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    //创建编辑控件
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    //设置编辑控件的数据
protected:
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    //设置模型的数据
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    //刷新尺寸
    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

signals:
    void wordlib_removed(QString file_path);
    void wordlid_download(QString worlib_name);
    void wordlib_state_changed();
    void import_user_wordlib_btn_clicked(bool click_state);

private:
    QTableView * m_table_view; //对应的视图
    QModelIndex m_mouseModelIdx;

};


#endif // MYWORDLIBTABLEVIEW_H
