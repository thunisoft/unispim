#ifndef HOTKEYMANAGER_H
#define HOTKEYMANAGER_H
#ifdef _WIN32
#include <QObject>
#include <QHotkey>
#include <QVector>

class HotkeyManager : public  QObject
{
public:
    static HotkeyManager* Instance();
private:
    HotkeyManager(QObject* parent = 0);
public:
    ~HotkeyManager();
public:
    void EnableAllHotkey();
    void DisableAllHotkey();
    void RegisterNewHotkey(QHotkey* newhotkey);

private:
    static HotkeyManager* m_instance;
    QVector<QHotkey*> m_hotkey_vector;
};
#endif
#endif // HOTKEYMANAGER_H
