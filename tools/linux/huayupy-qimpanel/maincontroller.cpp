// 主控

#include "maincontroller.h"
#include <QDebug>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusError>
#include <QProcess>
#include <QMessageBox>
#include <QIcon>
#include <QPixmap>
#include <QCursor>
#include <QToolTip>
#include <QDebug>

#include <unistd.h>
#include <stdlib.h>

#include "kimpanelagent.h"
#include "systemtraymenu.h"
#include "myaction.h"
#include "skin/skinbase.h"
#include "skin/skinfcitx.h"
#include "../public/utils.h"
#include "traycontroller.h"
#include "../public/config.h"
#include "../public/configmanager.h"

#include <X11/extensions/XTest.h>
#include <X11/Xlib.h>
#include <X11/keysymdef.h>

using namespace std;

static void ShowMenuCallback_(void* maincontroller, int x, int y)
{
    Q_UNUSED(x)
    Q_UNUSED(y)
    MainController* controller = (MainController*)maincontroller;
    controller->ShowMenu(QCursor::pos().x(), QCursor::pos().y());
}

static void ChangeIMCallback_(void* maincontroller)
{
    MainController* controller = (MainController*)maincontroller;
    controller->ChangeToEnglishMode();
}

MainController* MainController::mInstance = NULL;

MainController::MainController(QObject *parent)
    : QObject(parent), mSystemTray(NULL), mAgent(NULL)
    , mTrayMenu(NULL), mSkinBase(NULL)
    , mModel(NULL), mToolBarModel(NULL), mIcon(NULL)
    , mIsHorizontal(true), mServiceOpen(false)
    , mFcitxMonitorTimer(NULL), mIsFisrtUpdateImProperty(true)
    , mHasUpdate(false), mCandidateWindow(NULL), mToolbarWindow(NULL)
    , mCanShow(true)
{

}

MainController::~MainController()
{
    if (mSystemTray)
    {
        delete mSystemTray;
        mSystemTray = NULL;
    }
    if (mAgent)
    {
        delete mAgent;
        mAgent = NULL;
    }
    if (mTrayMenu)
    {
        delete mTrayMenu;
        mTrayMenu = NULL;
    }
    if (mModel)
    {
        delete mModel;
        mModel = NULL;
    }
    if (mSkinBase)
    {
        delete mSkinBase;
        mSkinBase = NULL;
    }
    if (mToolBarModel)
    {
        delete mToolBarModel;
        mToolBarModel = NULL;
    }
    if (mIcon)
    {
        delete mIcon;
        mIcon = NULL;
    }
    if(mCandidateWindow)
    {
        delete mCandidateWindow;
        mCandidateWindow = NULL;
    }
    if(mToolbarWindow)
    {
        delete mToolbarWindow;
        mToolbarWindow = NULL;
    }
}

MainController* MainController::Instance()
{
    if (!mInstance)
    {
        mInstance = new MainController();
        mInstance->Init();
    }

    return mInstance;
}

void MainController::Init()
{
    LoadConfig();

    OpenDBus();

    mSkinBase = new SkinFcitx;

    QString install_path = Config::Instance()->GetPackageInstallpath();
    QString skin_file_path = QDir::toNativeSeparators(install_path + QString("files/skin/default/"));

#ifdef USE_FCITX
    mSkinBase->loadSkin(skin_file_path);
    mIcon = new QIcon(":/image/huayupy_tray.png");
#else
    mSkinBase->loadSkin(skin_file_path);
    mIcon = new QIcon(":/image/huayupy_tray.png");
#endif
#ifndef QT5
    QStringList sl = QIcon::themeSearchPaths();
    sl.append("/usr/share/pixmaps");
    QIcon::setThemeSearchPaths(sl);
#endif
    mAgent = new KimpanelAgent(this);

    mTrayMenu = new SystemTrayMenu(mAgent, mHideTray);
    mTrayMenu->setFocusPolicy(Qt::NoFocus);

    mAgent->Created();
    mTrayMenu->Init();

#ifdef USE_FCITX
    mSystemTray = new TrayController((ShowMenuCallback)ShowMenuCallback_, this);
    mSystemTray->SetChangeIMCallBack((ChangeIMCallback)ChangeIMCallback_);
#else
    mSystemTray = new QSystemTrayIcon(this);
    mSystemTray->setIcon(*mIcon);
    mSystemTray->setContextMenu(mTrayMenu);
    mSystemTray->setToolTip("华宇输入法");
#endif

    QString configFilePath = Config::Instance()->configFilePath();
    QSettings setting(configFilePath,QSettings::IniFormat);
    //mIsWb = setting.value("mode/wubimode","0").toBool();

    int default_chinese_input_mode = setting.value("basic/default_chinese_input_mode",
                                                DEFAULT_CHINESE_INPUT_MODE_PINYIN).toInt();
    mIsWb = (default_chinese_input_mode == DEFAULT_CHINESE_INPUT_MODE_WUBI);

    mIsUmode = setting.value("mode/enableumode", 1).toBool();
    mPinyinMode = setting.value("basic/pinyin_mode", 0).toInt();
    mToolbarConfig = setting.value("basic/tool_bar_config","127").toInt();

    CreateInputView(mAgent);
    CreateToolBarView();

    connect(mTrayMenu, SIGNAL(hideTray(int)), this ,SLOT(HandlerHideTray(int)));

    connect(mAgent, SIGNAL(UpdatePropertySignal(KimpanelProperty)), this, SLOT(UpdateProperty(KimpanelProperty)));
    connect(mAgent, SIGNAL(UpdatePreeditTextSignal(QString, QList<TextAttribute>)), this, SLOT(UpdatePreeditText(QString, QList<TextAttribute>)));
    connect(mAgent, SIGNAL(UpdateLookupTableSignal(KimpanelLookupTable)), this, SLOT(UpdateLookupTable(KimpanelLookupTable)));
    connect(mAgent, SIGNAL(UpdateLookupTableFullSignal(KimpanelLookupTable, int, int)), this, SLOT(UpdateLookupTableFull(KimpanelLookupTable,int,int)));
    connect(mAgent, SIGNAL(UpdateSpotLocationSignal(int, int)), this, SLOT(UpdateSpotLocation(int, int)));
    connect(mAgent, SIGNAL(UpdateSpotRectSignal(int, int, int, int)), this, SLOT(UpdateSpotRect(int, int, int, int)));
    connect(mAgent, SIGNAL(ShowPreeditSignal(bool)), this, SLOT(ShowPreedit(bool)));
    connect(mAgent, SIGNAL(ShowAuxSignal(bool)), this, SLOT(ShowAux(bool)));
    connect(mAgent, SIGNAL(UpdateAuxSignal(QString,QList<TextAttribute>)), this, SLOT(UpdateAux(QString, QList<TextAttribute>)));
    connect(mAgent, SIGNAL(ShowLookupTableSignal(bool)), this, SLOT(ShowLookupTable(bool)));
    connect(mAgent, SIGNAL(UpdateLookupTableCursorSignal(int)), this, SLOT(UpdateLookupTableCursor(int)));
    connect(mAgent, SIGNAL(RegisterPropertiesSignal(QList<KimpanelProperty>)), this, SLOT(RegisterProperties(QList<KimpanelProperty>)));
    connect(mAgent, SIGNAL(UpdatePreeditCaretSignal(int)), this, SLOT(UpdatePreeditCaret(int)));
    connect(mAgent, SIGNAL(OnRecieveSignalHiddenQimPanel()), this, SLOT(HiddenQimPanel()));
    connect(mAgent, SIGNAL(OnRecieveSingalToWubiMode(int)),this,SLOT(ChangeWubiMode(int)));
    connect(mAgent, SIGNAL(OnUmodeChanged(int)), this, SLOT(ChangedUmode(int)));
    connect(mAgent, SIGNAL(OnReciveGetInputMode(int)), this, SLOT(GotInputModeFromEngine(int)));
    connect(mAgent, SIGNAL(OnReciveSetPinyinMode(int)), this, SLOT(SetPinyinMode(int)));
    //connect(mAgent, SIGNAL(GotResponseInputMode(int)), this, SLOT(ChangeInputMode(int)));
    connect(mAgent, SIGNAL(GotResponseEngineState(int)), this, SLOT(GotStateFromEngine(int)));
    connect(mAgent, SIGNAL(GotResponseHuayuInUse(int)), this, SLOT(GotInUseFromEngine(int)));
    connect(mAgent, SIGNAL(SetHypyEnable(bool)), this, SLOT(OnSetHypyEnable(bool)));
    connect(mAgent,SIGNAL(OnReciveSetExpandMode(int)),this,SLOT(ChangeCandiatesExpandFlag(int)));
    connect(mAgent,SIGNAL(OnHiddenToolbar()),this,SLOT(OnHiddenToolbar()));

#ifndef USE_FCITX
    connect(mSystemTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(OnTrayIconActivated(QSystemTrayIcon::ActivationReason)));
#endif

    CreateDBusService();
    MonitorFcitx();
    InitPanelSize();
}

void MainController::CreateDBusService()
{
    // 用于建立到session bus的连接
    QDBusConnection bus = QDBusConnection::sessionBus();
    // 在session bus上注册名为"com.huayupy.qimpanel.hotel"的service
    if (!bus.registerService("com.thunisoft.huayupy.hotel"))
    {
            qDebug() << bus.lastError().message();
            exit(1);
    }
    // "QDBusConnection::ExportAllSlots"表示把类Hotel的所有Slot都导出为这个Object的method
#ifdef QT5
    bus.registerObject("/", "com.thunisoft.qimpanel", mInstance ,QDBusConnection::ExportAllSlots); //可以优化只导出需要的
#else
    bus.registerObject("/", mInstance, QDBusConnection::ExportAllSlots);
#endif
    QDBusConnection::sessionBus().connect("","/","com.thunisoft.qimpanel","ChangeInputMode",this, SLOT(ChangeInputMode(int)));
    QDBusConnection::sessionBus().connect("","/","com.thunisoft.qimpanel","istraditional",this, SLOT(ChangeTraditional(int)));
    QDBusConnection::sessionBus().connect("","/","com.thunisoft.qimpanel","wubimode",this, SLOT(ChangeWubiMode(int)));
    QDBusConnection::sessionBus().connect("","/","com.thunisoft.qimpanel","enableumode",this, SLOT(ChangedUmode(int)));
    QDBusConnection::sessionBus().connect("","/","com.thunisoft.qimpanel","toolbar_config",this, SLOT(SetToolBarConfig(int)));
    QDBusConnection::sessionBus().connect("","/","com.thunisoft.qimpanel","qimpanel_config_str",this, SLOT(SetQimpanel(QString, QString)));
    QDBusConnection::sessionBus().connect("","/","com.thunisoft.qimpanel","qimpanel_config_int",this, SLOT(SetQimpanel(QString, int)));
    QDBusConnection::sessionBus().connect("","/","com.thunisoft.qimpanel", "using_english_punctuation_in_chinese_mode",
                                          this, SLOT(OnUsingEnglishPunctuationInChineseMode(int)));
    QDBusConnection::sessionBus().connect("","/","com.thunisoft.qimpanel","hide_tray",this, SLOT(GetHideTrayFromSet(int)));
    QDBusConnection::sessionBus().connect("","/","com.thunisoft.qimpanel","has_update",this, SLOT(SetHasUpdate(int)));
    QDBusConnection::sessionBus().connect("","/","com.thunisoft.qimpanel","loginstate_changed",this, SLOT(ReloadLoginState()));
    QDBusConnection::sessionBus().connect("","/","com.thunisoft.qimpanel","CurrentExpandMode",this, SLOT(ChangeCandiatesExpandFlag(int)));
	QDBusConnection::sessionBus().connect("","/","com.thunisoft.qimpanel","show_addon_str",this, SLOT(slot_fix_target_addon_on_toolbar(QString)));
    QDBusConnection::sessionBus().connect("","/","com.thunisoft.qimpanel", "hz_option",
                                          this, SLOT(OnHzOptionChanged(int)));
    QDBusConnection::sessionBus().connect("","/","com.thunisoft.qimpanel","CallOCRAddon",this, SLOT(execuate_ocr_addon()));
    QDBusConnection::sessionBus().connect("","/","com.thunisoft.qimpanel","CallVoiceAddon",this, SLOT(execuate_voice_addon()));
    QDBusConnection::sessionBus().connect("","/","com.thunisoft.qimpanel","NameCandidatesIndex",this, SLOT(update_person_name_candidate_index(int)));
}

void MainController::OpenDBus()
{
    if (mServiceOpen)
        return;

    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.registerService("org.kde.impanel"))
    {
        QString logcontent = QString("DBus register error:%1").arg(conn.lastError().message());
        Config::Instance()->WriteQimpanelLog(logcontent);

    }
    mAgent->OpenDBus();
    mServiceOpen = true;
}

void MainController::CloseDBus()
{
    if (!mServiceOpen)
        return;

    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.unregisterService("org.kde.impanel"))
    {
        QString logcontent = QString("DBus unregister error:%1").arg(conn.lastError().message());
        Config::Instance()->WriteQimpanelLog(logcontent);

    }
    mAgent->CloseDBus();
    mServiceOpen = false;
}

void MainController::LoadConfig()
{
    QSettings* settings = new QSettings("huayupy-qimpanel", "main");
    QString defaultSkin;
    settings->setIniCodec("UTF-8");
    settings->beginGroup("base");
    mIsHorizontal = !settings->value("VerticalList", false).toBool();
    qDebug() << "mIsHorizontal:" << mIsHorizontal;
    defaultSkin = "ubuntukylin-dark1";
    mSkinName = settings->value("CurtSkinType", defaultSkin).toString();
    qDebug() << "mSkinName:" << mSkinName;
    settings->endGroup();
    delete settings;

    QString config_file_path = Config::Instance()->configFilePath();
    QSettings config(config_file_path, QSettings::IniFormat);
    mHideTray = config.value("skin/hide_tray", 0).toInt();

    mHasUpdate = Utils::HasNewVersion();
}

void MainController::ReloadHideTray()
{
    QString config_file_path = Config::Instance()->configFilePath();
    QSettings config(config_file_path, QSettings::IniFormat);
    mHideTray = config.value("skin/hide_tray", 0).toInt();
}

QString MainController::GetSkinName()
{
    return mSkinName;
}

void MainController::SetSkinName(const QString &skinName)
{
    mSkinName = skinName;
    QSettings* settings = new QSettings("huayupy-qimpanel", "main");
    settings->setIniCodec("UTF-8");
    settings->beginGroup("base");
    settings->setValue("CurtSkinType", skinName);
    settings->endGroup();
    delete settings;
}

void MainController::SetSkinBase(SkinBase* skinBase, int skinType)
{
    qDebug() << "MainController::SetSkinBase";
    if (mSkinBase != skinBase)
        delete mSkinBase;
    mSkinBase = skinBase;

    Q_UNUSED(skinType)
}

SkinBase* MainController::GetSkinBase()
{
    return mSkinBase;
}

ToolBarModel* MainController::GetToolbarModel()
{
    return mToolBarModel;
}

// 处理引擎发过来的状态更新的消息
// 来自 fcitx 的 src/ui/kimpanel
void MainController::UpdateProperty(const KimpanelProperty &prop)
{
    qDebug() << "MainController::UpdateProperty: prop.lable=" << prop.label;
    if (prop.label.compare("无输入窗口") == 0)
    {
#ifndef USE_FCITX
        mSystemTray->setIcon(*mIcon);
#endif
        return;
    }

    if (prop.key == "/Fcitx/im")
        UpdateImProperty(prop);
    else if (prop.key == "/Fcitx/chttrans")
        UpdateChttransProperty(prop);
    else if (prop.key == "/Fcitx/fullwidth")
        UpdateFullWidthProperty(prop);
    else if (prop.key == "/Fcitx/punc")
        UpdatePuncProperty(prop);
    else if (prop.key == "/Fcitx/vk")       // 软键盘(没用)
        UpdateSoftKbdProperty(prop);
    else
        return;
}

// 处理用户输入串
void MainController::UpdatePreeditText(const QString inputString, QList<TextAttribute> attributes)
{
    Q_UNUSED(attributes)

    //mModel->setInputString(inputString);
    mCandidateWindow->SetInputString(inputString);
}

// 处理候选
void MainController::UpdateLookupTable(const KimpanelLookupTable &table)
{
    mCandidateWindow->SetCandidates(table);
}

//必须与FcitxCandidateLayoutHint的定义一致
enum CandidateLayout
{
    CLH_NotSet,
    CLH_Vertical,
    CLH_Horizontal
};

// 处理所有的消息(横竖排、高亮)
void MainController::UpdateLookupTableFull(const KimpanelLookupTable &table, int cursor, int layout)
{
    qDebug() << "MainController::UpdateLookupTableFull";

    bool isHorizontal;

    switch (layout)
    {
    case CLH_Vertical:
        isHorizontal = false;
        break;
    case CLH_Horizontal:
        isHorizontal = true;
        break;
    default:
        isHorizontal = mIsHorizontal;
        break;
    }

    int always_expand_flag = Config::Instance()->always_expand_candidates();
    //add expand mode
    if(isHorizontal)
    {
        if(mModel->candidatesExpandFlag() || always_expand_flag)
        {
            mCandidateWindow->SetCandidatesViewMode(VIEW_MODE_EXPAND);
        }
        else
        {
            mCandidateWindow->SetCandidatesViewMode(VIEW_MODE_HORIZONTAL);
        }

    }
    else
    {
        mCandidateWindow->SetCandidatesViewMode(VIEW_MODE_VERTICAL);
    }

    mCandidateWindow->SetHightIndex(cursor);        // 高亮
    UpdateLookupTable(table);
}

// 设置输入框位置
void MainController::UpdateSpotLocation(int x, int y)
{
    y += 6;//候选栏光标向下偏移两个像素
    mCandidateWindow->SetWindowPos(x, y);
}

// 设置输入框位置
void MainController::UpdateSpotRect(int x, int y, int width, int height)
{
    Q_UNUSED(width);
    y = y + 6;
    mToolbarWindow->HideChildWindows();
    mCandidateWindow->SetWindowPos(x, y);
}

// 是否显示预上屏的拼音串
void MainController::ShowPreedit(bool toShow)
{
    mModel->setShowPreedit(toShow);

    if(!toShow)
        mCandidateWindow->hide();
}

// 是否显示提示
void MainController::ShowAux(bool toShow)
{
    Q_UNUSED(toShow)
}

// 是否更新提示
void MainController::UpdateAux(const QString &text, const QList<TextAttribute> &attrs)
{
    Q_UNUSED(attrs)
    Q_UNUSED(text)
}

// 显示/隐藏候选框
void MainController::ShowLookupTable(bool toShow)
{
    mModel->setShowLookupTable(toShow);
    mToolbarWindow->HideChildWindows();
}

// 显示高亮
void MainController::UpdateLookupTableCursor(int pos)
{
    qDebug() << "UpdateLookupTableCursor-------------------------";
    mCandidateWindow->SetHightIndex(pos);
}

// 批量处理状态消息
void MainController::RegisterProperties(const QList<KimpanelProperty> &props)
{
    qDebug() << "MainController::RegisterProperties";
    foreach (const KimpanelProperty& prop, props)
    {
        UpdateProperty(prop);
    }
}

// 处理光标位置
void MainController::UpdatePreeditCaret(int pos)
{
    qDebug() << "UpdatePreeditCaret" <<endl;
    mCandidateWindow->SetCartIndex(pos);

    mCandidateWindow->DrawInputWindow();
    mCandidateWindow->ToShow();
}

// 整体隐藏
void MainController::HiddenQimPanel()
{
    if(mToolbarWindow->isVisible())
    {
        DoShowUI(false);
    }
    else
    {
        DoShowUI(false);
    }
}

// 输入法切换的时候调用
// 等待 200ms 其它输入法与 kimpanel 断开连接
void MainController::ShowUI(bool toShow)
{
    qDebug() << QString("MainController::ShowUI(%1)").arg(toShow);

    // 延迟一定时间显示自己的ui。
    // 如果不延迟，会出现两个托盘的问题。
    // 因为自己的ui在调用Create之前，fcitx的托盘图标尚未创建完成，
    // 而自己ui已经创建好了自己的托盘，当fcitx创建好之后，出现了2个托盘的问题
    mCanShow = true;
    QTimer timer;
    timer.singleShot(200, this, SLOT(Wait()));
}

// 右击 TaryIcon
void MainController::OnTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    qDebug() << QString("MainController::OnTrayIconActivated(%1)").arg(reason);
     if (reason == QSystemTrayIcon::Trigger)
     {
        mAgent->ChangeToNextIm();
     }
}

// 通过界面的图标来翻页
void MainController::getPrevPage()
{
    mAgent->LookupTablePageUp();
}

// 通过界面的图标来翻页
void MainController::getNextPage()
{
    mAgent->LookupTablePageDown();
}

// 鼠标选择候选
void MainController::selectCandidate(int index)
{
    mAgent->SelectCandidate(index);
}

//中英文进行切换
void MainController::changeCnMode(bool isCn)
{
    ChangeImToEnglish(isCn);
}

// 通过点击工具栏按钮切换五笔和拼音模式
void MainController::toggleWbOrPinyinMode(bool isWubi)
{
    ChangeImToWB(isWubi);
    mToolBarModel->toresetWindow();
}

// 切换繁简
void MainController::changeTrad(bool isTrad)
{
    mToolBarModel->setTrad(isTrad);
    //mAgent->TriggerProperty("/Fcitx/chttrans");
    mAgent->SetCurrentOutputMode(isTrad ? 1 : 0);
    SaveIsTraditional(isTrad);
}

// 切换全半角
void MainController::changeFullWidth(bool isFull)
{
    mToolBarModel->setFullWidth(isFull);
    mAgent->TriggerProperty("/Fcitx/fullwidth");
}

// 切换中英文符号
void MainController::changePunc(bool isCnMark)
{
    Q_UNUSED(isCnMark)
    //这块不用设置，一会儿引擎会传回来，否则会设置重了。
    //mToolBarModel->setCnMark(isCnMark);

    mAgent->TriggerProperty("/Fcitx/punc");
}

// 显示软键盘
// UOS 已经屏蔽了
void MainController::showSoftKbd(bool show)
{
    mToolBarModel->setSoftKbd(show);
    if (show)
        mAgent->TriggerProperty("/Fcitx/vk/0");
    else
        mAgent->TriggerProperty("/Fcitx/vk/11");
}

// 启动配置工具
void MainController::showConfigDialog(bool show)
{
    Q_UNUSED(show);
    if(!Utils::IsProcessOn(QString("huayupy-config-tools-fcitx")))
    {
        FILE* ptr2 = NULL;
        string command = Utils::GetBinFilePath().append("huayupy-config-tools-fcitx 82717623-mhe4-0293-aduh-ku87wh6328ne &").toStdString();
        if((ptr2 = popen(command.c_str(), "r")) != NULL)
        {
            pclose(ptr2);
        }
    }
    else
    {
        // 最小化的时候激活
        QDBusMessage msg = QDBusMessage::createSignal("/com/thunisoft/huayupy", "com.thunisoft.huayupy", QString("to_show"));
        QDBusConnection::sessionBus().send(msg);
    }

}

// 启动符号大全
void MainController::showSymbolsDialog(bool show)
{
    Q_UNUSED(show);

    if(!Utils::IsProcessOn(QString("special-symbols-fcitx")))
    {
        FILE* ptr2 = NULL;
        string command = Utils::GetBinFilePath().append("special-symbols-fcitx 82717623-mhe4-0293-aduh-ku87wh6328ne &").toStdString();
        if((ptr2 = popen(command.c_str(), "r")) != NULL)
        {
            pclose(ptr2);
        }
    }
    else
    {
        QDBusMessage msg = QDBusMessage::createSignal("/com/thunisoft/huayupy", "com.thunisoft.huayupy", QString("to_show"));
        QDBusConnection::sessionBus().send(msg);
    }
}

// 创建工具栏
void MainController::CreateToolBarView()
{
    if(mToolBarModel == NULL)
    {
        mToolBarModel = new ToolBarModel;
        mToolBarModel->loadConfig();
    }

    mToolBarModel->setWb(mIsWb);
    mToolBarModel->setToolbarConfig(mToolbarConfig);

    // 创建工具栏窗口
    mToolbarWindow = new ToolbarWindow();
}

// 创建候选栏
void MainController::CreateInputView(KimpanelAgent* agent)
{
    mModel = MainModel::self();
    mModel->setIsHorizontal(mIsHorizontal);
    mModel->setWb(mIsWb);
    mModel->setumode(mIsUmode);
    mModel->setPinyinMode(mPinyinMode);

    // 候选栏窗口
    mCandidateWindow = new CandidateWindow(agent);
}

// 切换到我们的输入法了
void MainController::UpdateImProperty(const KimpanelProperty &prop)
{
    if (prop.icon == "fcitx-huayupy")
    {
        DoShowUI(true);
        return;
    }
    DoShowUI(false);

    QIcon icon;
    if(prop.icon=="fcitx-kbd" || prop.icon==""|| prop.icon.indexOf("indicator-keyboard")!=-1)
        icon = QIcon::fromTheme("fcitx-kbd");
    else if(prop.icon.contains("/usr/share/fcitx"))
        icon = QIcon(prop.icon);
    else
        icon = QIcon::fromTheme(prop.icon, *mIcon);
    // USE_FCITX 银河麒麟，中可方德
    // USE_UOS   UOS 用
#ifndef USE_FCITX
    //mSystemTray->setIcon(icon);
#endif
    mModel->resetData();
}

// 我们不使用输入框架自带的繁体，有时莫名其妙，输入法框架会将输入法置为繁体，所以在每次接到输入法框架是繁体，就将繁体置为简体。
void MainController::UpdateChttransProperty(const KimpanelProperty &prop)
{    
    bool isTrad = (prop.icon != "fcitx-chttrans-inactive");
    if(isTrad)
        mAgent->TriggerProperty("/Fcitx/chttrans");
}

// 更新全半角状态
void MainController::UpdateFullWidthProperty(const KimpanelProperty &prop)
{
    bool isFullWidth = (prop.icon == "fcitx-fullwidth-active");
    mToolBarModel->setFullWidth(isFullWidth);
}

// 更新符号状态
void MainController::UpdatePuncProperty(const KimpanelProperty &prop)
{
    bool isCnMark = (prop.icon == "fcitx-punc-active");
    mToolBarModel->setCnMark(isCnMark);
}

// 更新软键盘状态(没用)
void MainController::UpdateSoftKbdProperty(const KimpanelProperty &prop)
{
    bool isShow = (prop.icon == "fcitx-vk-active");
    mToolBarModel->setSoftKbd(isShow);
}

// 设置中英文状态
void MainController::ChangeImToEnglish(bool flag)
{
    //change input mode
    mToolBarModel->setCn(flag);
    //QString inputMode = flag? "/Fcitx/im/fcitx-keyboard-cn":"/Fcitx/im/fcitx-keyboard-us";
    mAgent->ChangeInputMode(flag);

    if (flag
        && mToolBarModel->using_english_punctuation_in_chinese_mode())
    {
        if (mToolBarModel->cnMark())
        {
            mToolBarModel->setCnMark(false);
            mAgent->TriggerProperty("/Fcitx/punc");
        }
        else
        {
            ;  // pass
        }
    }
    else
    {
        if (mToolBarModel->cnMark() != flag)
        {
            mToolBarModel->setCnMark(flag);
            mAgent->TriggerProperty("/Fcitx/punc");
        }
    }
}

// 设置 五笔/拼音文状态
void MainController::ChangeImToWB(bool flag)
{
   mToolBarModel->setWb(flag);
   mModel->setWb(flag);
   int value = flag ? 1 : 0;
   mAgent->TogglePinyinOrWbMode(value);
}

void MainController::Wait()
{
    qDebug() << QString("MainController::Wait() mCanShow=（%1)").arg(mCanShow);
    if(mCanShow)
        DoShowUI(true);
}

// 显示 QimPanel
void MainController::DoShowUI(bool toShow)
{
    qDebug() << QString("MainController::DoShowUI(%1)").arg(toShow);
    if (toShow)
    {
        OpenDBus();
        mAgent->Created();
#ifndef USE_FCITX
        mSystemTray->setIcon(*mIcon);
#endif
        mSystemTray->show();
        mTrayMenu->DoAboutToShow();
        //SetWindowIMEType((QWidget*)mSystemTray);
        if(!mHideTray)
            mToolbarWindow->ToShow();
        mEngineNotResponse = false;
        /*向引擎查询当前是英文模式还是中文模式，避免引擎和工具栏表现形式不一致情况*/
        mAgent->QueryEngineState();
    }
    else
    {
        mCanShow = false;
        CloseDBus();
        mToolbarWindow->hide();
        mSystemTray->hide();        
        mCandidateWindow->hide();
    }
}

// 判断是否需要隐藏，判断是否是华宇输入法
void MainController::JudgeShouldHide()
{
    if(!mSystemTray->isVisible())
        return;
    if(mEngineNotResponse)
    {
        //DoShowUI(false);
    }
    if(!IsFcitxOn())
    {
#ifdef QT_NO_DEBUG
        DoShowUI(false);
        return;
#endif
    }
    mEngineNotResponse = true;
    mAgent->QueryHuayuInUse();
}

// 监控 fcitx 进程是否还存货
void MainController::MonitorFcitx()
{
    if(!mFcitxMonitorTimer)
    {
      mFcitxMonitorTimer = new QTimer(this);
    }

    connect(mFcitxMonitorTimer, SIGNAL(timeout()), this, SLOT(JudgeShouldHide()));
    mFcitxMonitorTimer->start(2000);
}

// 隐藏 TrayIcon 图标
void MainController::HandlerHideTray(const int hide_tray)
{
    mHideTray = hide_tray;
    if(mHideTray)
    {
        mToolbarWindow->hide();
    }
    else
    {
        if(mSystemTray->isVisible())
            mToolbarWindow->ToShow();
    }
}

// 获取是否显示 TrayIcon 图标
void MainController::GetHideTrayFromSet(const int hide_tray)
{
    HandlerHideTray(hide_tray);
    mTrayMenu->SetToolStatus(hide_tray);
}

// 在 TrayIcon 上右键菜单
void MainController::ShowMenu(int x, int y)
{
    bool hide_tray = Config::Instance()->get_hide_tray();
    mTrayMenu->SetToolStatus(hide_tray);
    if(mTrayMenu)
    {
        mTrayMenu->show();
        int pos_x = x - mTrayMenu->width() / 2;
        int pos_y = y - mTrayMenu->height();
        mTrayMenu->move(pos_x, pos_y);
    }
}

// 设置字体大小、颜色(int)
void MainController::SetQimpanel(const QString& key, int value)
{
    qDebug() << "key is : "<< key << "; int value is : " << value;
    //int值中可能存在对toolbar的设定，需要实时刷新
    if(WindowConfig::Instance()->SetIntValue(key, value) && mToolbarWindow->isVisible())
        mToolbarWindow->RefreshWindow();
    if(key.compare(QString::fromLocal8Bit("font_height")) == 0)
    {
        mCandidateWindow->RefreshWindow();
    }
    else if(key.compare(QString::fromLocal8Bit("compose_string_color")) == 0)
    {
        mCandidateWindow->ChangeLogoAndButtonColor(value);
    }
    else if(key.compare("ReloadConfig") == 0)
    {
        mCandidateWindow->RefreshWindow();
        mCandidateWindow->ChangeLogoAndButtonColor(WindowConfig::Instance()->compose_string_color);
        mToolbarWindow->RefreshWindow();
    }
    else if(key.compare("toolbar_config") == 0)
    {
        WindowConfig::Instance()->toolbar_config = value;
        mToolbarWindow->DrawWindow();
    }
    else{
        Config::Instance()->LoadConfig();
        mCandidateWindow->RefreshWindow();
        mToolbarWindow->RefreshWindow();
    }
}

// 设置字体大小、颜色(string)
void MainController::SetQimpanel(const QString& key, const QString& value)
{
    qDebug() << "key is : "<< key << "; str value is : " << value;
    WindowConfig::Instance()->SetStrValue(key, value);
    if(key.compare(QString::fromLocal8Bit("chinese_font_name")) == 0)
    {
        mCandidateWindow->SetChineseFont(value);
    }
    else if(key.compare(QString::fromLocal8Bit("english_font_name")) == 0)
    {
        mCandidateWindow->SetEnglishFont(value);
    }
    else {}
}

void MainController::SetToolBarConfig(int configValue)
{
    mToolbarWindow->RefreshWindow();
}

// 判断引擎的进程是否还在
bool MainController::IsFcitxOn()
{
    char command[128] = "ps -ef | grep -w /usr/bin/fcitx | grep -v grep |wc -l ";
    char return_value[150];
    int count = 0;
    FILE* ptr = NULL;
    if((ptr = popen(command, "r")) == NULL)
    {
        return false;
    }
    memset(return_value, 0, sizeof(return_value));
    if((fgets(return_value, sizeof(return_value), ptr))!= NULL)
    {
        count = atoi(return_value);
    }
    pclose(ptr);
    if(count <= 0)
    {
        return false;
    }
    return true;

}

// 没用 mSkinBase 没用了
void MainController::ChangeQimPanelSize(int size)
{
    if(size <= 0)
    {
        return;
    }
    qDebug() << "fontsize" << size;
    mSkinBase->setFontSize(size);
    //CreateInputView();
}

// 初始化候选栏大小
void MainController::InitPanelSize()
{
    QString configFilePath = Config::Instance()->configFilePath();
    QSettings setting(configFilePath,QSettings::IniFormat);
    int size = setting.value("basic/skinfont","14").toInt();
    ChangeQimPanelSize(size);
}

void MainController::SaveWubiConfig(bool is_wubi)
{
    QString configFilePath = Config::Instance()->configFilePath();
    QSettings setting(configFilePath,QSettings::IniFormat);
    setting.setValue("mode/wubimode", is_wubi ? 1 : 0);
}

// 保存是否使用繁体
void MainController::SaveIsTraditional(bool is_trad)
{
    QString configFilePath = Config::Instance()->configFilePath();
    QSettings setting(configFilePath,QSettings::IniFormat);
    setting.setValue("mode/tradition", is_trad ? 1 : 0);
}

// 改变中英文
void MainController::ChangeInputMode(int modeFlag)
{
    mToolBarModel->setCn(modeFlag == 1);
    mToolbarWindow->RefreshWindow();
}

// 询问引擎状态，根据返回结果进行状态同步
void MainController::GotInputModeFromEngine(int input_mode)
{
    bool isCn = input_mode != 0 ? true :false;
    if(mToolBarModel->cn() != isCn)
    {
        mToolBarModel->setCn(input_mode);
        if(!input_mode)
        {}
        if((bool)input_mode != mToolBarModel->cnMark())
        {
            if (mToolBarModel->using_english_punctuation_in_chinese_mode())
            {
                if (mToolBarModel->cnMark())
                {
                    mToolBarModel->setCnMark(false);
                    mAgent->TriggerProperty("/Fcitx/punc");
                }
            }
            else
            {
                mToolBarModel->setCnMark(input_mode);
                changePunc(input_mode);
            }

        }
        mToolbarWindow->RefreshWindow();
    }


}

// 通过配置发送dbus信号调用
void MainController::ChangeWubiMode(int modeFlag)
{
    mToolBarModel->setWb(modeFlag);
    //mToolBarModel->toresetWindow();
    mModel->setWb(modeFlag);
    mToolbarWindow->RefreshWindow();
}

void MainController::ChangedUmode(int modeFlag)
{
    mIsUmode = modeFlag;
    //CreateInputView();
}


void MainController::ChangeTraditional(int is_traditional)
{
    mToolBarModel->setTrad(is_traditional);
    mToolbarWindow->RefreshWindow();
}

void MainController::SetPinyinMode(int pinyin_mode)
{
    qDebug() << "MainController::SetPinyinMode:" << pinyin_mode;
    mPinyinMode = pinyin_mode;
    mModel->setPinyinMode(pinyin_mode); //0:全拼 1：双拼
    mToolbarWindow->RefreshWindow();
}

void MainController::ChangePinyinMode(int pinyin_mode)
{
    mPinyinMode = pinyin_mode;
    mModel->setPinyinMode(pinyin_mode); //0:全拼 1：双拼
    mToolbarWindow->RefreshWindow();
    mAgent->ToggleQuanOrShuangMode(pinyin_mode);
}

// 这个没用了
void MainController::showToolTip(QString toolTip)
{
    QToolTip::showText(QCursor::pos(),toolTip);
}

void MainController::hideToolTip()
{
    QToolTip::hideText();
}

void MainController::showToolBox(QPoint point)
{

    FILE* ptr2 = NULL;
    QString pos_argument(QString("%1#%2").arg(point.x()).arg(point.y()));
    string command = Utils::GetBinFilePath().append(QString("huayupy-toolBox %1 &").arg(pos_argument)).toStdString();
    if((ptr2 = popen(command.c_str(), "r")) != NULL)
    {
        pclose(ptr2);
    }

}

void MainController::execuate_ocr_addon()
{
    TOOL_ADDON_INFO index_addon_info;
    AddonConfig::Instance()->LoadUserAddon();
    if(AddonConfig::Instance()->GetToolAddonInfoByName("OCR",index_addon_info))
    {
        QProcess* process = new QProcess(this);
        QString exe_path = index_addon_info.exe_path;
        QString argument = index_addon_info.call_paramer;
        QStringList argumentlist;
        argumentlist << argument;
        process->startDetached(exe_path,argumentlist);
    }
}

void MainController::execuate_voice_addon()
{
    TOOL_ADDON_INFO index_addon_info;
    AddonConfig::Instance()->LoadUserAddon();
    if(AddonConfig::Instance()->GetToolAddonInfoByName("AsrInput",index_addon_info))
    {
        QProcess process;
        QString exe_path = index_addon_info.exe_path;
        QString argument = index_addon_info.call_paramer;
        QStringList argumentlist;
        argumentlist << argument;
        process.startDetached(exe_path,argumentlist);
    }
}

void MainController::update_person_name_candidate_index(int index)
{
    mCandidateWindow->SetPersonNameCandidatesIndex(index);
    qDebug() << "name candidates index:" << index;
}

void MainController::OnUsingEnglishPunctuationInChineseMode(int enabled)
{
    mToolBarModel->set_using_english_punctuation_in_chinese_mode(enabled);
    if (enabled && mToolBarModel->cn() && mToolBarModel->cnMark())
    {
        mToolBarModel->setCnMark(false);
        mAgent->TriggerProperty("/Fcitx/punc");
    }
    else if (!enabled && mToolBarModel->cn() && !mToolBarModel->cnMark())
    {
        mToolBarModel->setCnMark(true);
        mAgent->TriggerProperty("/Fcitx/punc");
    }
    mToolbarWindow->RefreshWindow();
}

void MainController::OnHzOptionChanged(int option)
{
    bool is_full_symbol = !(option & HZ_SYMBOL_HALFSHAPE);
    bool current_full_width = mToolBarModel->fullwidth();
    if(is_full_symbol != current_full_width)
    {
        changeFullWidth(is_full_symbol);
    }
}

// 获得中文标点还是英文标点
void MainController::GotPunchFromEngine(const bool is_cn)
{
    mToolBarModel->setCnMark(is_cn);
    mToolbarWindow->RefreshWindow();
}

// 从引擎获得全部状态
void MainController::GotStateFromEngine(const int state)
{
    mToolBarModel->setCn(state & CHINESE);
    mToolBarModel->setTrad(state & TRAD);
    mToolBarModel->setWb(state & WUBI);
    mToolbarWindow->RefreshWindow();

    mModel->setWb(state & WUBI);
    int pinyin_mode = (state & QUAN_PIN) ? 0 : 1;
    mModel->setPinyinMode(pinyin_mode);
}

// 定期询问引擎是否还在使用我们的输入法
void MainController::GotInUseFromEngine(const int use)
{
    mEngineNotResponse = false;
    bool flag = use;
    if(!flag)
    {
       DoShowUI(false);
    }
    else
    {
        RefreshTheWindowUI();
    }

}

// 是否显示红点提示
bool MainController::HasUpdate()
{
    return mHasUpdate;
}

void MainController::SetHasUpdate(const bool flag)
{
    mHasUpdate = flag;
    if(flag)
        mToolbarWindow->RefreshWindow();
}

void MainController::ChangeToEnglishMode()
{
    mAgent->ChangeToNextIm();
}

void MainController::OnSetHypyEnable(const bool enable)
{
    QString os_type = GetOSType();
    if(os_type.compare("nfschina") == 0)
    {
        qDebug() << "leex: 重建工具栏";
        if (mToolbarWindow && enable)
        {
            delete mToolbarWindow;
            mToolbarWindow = NULL;
            CreateToolBarView();
        }
    }
}

void MainController::RefreshTheWindowUI()
{
    if((!mToolbarWindow->isVisible()) && !mHideTray)
    {
        mToolbarWindow->ToShow();
    }
    if(!mSystemTray->isVisible())
    {
        mSystemTray->show();
    }

}

void MainController::ReloadLoginState()
{
    mToolbarWindow->RefreshWindow();
}

int MainController::GetPinyinMode()
{
    return mModel->pinyinMode();
}

void MainController::ChangeCandiatesExpandFlag(const int flag)
{
    mModel->setCandidatesExpandFlag(flag);
    mCandidateWindow->RefreshWindow();
}

void MainController::OnTabKeyclicked()
{
}

void MainController::OnHiddenToolbar()
{

    int hide_flag;
    ConfigManager::Instance()->ReloadUserConfig();
    ConfigManager::Instance()->GetIntConfigItem("hide_tray",hide_flag);
    ConfigManager::Instance()->SetIntConfigItem("hide_tray",!hide_flag);

    LoadConfig();

    bool isVisible = mToolbarWindow->isVisible();
    if(isVisible)
    {
        mToolbarWindow->setVisible(!isVisible);
    }
    else
    {
        mToolbarWindow->ToShow();
    }
}

QString MainController::GetOSType()
{
    QString os_release_path = "/etc/os-release";
    QFile file(os_release_path);\
    if(file.open(QIODevice::ReadOnly))
    {
        QString file_content = file.readAll();
        file.close();
        QRegExp kylin_reg(".*kylin.*");
        QRegExp uos_reg(".*uos.*");
        QRegExp nfs_reg(".*nfsdesktop.*");
        kylin_reg.setCaseSensitivity(Qt::CaseInsensitive);
        uos_reg.setCaseSensitivity(Qt::CaseInsensitive);
        nfs_reg.setCaseSensitivity(Qt::CaseInsensitive);
        if(kylin_reg.exactMatch(file_content)){
             return "kylin";
        }
        else if(uos_reg.exactMatch(file_content))
        {
            return "uos";
        }
        else if(nfs_reg.exactMatch(file_content)){
            return "nfschina";
        }
        else
        {
            return "";
        }
    }
    else
    {
        return "";
    }
}

void MainController::slot_fix_target_addon_on_toolbar(QString addon_name)
{
    int toolbar_config = Config::Instance()->GetToolBarConfig();
    if(addon_name == "OCR")
    {
        toolbar_config |= USE_OCR_ADDON;
    }
    else if(addon_name == "AsrInput")
    {
        toolbar_config |= USE_VOICE_ADDON;
    }
    Config::Instance()->SetToolBarConfig(toolbar_config);
    Config::Instance()->SaveConfig();
    WindowConfig::Instance()->LoadConfig();
    mToolbarWindow->RefreshWindow();
    mToolbarWindow->CheckWidgetPos();
}
