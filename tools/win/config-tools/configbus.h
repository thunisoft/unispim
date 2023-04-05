#ifndef CONFIGBUS_H
#define CONFIGBUS_H

#include <QString>

class ConfigBus
{
public:
    virtual ~ConfigBus() {}
    virtual bool init() = 0;
    virtual void uninit() = 0;
    virtual void valueChanged(const QString& name, int value) = 0;
    virtual void valueChanged(const QString& name, const QString& value) = 0;

    static ConfigBus *instance();

private:
    static ConfigBus *s_instance;
};


#endif // CONFIGBUS_H
