#ifndef DEFINE_H
#define DEFINE_H

enum PackageType
{
   RPM = 0,
   DEB = 1
};

enum UpdateTipPosition
{
    CENTER = 0,
    BOTTOM_RIGHT_CORNER
};

#define NOTFOUND "0"

#define REASON_IS_UPDATING "0"
#define REASON_IS_UPTO_DATE "1"
#define REASON_IS_ERROR "2"
#define HAS_NEW_VERSION "3"

#define MSG_FROM_UPDATER_FINISH "0"
#define MSG_FROM_UPDATER_OK "1"

#define NOTFOUND "0"
#define CHECK_UPDATE "1"
#define UPDATE_UNISPY "3"
#define CHECK_UPDATE_FCITX "1"
#define CHECK_UPDATE_IBUS "2"

#define CHECK_FROM_SETTING "0"
#define CHECK_FROM_AUTO "1"

#endif // DEFINE_H
