#include "toviewphrasedialog.h"
#include "ui_toviewphrasedialog.h"
#include "phraseoptionwidget.h"
#include "addoreditphrasedialog.h"
#include "utils.h"
#include <QDir>
#include <QListView>
#include <QTimer>
#include <QTextStream>
#include "informmsgbox.h"
#include "fileviewerdialog.h"
#include "config.h"
#include "configbus.h"
#include "phrasemodel.h"
#include "editphrasefiledialog.h"
#include "addoreditphrasedialog.h"
#include "dpiadaptor.h"
#include "customize_ui/customizephrasebtn.h"

#include <QStyleOption>
#include <QPainter>
#include <QBrush>


ToViewPhraseDialog::ToViewPhraseDialog(QWidget *parent) :   
    QDialog(parent),
    m_phrase_modified(false),
    m_move_widget_flag(false),
    ui(new Ui::ToViewPhraseDialog)
{
    Init();
}

ToViewPhraseDialog::~ToViewPhraseDialog()
{
    delete ui;
}

void ToViewPhraseDialog::Init()
{
    this->setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::Dialog);
    m_addorEditDlg = new AddOrEditPhraseDialog(ADD_PHRASE_DIALOG, -1, NULL, this);
    connect(m_addorEditDlg, SIGNAL(SignalToSavePhrase(PHRASE, int)), this, SLOT(OnPhraseAddedOrChanged(PHRASE, int)));
    m_editPhraseFileDlg = new EditPhraseFileDialog(this);
    m_fileViewDlg = new FileViewerDialog(QString::fromLocal8Bit("系统短语"), Utils::GetSysPhrasePath(), false, this);

    QTimer::singleShot(200,this,SLOT(UpdateComboBoxStyleSheet()));

    SetUpCustomizeUI();

    m_tempPhrase.clear();
    PhraseModel::Instance()->LoadPhraseFromFile();
    m_tempPhrase = PhraseModel::Instance()->GetParsedPhrase();
    FillDataIntoTable(m_tempPhrase);
    RegisterSlots();
}

void ToViewPhraseDialog::SetUpCustomizeUI()
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_TranslucentBackground,true);
    this->setProperty("type","borderwidget");

    QList<QPushButton*> btn_list;
    btn_list << ui->add_push_button << ui->edit_phrase_push_button <<
                                    ui->to_view_sys_phrase_button << ui->apply_push_button
                                 << ui->cancel_push_button;
    for(QPushButton* btn : btn_list)
    {
        btn->setProperty("type","normal");
        btn->setCursor(Qt::PointingHandCursor);
    }


    ui->table_widget->setProperty("type","normal");
    ui->table_widget->setMouseTracking(true);
    ui->phrase_show_eara->setMouseTracking(true);
    this->setMouseTracking(true);
    ui->widget->setProperty("type","window_title_widget");
    ui->default_pos_label->setProperty("type","h2");


    ui->comboBox->setView(new QListView());
    ui->comboBox->setAutoFillBackground(true);
    ui->comboBox->setMaxVisibleItems(6);


    m_btnGroupEdit = new Customizephrasebtn("编辑", this);
    connect(m_btnGroupEdit, SIGNAL(btnclicked()), this, SLOT(SlotEditPrase()));
    m_btnGroupEdit->hide();

    m_btnGroupDel = new Customizephrasebtn("删除", this);
    connect(m_btnGroupDel, SIGNAL(btnclicked()), this, SLOT(SlotDelPrase()));
    m_btnGroupDel->hide();

    QStringList tableHeaderName;
    tableHeaderName << "缩写" << "内容" << "位置";

    ui->table_widget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->table_widget->verticalHeader()->setHidden(true);
    ui->table_widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->table_widget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->table_widget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->table_widget->horizontalHeader()->setStretchLastSection(true);
    ui->table_widget->resizeColumnToContents(3);

    ui->table_widget->setShowGrid(false);
    ui->table_widget->setFocusPolicy(Qt::NoFocus);

    //ui->table_widget->clear();
    ui->table_widget->setColumnCount(3); //缩写 内容 位置
    ui->table_widget->setHorizontalHeaderLabels(tableHeaderName);
    ui->table_widget->setFocus();

    ConfigItemStruct configInfo;
    if(Config::Instance()->GetConfigItemByJson("use_sys_phrase",configInfo))
    {
        configInfo.itemCurrentIntValue ? ui->open_phrase_check_box->setChecked(true):ui->open_phrase_check_box->setChecked(false);
    }
    else
    {
        ui->open_phrase_check_box->setChecked(true);
    }
    ui->open_phrase_check_box->setVisible(false);

    SetDefaultPhrasePosition();
    connect(ui->table_widget,SIGNAL(hoverIndexChanged(QPoint)),this, SLOT(SlotRowIndexChanged(QPoint)));
    connect(ui->table_widget, SIGNAL(itemPressed(QTableWidgetItem*)), this, SLOT(SlotTableWidgetItemPressed(QTableWidgetItem*)));
    connect(ui->table_widget, SIGNAL(loseFocus()), this, SLOT(SlotTableWidgetLoseFocus()));
}

void ToViewPhraseDialog::RegisterSlots()
{
    connect(ui->add_push_button, SIGNAL(clicked()), this, SLOT(OnAddPhraseButtonClicked()));
    connect(ui->edit_phrase_push_button, SIGNAL(clicked(bool)), this, SLOT(OnEditPhraseFileButtonClicked()));
    connect(ui->to_view_sys_phrase_button, SIGNAL(clicked(bool)), this, SLOT(OnViewSysPhraseButtonClicked()));
    connect(ui->apply_push_button, SIGNAL(clicked(bool)), this, SLOT(OnApplyButtonClicked()));
    connect(ui->cancel_push_button, SIGNAL(clicked(bool)), this, SLOT(OnCancelButtonClicked()));
    connect(ui->close_btn, SIGNAL(clicked()), this, SLOT(close()));
}

void ToViewPhraseDialog::FillDataIntoTable(QMap<int,PHRASE> phraseMap)
{
    ui->table_widget->clearContents();
    ui->table_widget->setRowCount(0);
    int row_index = 0;
    QMap<int, PHRASE>::iterator it = phraseMap.begin();
    for(; it!= phraseMap.end(); it++)
    {
        if(it.value().state == REMOVED)
            continue;
        QTableWidgetItem *item = new QTableWidgetItem(((PHRASE)it.value()).abbr);
        item->setTextAlignment(Qt::AlignCenter);
        Qt::ItemFlags flags = item->flags();
        flags &= ~Qt::ItemIsEditable;
        item->setFlags(flags);
        ui->table_widget->insertRow(row_index);
        ui->table_widget->setItem(row_index, 0, item);

        QString content = ((PHRASE)it.value()).content;
        if(content.length() > 9)
        {
            content = content.left(9);
            content.append("...");
        }
        item = new QTableWidgetItem(content);
        item->setToolTip(((PHRASE)it.value()).content);
        item->setTextAlignment(Qt::AlignCenter);
        flags = item->flags();
        flags &= ~Qt::ItemIsEditable;
        item->setFlags(flags);
        ui->table_widget->setItem(row_index, 1, item);

        item = new QTableWidgetItem(((PHRASE)it.value()).position > 0 ? QString::number(((PHRASE)it.value()).position) : "-");
        item->setTextAlignment(Qt::AlignCenter);
        flags = item->flags();
        flags &= ~Qt::ItemIsEditable;
        item->setFlags(flags);
        ui->table_widget->setItem(row_index, 2, item);

        row_index++;
    }
}

void ToViewPhraseDialog::OnAddPhraseButtonClicked()
{
    m_addorEditDlg->show();

    m_addorEditDlg->hide();

    m_addorEditDlg->ResetContent();
    m_addorEditDlg->exec();
}

void ToViewPhraseDialog::OnPhraseAddedOrChanged(PHRASE phrase, int phrase_index)
{
    if(phrase_index == -1)
    {
        int index = m_tempPhrase.isEmpty()? 0 : m_tempPhrase.lastKey() + 1;
        m_tempPhrase.insert(index,phrase);
    }
    else
    {
        m_tempPhrase[phrase_index] = phrase;
    }
    FillDataIntoTable(m_tempPhrase);
    SetPhraseIsModified(true);
    PhraseModel::Instance()->SetParsedPhrase(m_tempPhrase);
    PhraseModel::Instance()->SavePhraseToRealFile();
}

void ToViewPhraseDialog::OnEditPhraseFileButtonClicked()
{

    if(!CompareTempPhraseAndRealPhrase())
    {
        if(Utils::NoticeMsgBox("已添加自定义短语\n是否保存到文件?",this,1) == QDialog::Accepted)
        {
            PhraseModel::Instance()->SetParsedPhrase(m_tempPhrase);
            PhraseModel::Instance()->SavePhraseToRealFile();
        }
    }

    //先保存一下当前页面
    m_editPhraseFileDlg->LoadDataIntoPhraseWidget();
    int returnCode = m_editPhraseFileDlg->exec();
    if(returnCode == QDialog::Rejected)
    {
        return;
    }
    else
    {
        //刷新当前页面
        PhraseModel::Instance()->LoadPhraseFromFile();
        m_tempPhrase = PhraseModel::Instance()->GetParsedPhrase();
        FillDataIntoTable(m_tempPhrase);
        //将过滤出来的合法的短语保存到RealFile
        PhraseModel::Instance()->SavePhraseToRealFile();
    }
}

void ToViewPhraseDialog::OnViewSysPhraseButtonClicked()
{
    m_fileViewDlg->exec();
}

void ToViewPhraseDialog::OnApplyButtonClicked()
{
    int toggle = 0;
    if(Qt::Checked == ui->open_phrase_check_box->checkState())
        toggle = 1;
    ConfigItemStruct configInfo;
    configInfo.itemCurrentIntValue = toggle;
    configInfo.itemName = "use_sys_phrase";
    Config::Instance()->SetConfigItemByJson("use_sys_phrase",configInfo);

    SaveDefaultPhrasePoistion();
    PhraseModel::Instance()->SetParsedPhrase(m_tempPhrase);
    PhraseModel::Instance()->SavePhraseToRealFile();
    this->close();
}

void ToViewPhraseDialog::SetPhraseIsModified(bool toggle)
{
    m_phrase_modified = toggle;
}

bool ToViewPhraseDialog::GetPhraseIsModified()
{
    return m_phrase_modified;
}

void ToViewPhraseDialog::closeEvent(QCloseEvent *event)
{
    event->accept();
}

void ToViewPhraseDialog::SetStyleSheet()
{

}

void ToViewPhraseDialog::SetDefaultPhrasePosition()
{
    QStringList positions;
    positions << "1" << "2" << "3"
              << "4" << "5" << "6"
              << "7" << "8" << "9";
    ui->comboBox->addItems(positions);
    ui->comboBox->setView(new QListView());
    ui->comboBox->setMaxVisibleItems(6);
    ConfigItemStruct configInfo;
    if(Config::Instance()->GetConfigItemByJson("spw_position",configInfo))
    {
       QString text = QString::number(configInfo.itemCurrentIntValue);
       int currentIndex = ui->comboBox->findText(text);
       ui->comboBox->setCurrentIndex(currentIndex);
    }
    else
    {
        ui->comboBox->setCurrentIndex(0);
    }

}

void ToViewPhraseDialog::SaveDefaultPhrasePoistion()
{
    ConfigItemStruct configInfo;
    if(Config::Instance()->GetConfigItemByJson("spw_position",configInfo))
    {
        if(configInfo.itemCurrentIntValue != ui->comboBox->currentText().toInt())
        {
            configInfo.itemName = "spw_position";
            int currentValue = ui->comboBox->currentText().toInt();
            configInfo.itemCurrentIntValue = currentValue;
            Config::Instance()->SetConfigItemByJson("spw_position",configInfo);
        }
    }
    else
    {
        ui->comboBox->setCurrentIndex(0);
    }
}

void ToViewPhraseDialog::OnCloseButtonClicked()
{
    close();
}

void ToViewPhraseDialog::OnCancelButtonClicked()
{
    close();
}

void ToViewPhraseDialog::ScaledToHighDpi()
{
    double geoRate = Config::Instance()->GetGeoRate();

    this->resize(500*geoRate,300*geoRate);
    m_addorEditDlg->resize(400*geoRate,424*geoRate);
    m_editPhraseFileDlg->resize(400*geoRate,250*geoRate);
    m_fileViewDlg->resize(350*geoRate,430*geoRate);

}

void ToViewPhraseDialog::UpdateComboBoxStyleSheet()
{
    double fontRate = Config::Instance()->GetFontScaledRate();
    QList<QComboBox*> resultCombo = this->findChildren<QComboBox*>();
    for(int index=0; index<resultCombo.size(); ++index)
    {
        QFont font = resultCombo.at(index)->font();
        font.setPixelSize(14*fontRate);
        resultCombo.at(index)->setFont(font);
        resultCombo.at(index)->view()->setFont(font);
    }

}

bool ToViewPhraseDialog::CompareTempPhraseAndRealPhrase()
{
    //先判断一下是否有变化
    QMap<int,PHRASE> realPhrase = PhraseModel::Instance()->GetParsedPhrase();
    if(m_tempPhrase.size() != realPhrase.size())
    {
        return false;
    }
    QList<int> keyList = m_tempPhrase.keys();
    for(int index=0; index<keyList.size(); ++index)
    {
        if(!realPhrase.contains(keyList.at(index)))
        {
            return false;
        }
        PHRASE  tempPhrase = m_tempPhrase[keyList.at(index)];
        PHRASE realItem = realPhrase[keyList.at(index)];
        if((tempPhrase.abbr == realItem.abbr)
                && (tempPhrase.state == realItem.state)
                && (tempPhrase.content == realItem.content)
                && (tempPhrase.position == realItem.position))
        {
            continue;
        }
        return false;

    }
    return true;
}

void ToViewPhraseDialog::SlotEditPrase()
{
    Customizephrasebtn* senderObj = qobject_cast<Customizephrasebtn*>(sender());
    if(senderObj == nullptr)
    {
        return;
    }
    int selectedRow = senderObj->GetRowNum();
    int i=0,validRow=0;
    for(;i<m_tempPhrase.size();i++)
    {
        if(m_tempPhrase[i].state!=REMOVED)
        {
            if((++validRow) == (selectedRow+1))
                break;
        }
    }
    if(i>=m_tempPhrase.size())
        return;
    PHRASE phrase = m_tempPhrase.find(i).value();

    AddOrEditPhraseDialog *dialog = new AddOrEditPhraseDialog(EDIT_PHRASE_DIALOG,selectedRow, &phrase, this);
    double geoRate = Config::Instance()->GetGeoRate();
    double fontRate = Config::Instance()->GetFontScaledRate();

    int width = 380*geoRate;
    int height = 450*geoRate;
    dialog->resize(width,height);


    dialog->show();
    DpiAdaptor adaptor;
    adaptor.ChangeObjectSizeAndFontSize(*dialog,geoRate,fontRate);
    dialog->hide();

    connect(dialog, SIGNAL(SignalToSavePhrase(PHRASE, int)), this, SLOT(OnPhraseAddedOrChanged(PHRASE, int)));
    dialog->exec();
}

void ToViewPhraseDialog::SlotDelPrase()
{
    if(Utils::NoticeMsgBox("是否确定删除短语?",this,1) == QDialog::Accepted)
    {
        Customizephrasebtn* senderObj = qobject_cast<Customizephrasebtn*>(sender());
        if(senderObj == nullptr)
        {
            return;
        }
        int selectedRow = senderObj->GetRowNum();

        /*将操作记录暂存，待点击应用后执行操作记录到文件*/
        int i=0,validRow=0;
        for(;i<m_tempPhrase.size();i++)
        {
            if(m_tempPhrase[i].state!=REMOVED)
            {
                if((++validRow) == (selectedRow+1))
                    break;
            }
        }
        if(i>=m_tempPhrase.size())
            return;
        m_tempPhrase[i].state = REMOVED;
        //刷新界面
        SetPhraseIsModified(true);
        FillDataIntoTable(m_tempPhrase);
        PhraseModel::Instance()->SetParsedPhrase(m_tempPhrase);
        PhraseModel::Instance()->SavePhraseToRealFile();
    }
}

void ToViewPhraseDialog::SlotRowIndexChanged(QPoint index)
{
    //计算悬浮窗口的位置
    QTableWidgetItem* currentItem = ui->table_widget->itemAt(index);
    int width = ui->table_widget->width();
    int headerHeight = ui->table_widget->horizontalHeader()->height();

    if(currentItem == NULL)
    {
        m_btnGroupEdit->close();
        m_btnGroupDel->close();
        return;
    }

    int rowHeight = ui->table_widget->rowHeight(0);
    int rowNum = (index.y())/rowHeight;


    static int currentRow = -1;
    currentRow = rowNum;
    int offSet = (m_btnGroupDel->height() - rowHeight)/2;
    int yPos = headerHeight + (rowNum)*rowHeight - offSet;
    int xPos = width - m_btnGroupDel->width();
    QPoint lastPoint;
    lastPoint.setX(xPos);
    lastPoint.setY(yPos);
    QPoint globalPoint = ui->table_widget->mapToGlobal(lastPoint);
    m_btnGroupDel->move(globalPoint);
    DpiAdaptor adaptor;
    double geoRate = Config::Instance()->GetGeoRate();
    double fontRate = Config::Instance()->GetFontScaledRate();
    int groupheight = m_btnGroupDel->minimumHeight()*geoRate;
    int groupwidth = m_btnGroupDel->minimumWidth()*geoRate;
    m_btnGroupDel->resize(groupwidth,groupheight);
    adaptor.ChangeObjectSizeAndFontSize(*m_btnGroupDel,geoRate,fontRate);
    m_btnGroupDel->SetRowNum(currentItem->row());
    if(!m_btnGroupDel->isVisible())
    {
        m_btnGroupDel->setVisible(true);
    }
    m_btnGroupDel->show();

    int yPos2 = headerHeight + (rowNum)*rowHeight - offSet;
    int xPos2 = width - 2*m_btnGroupEdit->width();
    QPoint lastPoint2;
    lastPoint2.setX(xPos2);
    lastPoint2.setY(yPos2);
    QPoint globalPoint2 = ui->table_widget->mapToGlobal(lastPoint2);
    m_btnGroupEdit->move(globalPoint2);
    DpiAdaptor adaptor2;
    int groupheight2 = m_btnGroupEdit->minimumHeight()*geoRate;
    int groupwidth2 = m_btnGroupEdit->minimumWidth()*geoRate;
    m_btnGroupEdit->resize(groupwidth2,groupheight2);
    adaptor2.ChangeObjectSizeAndFontSize(*m_btnGroupEdit,geoRate,fontRate);
    m_btnGroupEdit->SetRowNum(currentItem->row());
    if(!m_btnGroupEdit->isVisible())
    {
        m_btnGroupEdit->setVisible(true);
    }
    m_btnGroupEdit->show();
}

void ToViewPhraseDialog::SlotTableWidgetItemPressed(QTableWidgetItem* clickedItem)
{
    ui->table_widget->setFocus();
    int rowNum = clickedItem->row();
    m_btnGroupEdit->SetRowNum(rowNum);
    m_btnGroupDel->SetRowNum(rowNum);
}

void ToViewPhraseDialog::SlotTableWidgetLoseFocus()
{
    m_btnGroupEdit->hide();
    m_btnGroupDel->hide();
}

void ToViewPhraseDialog::focusOutEvent(QFocusEvent* e)
{
    Q_UNUSED(e);
    m_btnGroupEdit->hide();
    m_btnGroupDel->hide();
}

void ToViewPhraseDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (!event->buttons().testFlag(Qt::LeftButton) || !m_move_widget_flag)
            return;
    const QPoint position = pos() + event->globalPos() - m_last_mouse_position;
    move(position.x(), position.y());
    m_last_mouse_position = event->globalPos();

    if(!ui->table_widget->geometry().contains(this->ui->phrase_show_eara->mapToGlobal(QCursor::pos())))
    {
        m_btnGroupEdit->hide();
        m_btnGroupDel->hide();
    }
}

void ToViewPhraseDialog::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ToViewPhraseDialog::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_last_mouse_position = event->globalPos();
        m_move_widget_flag = true;
    }
}

void ToViewPhraseDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_move_widget_flag = false;
    }
}




















