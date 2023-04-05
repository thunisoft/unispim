// DBus 通讯代理

#include "kimpanelagent.h"
#include "impanel_adaptor.h"
#include <QDBusServiceWatcher>

#include "impanel_interface.h"
#include "agenttype.h"

#define DBUS_NAME "uni_bus"
#define DBUS_NAME2 "uni_bus2"
#define FCITX_KIMPANEL_INTERFACE "org.kde.kimpanel.inputmethod"
#define HUAYUPY_KIMPANEL_INTERFACE "org.kde.huayupykimpanel.inputmethod"
#define FCITX_KIMPANEL_PATH "/kimpanel"


static KimpanelProperty String2Property(const QString &str)
{
    KimpanelProperty result;

    QStringList list = str.split(':');

    if (list.size() < 4)
        return result;

    result.key = list.at(0);
    result.label = list.at(1);
    result.icon = list.at(2);
    result.tip = list.at(3);

    if (list.size() >= 5)
        result.menu = list.at(4);

    return result;
}

static QList<TextAttribute> String2AttrList(const QString& str)
{
    QList<TextAttribute> result;
    if (str.isEmpty())
    {
        return result;
    }

    foreach(const QString& s, str.split(';'))
    {
        TextAttribute attr;
        QStringList list = s.split(':');
        if (list.size() < 4)
            continue;
        switch (list.at(0).toInt())
        {
        case 0:
            attr.type = TextAttribute::None;
            break;
        case 1:
            attr.type = TextAttribute::Decorate;
            break;
        case 2:
            attr.type = TextAttribute::Foreground;
            break;
        case 3:
            attr.type = TextAttribute::Background;
            break;
        default:
            attr.type = TextAttribute::None;
        }
        attr.start = list.at(1).toInt();
        attr.length = list.at(2).toInt();
        attr.value = list.at(3).toInt();
        result << attr;
    }
    return result;
}

static KimpanelLookupTable Args2LookupTable(const QStringList &labels, const QStringList &candis, const QStringList &attrs, bool has_prev, bool has_next)
{
    Q_ASSERT(labels.size() == candis.size());
    Q_ASSERT(labels.size() == attrs.size());

    KimpanelLookupTable result;

    for (int i = 0; i < labels.size(); i++)
    {
        KimpanelLookupTable::Entry entry;

        entry.label = labels.at(i);
        entry.text = candis.at(i);
        entry.attr = String2AttrList(attrs.at(i));

        result.entries << entry;
    }

    result.has_prev = has_prev;
    result.has_next = has_next;
    return result;
}

KimpanelAgent::KimpanelAgent(QObject *parent)
    : QObject(parent)
    , mAdaptor(new ImpanelAdaptor(this))
    , mAdaptor2(new Impanel2Adaptor(this))
    , mWatcher(new QDBusServiceWatcher(this))
{
    mWatcher->setConnection(QDBusConnection::sessionBus());
    mWatcher->setWatchMode(QDBusServiceWatcher::WatchForUnregistration);

    OpenDBus();

    connect(mWatcher, SIGNAL(serviceUnregistered(QString)), this, SLOT(ServiceUnregistered(QString)));
}

KimpanelAgent::~KimpanelAgent()
{
    QDBusConnection::disconnectFromBus(DBUS_NAME);
}

void KimpanelAgent::OpenDBus()
{
    QDBusConnection::connectToBus(QDBusConnection::SessionBus, DBUS_NAME).registerObject("/org/kde/impanel", this);
    QDBusConnection::connectToBus(QDBusConnection::SessionBus, DBUS_NAME).registerService("org.kde.impanel");
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerService("org.kde.impanel"))
    {
        qDebug() << "register service: <org.kde.impanel> failed!";
    }
#ifdef QT5
    bus.registerObject("/org/kde/impanel",    "org.kde.impanel2", this, QDBusConnection::ExportAllSlots);
#else
    bus.registerObject("/org/kde/impanel", this, QDBusConnection::ExportAllSlots);
#endif

    QDBusConnection(DBUS_NAME).connect("", "", HUAYUPY_KIMPANEL_INTERFACE,
                                       "ExecDialog", this, SLOT(ExecDialog(QString)));
    QDBusConnection(DBUS_NAME).connect("", "", HUAYUPY_KIMPANEL_INTERFACE,
                                       "ExecMenu", this, SLOT(ExecMenu(QStringList)));
    QDBusConnection(DBUS_NAME).connect("", "", HUAYUPY_KIMPANEL_INTERFACE,
                                       "RegisterProperties", this, SLOT(RegisterProperties(QStringList)));
    QDBusConnection(DBUS_NAME).connect("", "", HUAYUPY_KIMPANEL_INTERFACE,
                                       "UpdateProperty", this, SLOT(UpdateProperty(QString)));
    QDBusConnection(DBUS_NAME).connect("", "", HUAYUPY_KIMPANEL_INTERFACE,
                                       "RemoveProperty", this, SLOT(RemoveProperty(QString)));
    QDBusConnection(DBUS_NAME).connect("", "", HUAYUPY_KIMPANEL_INTERFACE,
                                       "ShowAux", this, SLOT(ShowAux(bool)));
    QDBusConnection(DBUS_NAME).connect("", "", HUAYUPY_KIMPANEL_INTERFACE,
                                       "ShowPreedit", this, SLOT(ShowPreedit(bool)));
    QDBusConnection(DBUS_NAME).connect("", "", HUAYUPY_KIMPANEL_INTERFACE,
                                       "ShowLookupTable", this, SLOT(ShowLookupTable(bool)));
    QDBusConnection(DBUS_NAME).connect("", "", HUAYUPY_KIMPANEL_INTERFACE,
                                       "UpdateLookupTable", this, SLOT(UpdateLookupTable(QStringList,
                                                                                         QStringList, QStringList, bool, bool)));
    QDBusConnection(DBUS_NAME).connect("", "", HUAYUPY_KIMPANEL_INTERFACE,
                                       "UpdateLookupTableCursor", this, SLOT(UpdateLookupTableCursor(int)));
    QDBusConnection(DBUS_NAME).connect("", "", HUAYUPY_KIMPANEL_INTERFACE,
                                       "UpdatePreeditCaret", this, SLOT(UpdatePreeditCaret(int)));
    QDBusConnection(DBUS_NAME).connect("", "", HUAYUPY_KIMPANEL_INTERFACE,
                                       "UpdatePreeditText", this, SLOT(UpdatePreeditText(QString, QString)));
    QDBusConnection(DBUS_NAME).connect("", "", HUAYUPY_KIMPANEL_INTERFACE,
                                       "UpdateAux", this, SLOT(UpdateAux(QString, QString)));
    QDBusConnection(DBUS_NAME).connect("", "", HUAYUPY_KIMPANEL_INTERFACE,
                                       "UpdateSpotLocation", this, SLOT(UpdateSpotLocation(int, int)));
    QDBusConnection(DBUS_NAME).connect("", "", HUAYUPY_KIMPANEL_INTERFACE,
                                       "UpdateScreen", this, SLOT(UpdateScreen(int)));
    QDBusConnection(DBUS_NAME).connect("", "", HUAYUPY_KIMPANEL_INTERFACE,
                                       "Enable", this, SLOT(Enable(bool)));

    QDBusConnection(DBUS_NAME).connect("", "", FCITX_KIMPANEL_INTERFACE,
                                            "ExecDialog", this, SLOT(ExecDialog(QString)));
    QDBusConnection(DBUS_NAME).connect("", "", FCITX_KIMPANEL_INTERFACE,
                                            "ExecMenu", this, SLOT(ExecMenu(QStringList)));
    QDBusConnection(DBUS_NAME).connect("", "", FCITX_KIMPANEL_INTERFACE,
                                            "RegisterProperties", this, SLOT(RegisterProperties(QStringList)));
    QDBusConnection(DBUS_NAME).connect("", "", FCITX_KIMPANEL_INTERFACE,
                                            "UpdateProperty", this, SLOT(UpdateProperty(QString)));
    QDBusConnection(DBUS_NAME).connect("", "", FCITX_KIMPANEL_INTERFACE,
                                            "RemoveProperty", this, SLOT(RemoveProperty(QString)));
    QDBusConnection(DBUS_NAME).connect("", "", FCITX_KIMPANEL_INTERFACE,
                                            "ShowAux", this, SLOT(ShowAux(bool)));
    QDBusConnection(DBUS_NAME).connect("", "", FCITX_KIMPANEL_INTERFACE,
                                            "ShowPreedit", this, SLOT(ShowPreedit(bool)));
    QDBusConnection(DBUS_NAME).connect("", "", FCITX_KIMPANEL_INTERFACE,
                                            "ShowLookupTable", this, SLOT(ShowLookupTable(bool)));
    QDBusConnection(DBUS_NAME).connect("", "", FCITX_KIMPANEL_INTERFACE,
                                            "UpdateLookupTable", this, SLOT(UpdateLookupTable(QStringList,
                                                    QStringList, QStringList, bool, bool)));
    QDBusConnection(DBUS_NAME).connect("", "", FCITX_KIMPANEL_INTERFACE,
                                            "UpdateLookupTableCursor", this, SLOT(UpdateLookupTableCursor(int)));
    QDBusConnection(DBUS_NAME).connect("", "", FCITX_KIMPANEL_INTERFACE,
                                            "UpdatePreeditCaret", this, SLOT(UpdatePreeditCaret(int)));
    QDBusConnection(DBUS_NAME).connect("", "", FCITX_KIMPANEL_INTERFACE,
                                            "UpdatePreeditText", this, SLOT(UpdatePreeditText(QString, QString)));
    QDBusConnection(DBUS_NAME).connect("", "", FCITX_KIMPANEL_INTERFACE,
                                            "UpdateAux", this, SLOT(UpdateAux(QString, QString)));
    QDBusConnection(DBUS_NAME).connect("", "", FCITX_KIMPANEL_INTERFACE,
                                            "UpdateSpotLocation", this, SLOT(UpdateSpotLocation(int, int)));
    QDBusConnection(DBUS_NAME).connect("", "", FCITX_KIMPANEL_INTERFACE,
                                            "UpdateScreen", this, SLOT(UpdateScreen(int)));
    QDBusConnection(DBUS_NAME).connect("", "", FCITX_KIMPANEL_INTERFACE,
                                            "Enable", this, SLOT(Enable(bool)));
    QDBusConnection(DBUS_NAME2).connect("", "", "org.kde.impanel2",
                                            "SetSpotRect", this, SLOT(SetSpotRect(int,int,int,int)));
    QDBusConnection(DBUS_NAME).connect("", "", "org.kde.impanel",
                                            "HiddenQimPanel", this, SLOT(HiddenQimPanel()));
    QDBusConnection(DBUS_NAME).connect("", "", "org.kde.impanel",
                                            "HiddenToolbar", this, SLOT(HiddenToolbar()));
    QDBusConnection(DBUS_NAME).connect("", "", "org.kde.impanel",
                                            "IsShowToolBar", this, SLOT(IsShowToolBar(int)));
    QDBusConnection(DBUS_NAME).connect("", "", "org.kde.impanel",
                                            "wubimode", this, SLOT(IsWubiModeEnabled(int)));
    QDBusConnection(DBUS_NAME).connect("", "", "org.kde.impanel",
                                            "enableumode", this, SLOT(IsUmodeEnabled(int)));
    QDBusConnection(DBUS_NAME).connect("", "", "org.kde.impanel",
                                            "CurrentInputMode", this, SLOT(GetInputMode(int))); //shift切换中英文接到消息
    QDBusConnection(DBUS_NAME).connect("", "", "org.kde.impanel",
                                            "ResponseInputMode", this, SLOT(ResponseInputMode(int)));
    QDBusConnection(DBUS_NAME).connect("", "", "org.kde.impanel",
                                            "ResponseEngineState", this, SLOT(ResponseEngineState(int)));
    QDBusConnection(DBUS_NAME).connect("", "", "org.kde.impanel",
                                            "ResponseHuayuInUse", this, SLOT(ResponseHuayuInUse(int)));

    QDBusConnection(DBUS_NAME).connect("", "", "org.kde.impanel",
                                            "istraditional", this, SLOT(GetIsTraditional(int)));

    QDBusConnection(DBUS_NAME).connect("", "", "org.kde.impanel",
                                            "pinyinMode", this, SLOT(SetPinyinMode(int)));
    QDBusConnection(DBUS_NAME).connect("", "", "org.kde.impanel",
                                            "CurrentExpandMode", this, SLOT(SetCurrentCandidatesExpand(int)));
}

void KimpanelAgent::CloseDBus()
{
    QDBusConnection::disconnectFromBus(DBUS_NAME);
    QDBusConnection::disconnectFromBus(DBUS_NAME2);
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.unregisterService("org.kde.impanel"))
    {
        qDebug() << "unregister service: <org.kde.impanel> failed!";
    }
#ifdef QT5
    bus.unregisterObject("/org/kde/impanel"/*, "org.kde.impanel2"*/);
#else
    bus.unregisterObject("/org/kde/impanel");
#endif
}

void KimpanelAgent::ServiceUnregistered(const QString& service)
{
    qDebug() << QString("KimpanelAgent::ServiceUnregistered(%1)").arg(service);

    if (service == mCurrentService)
    {
        mWatcher->setWatchedServices(QStringList());
        mCachedProps.clear();
        mCurrentService = "";
        emit ShowAuxSignal(false);
        emit ShowPreeditSignal(false);
        emit ShowLookupTableSignal(false);
        emit RegisterPropertiesSignal(QList<KimpanelProperty>());
    }
}

// send to kimpanel

void KimpanelAgent::Created()
{
    SendSignal("PanelCreated");
//    SendSignal("PanelCreated2");
    QDBusMessage msg = QDBusMessage::createSignal("/org/kde/impanel", "org.kde.impanel2", "PanelCreated2");
    QDBusConnection::sessionBus().send(msg);
}

// 移动光标
void KimpanelAgent::MovePreeditCaret(int pos)
{
    SendSignal("MovePreeditCaret", pos);
}

// 选择候选
void KimpanelAgent::SelectCandidate(int index)
{
    SendSignal("SelectCandidate", index);
}

// 上翻页
void KimpanelAgent::LookupTablePageUp()
{
    SendSignal("LookupTablePageUp");
}

// 下翻页
void KimpanelAgent::LookupTablePageDown()
{
    SendSignal("LookupTablePageDown");
}

// 切换 "key" 状态
void KimpanelAgent::TriggerProperty(const QString &key)
{
    SendSignal("TriggerProperty", key);
}

// 退出 QimPanel
void KimpanelAgent::Exit()
{
    SendSignal("Exit");
}

void KimpanelAgent::ReloadConfig()
{
    SendSignal("ReloadConfig");
}

void KimpanelAgent::Restart()
{
    // "/Fcitx/logo/toggle";
    SendSignal("Restart");
}

void KimpanelAgent::Configure()
{
    SendSignal("Configure");
}

void KimpanelAgent::ExitUI()
{
    QDBusMessage msg = QDBusMessage::createSignal("/org/freedesktop/DBus", "org.freedesktop.DBus", "NameOwnerChanged");
    QList<QVariant> args;
    args.append("org.kde.impanel");
    args.append(mCurrentService);
    args.append("");
    msg.setArguments(args);
    QDBusConnection::sessionBus().send(msg);
}

// 查询引擎当前状态
void KimpanelAgent::QueryEngineState()
{
   SendSignal("QueryEngineState");
}

void KimpanelAgent::SetCurrentOutputMode(const int is_traditional)
{
    QDBusMessage msg = QDBusMessage::createSignal("/org/kde/impanel", "org.kde.im", "SetTraditional");
    msg << is_traditional;
    QDBusConnection::sessionBus().send(msg);
}

void KimpanelAgent::QueryHuayuInUse()
{
    SendSignal("QueryHuayuInUse");
}

void KimpanelAgent::QueryPersonNameCandidatesIndex()
{
    QDBusMessage msg = QDBusMessage::createSignal("/", "com.thunisoft.huayupy.setting", "PersonNameCanidateIndex");
    msg << 1;
    QDBusConnection::sessionBus().send(msg);
}

void KimpanelAgent::RemoveUserCiByindex(const int ci_index)
{
    QDBusMessage msg = QDBusMessage::createSignal("/", "com.thunisoft.huayupy.setting", "remove_user_ci");
    msg << ci_index;
    QDBusConnection::sessionBus().send(msg);
}

void KimpanelAgent::ChangeInputMode(int modeFlag)
{
    QDBusMessage msg = QDBusMessage::createSignal("/org/HY/impanel", "org.kde.im", "TriggerProperty");
    msg << modeFlag;
    QDBusConnection::sessionBus().send(msg);
}

void KimpanelAgent::TogglePinyinOrWbMode(int modeFlag)
{
    QDBusMessage msg = QDBusMessage::createSignal("/", "com.thunisoft.huayupy.setting", "wubimode");
    msg << modeFlag;
    QDBusConnection::sessionBus().send(msg);
}

void KimpanelAgent::ToggleQuanOrShuangMode(int inputMode)
{
    QDBusMessage msg = QDBusMessage::createSignal("/", "com.thunisoft.huayupy.setting", "change_pinyin_mode");
    msg << inputMode;
    QDBusConnection::sessionBus().send(msg);
}

void KimpanelAgent::ChangeToNextIm()
{
    QDBusMessage msg = QDBusMessage::createSignal("/", "com.thunisoft.huayupy.setting", "change_to_next_im");
    msg << 1;
    QDBusConnection::sessionBus().send(msg);
}

void KimpanelAgent::SetCurrentCandidatesExpand(const int expand_flag)
{
    emit OnReciveSetExpandMode(expand_flag);
}

void KimpanelAgent::HiddenToolbar()
{
   emit OnHiddenToolbar();
}

// message from kimpanel

void KimpanelAgent::ExecDialog(const QString &prop)
{
    qDebug() << QString("KimpanelAgent::ExecDialog(%1)").arg(prop);

    emit ExecDialogSignal(String2Property(prop));
}

void KimpanelAgent::ExecMenu(const QStringList &entries)
{
    qDebug() << "KimpanelAgent::ExecMenu, entries: " << entries;
    QList<KimpanelProperty> list;
    foreach (const QString& entry, entries)
    {
        list.append(String2Property(entry));
    }

    emit ExecMenuSignal(list);
}

// 注册状态
void KimpanelAgent::RegisterProperties(const QStringList &props)
{
    qDebug() << "KimpanelAgent::RegisterProperties";
    const QDBusMessage& msg = message();
    qDebug() << "CurrentService: " << mCurrentService << ", receive service: " << msg.service();
    qDebug() << "props: " << props;
    if (msg.service() != mCurrentService)
    {
        mWatcher->removeWatchedService(mCurrentService);
        mCurrentService = msg.service();
        mWatcher->addWatchedService(mCurrentService);
    }

    if (mCachedProps != props)
    {
        mCachedProps = props;
        QList<KimpanelProperty> list;
        foreach (const QString& prop, props)
        {
            list.append(String2Property(prop));
        }

        emit RegisterPropertiesSignal(list);
    }
}

void KimpanelAgent::UpdateProperty(const QString &prop)
{
    qDebug() << QString("KimpanelAgent::UpdateProperty(%1)").arg(prop);
    emit UpdatePropertySignal(String2Property(prop));
}

void KimpanelAgent::RemoveProperty(const QString &prop)
{
    qDebug() << QString("KimpanelAgent::RemoveProperty(%1)").arg(prop);
}

// 显示提示 (没用)
void KimpanelAgent::ShowAux(bool toShow)
{
    qDebug() << QString("KimpanelAgent::ShowAux(%1)").arg(toShow);

    emit ShowAuxSignal(toShow);
}

// 输入框
void KimpanelAgent::ShowPreedit(bool toShow)
{
    qDebug() << QString("KimpanelAgent::ShowPreedit(%1)").arg(toShow);
    emit ShowPreeditSignal(toShow);
}

// 候选栏
void KimpanelAgent::ShowLookupTable(bool toShow)
{
    qDebug() << QString("KimpanelAgent::ShowLookupTable(%1)").arg(toShow);

    emit ShowLookupTableSignal(toShow);
}

// 更新候选栏
void KimpanelAgent::UpdateLookupTable(const QStringList &labels,
                                   const QStringList &cands,
                                   const QStringList &attrs,
                                   bool has_prev, bool has_next)
{
    qDebug() << "KimpanelAgent::UpdateLookupTable";
    qDebug() << "labels: " << labels;       // 编号
    qDebug() << "cands: " << cands;         // 候选词
    qDebug() << "attrs: " << attrs;         // 备注
    qDebug() << "prev: " << has_prev << ", next: " << has_next;     // 前后

    emit UpdateLookupTableSignal(Args2LookupTable(labels, cands, attrs, has_prev, has_next));
}

void KimpanelAgent::UpdateLookupTableCursor(int pos)
{
    qDebug() << QString("KimpanelAgent::UpdateLookupTableCursor(%1)").arg(pos);

    emit UpdateLookupTableCursorSignal(pos);
}

void KimpanelAgent::UpdatePreeditCaret(int pos)
{
    qDebug() << QString("KimpanelAgent::UpdatePreeditCaret(%1)").arg(pos);

    emit UpdatePreeditCaretSignal(pos);
}

void KimpanelAgent::UpdatePreeditText(const QString &text, const QString &attr)
{
    qDebug() << QString("KimpanelAgent::UpdatePreeditText(%1, %2)").arg(text).arg(attr);

    emit UpdatePreeditTextSignal(text, String2AttrList(attr));
}

void KimpanelAgent::UpdateAux(const QString &text, const QString &attr)
{
    qDebug() << QString("KimpanelAgent::UpdateAux(%1, %2)").arg(text).arg(attr);

    emit UpdateAuxSignal(text, String2AttrList(attr));
}

// 候选栏位置
void KimpanelAgent::UpdateSpotLocation(int x, int y)
{
    qDebug() << QString("KimpanelAgent::UpdateSpotLocation(%1, %2)").arg(x).arg(y);
    emit UpdateSpotLocationSignal(x, y);
}

void KimpanelAgent::UpdateScreen(int id)
{
    qDebug() << QString("KimpanelAgent::UpdateScreen(%1)").arg(id);
}

void KimpanelAgent::Enable(bool enable)
{
    qDebug() << QString("KimpanelAgent::Enable(%1)").arg(enable);
    emit SetHypyEnable(enable);
}

void KimpanelAgent::SetSpotRect(int x, int y, int width, int height)
{
    qDebug() << QString("KimpanelAgent::SetSpotRect(%1, %2, %3, %4)").arg(x).arg(y).arg(width).arg(height);
    emit UpdateSpotRectSignal(x, y, width, height);
}

void KimpanelAgent::SetLookupTable(const QStringList& labels, const QStringList& cands, const QStringList& attrs, bool hasPrev, bool hasNext, int cursor, int layout)
{
    qDebug() << "KimpanelAgent::SetLookupTable";
    qDebug() << "labels: " << labels;
    qDebug() << "cands: " << cands;
    qDebug() << "attrs: " << attrs;
    emit UpdateLookupTableFullSignal(Args2LookupTable(labels, cands, attrs, hasPrev, hasNext), cursor, layout);
}

void KimpanelAgent::SendSignal(const QString &name)
{
    qDebug() << "Send signal: " << name;

    QDBusMessage msg = QDBusMessage::createSignal("/org/kde/impanel", "org.kde.impanel", name);
    QDBusConnection::sessionBus().send(msg);
}

void KimpanelAgent::SendSignal(const QString &name, const QVariant &arg1)
{
    QList<QVariant> args;
    args.append(arg1);

    SendSignal(name, args);
}

void KimpanelAgent::SendSignal(const QString &name, const QList<QVariant> &args)
{
    QString info = "Send signal, name: ";
    info += name;
    info += ", args: ";
    for (int i=0; i<args.size(); ++i)
    {
        info += args[i].toString();
        info.append(" ");
    }
    //qDebug() << info;

    QDBusMessage msg = QDBusMessage::createSignal("/org/kde/impanel", "org.kde.impanel", name);
    msg.setArguments(args);
    QDBusConnection::sessionBus().send(msg);
}


void KimpanelAgent::HiddenQimPanel()
{
    qDebug() << "HiddenQimPanel()";
    emit OnRecieveSignalHiddenQimPanel();
}

void KimpanelAgent::IsShowToolBar(const int is_show_tool_bar)
{
    emit OnRecieveSignalIsShowToolBar(is_show_tool_bar);
}

void KimpanelAgent::IsWubiModeEnabled(const int is_wubi_enable)
{
    emit OnRecieveSingalToWubiMode(is_wubi_enable);
}

void KimpanelAgent::IsUmodeEnabled(const int is_umode_enable)
{
    emit OnUmodeChanged(is_umode_enable);
}

void KimpanelAgent::GetInputMode(const int input_mode)
{
    qDebug() << "GetInputMode()";
    emit OnReciveGetInputMode(input_mode);
}

void KimpanelAgent::ResponseInputMode(const int input_mode)
{
    qDebug() << "ResponseInputMode()";
    emit GotResponseInputMode(input_mode);
}

void KimpanelAgent::ResponseEngineState(const int state)
{
    qDebug() << QString("ResponseEngineState(%1)").arg(state);
    emit GotResponseEngineState(state);
}

void KimpanelAgent::ResponseHuayuInUse(const int use)
{
    qDebug() << QString("ResponseHuayuInUse(%1)").arg(use);
    emit GotResponseHuayuInUse(use);
}

void KimpanelAgent::GetIsTraditional(const int is_traditional)
{
    emit OnRecieveSignalToSetTraditional(is_traditional);
}

void KimpanelAgent::SetPinyinMode(const int pinyin_mode)
{
    qDebug() << "SetPinyinMode()";
    emit OnReciveSetPinyinMode(pinyin_mode);
}
