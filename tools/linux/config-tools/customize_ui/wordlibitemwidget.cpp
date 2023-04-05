#include "wordlibitemwidget.h"
#include "ui_wordlibitemwidget.h"
#include <QFontMetrics>

WordlibItemWidget::WordlibItemWidget(QWidget *parent) :
    CustomizeQWidget(parent),
    ui(new Ui::WordlibItemWidget)
{
    ui->setupUi(this);
    InitWidget();
}
WordlibItemWidget::WordlibItemWidget(QString itemName,QWidget*parent):
    CustomizeQWidget(parent),
    m_itemName(itemName),
    ui(new Ui::WordlibItemWidget)
{
    ui->setupUi(this);
    InitWidget();
}

WordlibItemWidget::~WordlibItemWidget()
{
    delete ui;
}


void WordlibItemWidget::InitWidget()
{
    ui->innerWidget->setProperty("type","wordlibitem");
    ui->label->setProperty("type","bluelabel");
    ui->closebtn->setProperty("type","blueclose");
    ui->label->setText(m_itemName);
    ui->label->setToolTip(m_itemName);
    connect(ui->closebtn, SIGNAL(clicked()),this,SLOT(SlotDeleteItem()));

}

void WordlibItemWidget::RefreshTextShow()
{
    QFontMetrics fontWidth(ui->label->font());

    int showWidth  = 75;
    QString elideNote = fontWidth.elidedText(m_itemName, Qt::ElideRight, showWidth);//最大宽度150像素

    ui->label->setText(elideNote);//显示省略好的字符串
    ui->label->setToolTip(m_itemName);//设置tooltips
}

void WordlibItemWidget::SlotDeleteItem()
{
    emit ItemDelete(m_itemName);
}
