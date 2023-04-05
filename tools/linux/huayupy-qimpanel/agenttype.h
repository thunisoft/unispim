#ifndef AGENTTYPE_H
#define AGENTTYPE_H

#include <QString>
#include <QList>
#include <QVariant>

struct TextAttribute
{
    enum Type
    {
        None,
        Decorate,
        Foreground,
        Background
    };
    Type type;
    int start;
    int length;
    int value;
};

struct KimpanelProperty
{
    enum State
    {
        None = 0,
        Active = 1,
        Visible = (1 << 1)
    };
    Q_DECLARE_FLAGS(States, State)

    KimpanelProperty() { }

    KimpanelProperty(QString key, QString label, QString icon, QString tip, int state, QString menu)
    {
        this->key = key;
        this->label = label;
        this->tip = tip;
        this->icon = icon;
        this->state = (State) state;
        this->menu = menu;
    }

    QString key;
    QString label;
    QString icon;
    QString tip;
    States state;
    QString menu;

    bool operator==(KimpanelProperty& input) const
    {
        return (key == input.key &&
                label == input.label &&
                icon == input.icon &&
                tip == input.tip &&
                menu == input.menu &&
                state == input.state);
    }

    QVariantMap toMap() const
    {
        QVariantMap map;
        map["key"] = key;
        map["label"] = label;
        map["icon"] = icon;
        map["tip"] = tip;
        map["state"] = (int) state;
        map["menu"] = menu;
        return map;
    }
};
Q_DECLARE_OPERATORS_FOR_FLAGS(KimpanelProperty::States)

struct KimpanelLookupTable
{
    struct Entry
    {
        QString label;  //存放标号字符串，比如“1.”，“2.”，“3.”，“4.”，“5.”
        QString text;   //存放候选词字符串，比如“啊”，“阿”，“吖”，“呵”，“腌”
        QList<TextAttribute> attr;
    };

    QList<Entry> entries;
    bool has_prev;
    bool has_next;
};

#endif // AGENTTYPE_H
