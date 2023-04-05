#include "hotkeymanager.h"
#ifdef _WIN32
HotkeyManager* HotkeyManager::m_instance = nullptr;

HotkeyManager *HotkeyManager::Instance()
{
    if(m_instance == nullptr)
    {
        m_instance = new HotkeyManager();
    }
    return m_instance;
}

HotkeyManager::HotkeyManager(QObject *parent):QObject(parent)
{

}

HotkeyManager::~HotkeyManager()
{

}

void HotkeyManager::EnableAllHotkey()
{
    try
    {
        for(QHotkey* index_hotkey : m_hotkey_vector)
        {
            if(index_hotkey != nullptr)
            {
                index_hotkey->setRegistered(true);
            }
        }
    }
    catch(std::exception e)
    {
        Q_UNUSED(e);
    }
}

void HotkeyManager::DisableAllHotkey()
{
    try
    {
        for(QHotkey* index_hotkey : m_hotkey_vector)
        {
            if(index_hotkey != nullptr)
            {
                index_hotkey->setRegistered(false);
            }
        }
    }
    catch(std::exception e)
    {
        Q_UNUSED(e);
    }
}

void HotkeyManager::RegisterNewHotkey(QHotkey *newhotkey)
{
    if(!m_hotkey_vector.contains(newhotkey))
    {
        m_hotkey_vector.push_back(newhotkey);
    }
}
#endif
