#ifndef RECENTRECORD_H
#define RECENTRECORD_H

#include <QObject>

class RecentRecord : public QObject
{
    Q_OBJECT
public:
    explicit RecentRecord(QObject *parent = 0);

};

#endif // RECENTRECORD_H
