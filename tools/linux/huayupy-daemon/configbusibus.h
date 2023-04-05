#ifndef CONFIGBUSIBUS_H
#define CONFIGBUSIBUS_H

#include <ibus.h>
#include <QString>
#include <QDebug>

class ConfigBusIBus : public ConfigBus
{
public:
    virtual ~ConfigBusIBus() {}

    virtual bool init()
    {
        ibus_init();
        IBusBus *bus = ibus_bus_new();
        m_config = ibus_config_new(ibus_bus_get_connection(bus), NULL, NULL);
        return true;
    }

    virtual void uninit() {}

    virtual void valueChanged(const QString& name, int value)
    {
        qDebug() << "name: " << name << ", value: " << value;

        qDebug() << name.toStdString().c_str();

        GVariant *gvalue = g_variant_new_int32(value);
        if (!ibus_config_set_value(m_config, "engine/huayupy", name.toStdString().c_str(), gvalue))
        {
            qDebug() << "ibus set value failed!";
        }
    }

    virtual void valueChanged(const QString& name, const QString& value)
    {
        GVariant *gvalue = g_variant_new_string(value.toStdString().c_str());
        if (!ibus_config_set_value(m_config, "engine/huayupy", name.toStdString().c_str(), gvalue))
        {
            qDebug() << "ibus set value failed!";
        }
    }

private:
    IBusConfig *m_config;
};

ConfigBus *ConfigBus::instance()
{
    if (!s_instance)
    {
        s_instance = new ConfigBusIBus();
    }
    return s_instance;
}

#endif // CONFIGBUSIBUS_H
