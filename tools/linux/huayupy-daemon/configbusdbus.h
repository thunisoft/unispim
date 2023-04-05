#ifndef CONFIGBUSDBUS_H
#define CONFIGBUSDBUS_H

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDebug>

#define UNISPY_SETTING_INTERFACE "com.thunisoft.huayupy.setting"

class ConfigBusDBus : public ConfigBus
{
public:
    virtual ~ConfigBusDBus() {}
    virtual bool init()
    {
        if (!QDBusConnection::sessionBus().isConnected())
        {
            qWarning("Cannot connect to the D-Bus session bus.\n"
                     "Please check your system settings and try again.\n");
            return false;
        }

        return true;
    }

    virtual void uninit() {}

    virtual void valueChanged(const QString& name, int value)
    {
        sendDBusSignal(name, value);
    }

    virtual void valueChanged(const QString& name, const QString& value)
    {
        sendDBusSignal(name, value);
    }

private:
    template <typename ValueType>
    void sendDBusSignal(const QString &name, ValueType value)
    {
        qDebug() << "name: " << name << ", value: " << value;

        QDBusMessage msg = QDBusMessage::createSignal("/", UNISPY_SETTING_INTERFACE, name);
        msg << value;
        QDBusConnection::sessionBus().send(msg);
    }
};

ConfigBus *ConfigBus::instance()
{
    if (!s_instance)
    {
        s_instance = new ConfigBusDBus();
    }
    return s_instance;
}

#endif // CONFIGBUSDBUS_H
