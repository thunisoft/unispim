/*********************************************************************
* @file  mytableview.h
* @brief      插件列表的控件
* @details    自定义实现插件列表控件
* @date        2022-2-24
* @version     V1.0
* @copyright    Copyright (c) 2018-2022  thunisoft
*********************************************************************
*/
#ifndef MY_TABLE_VIEW_H
#define MY_TABLE_VIEW_H

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
#include "commondef.h"

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

  using data_container =    QVector<QSharedPointer<TOOL_ADDON_INFO>>;
public:
    MyTableModel(QObject* parent = nullptr);
    ~MyTableModel();

public:
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    void set_data(const data_container& data);   //设置表格数据
    void refresh_data(const data_container& data);

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
    data_container m_addon_vector;

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
    virtual void destroyEditor(QWidget *editor, const QModelIndex &index) const override;
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
    void toolbox_hasupdate();
    void addon_removed(QString addon_name);

private:
    QTableView * m_table_view; //对应的视图
};


#endif // MYWORDLIBTABLEVIEW_H
