#ifndef COMMONDEF_H
#define COMMONDEF_H
#include <QString>
#include <QMap>
#include <QVariant>

#ifdef USE_IBUS
#define DEFAULT_CANDIDATE_COUNT 8
#else
#define DEFAULT_CANDIDATE_COUNT 5
#endif

#define MAX_CANDIDATE_COUNT 9

#define INI_POSTFIX ".ini"

typedef QMap<QString, QVariant> UserInfo;

#endif // COMMONDEF_H
