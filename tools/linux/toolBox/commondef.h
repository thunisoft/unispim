#ifndef COMMONDEF_H
#define COMMONDEF_H
#include <QString>

struct TOOL_ADDON_INFO
{
    QString addon_id;
    QString addon_name;
    QString addon_displayname;
    QString addon_describe;
    QString icon_path;
    QString publisher;
    QString author;
    QString client_version;
    QString addon_version;
    QString autor_name;
    QString home_page;
    QString client_page;
    QString category;
    QString exe_path;
    QString call_paramer;
    QString short_cut;
    QString addon_status;
    int addon_index;
    inline bool operator ==(const TOOL_ADDON_INFO& input_info)
    {
        return ((this->addon_id == input_info.addon_id)&&(this->addon_name == input_info.addon_name));
    }
};

struct TOOL_ADDON_INDEX_INFO
{
     int addon_index;   //插件的索引信息用来标记显示位置
     QString addo_id;   //插件的ID
     QString addon_name;//插件的名称
     QString addon_version;//插件的版本号
     QString addon_status; //插件的状态(禁用,还是开启)
};

struct SEVER_ADDON_INFO
{
    QString addon_name;
    QString addon_version;
    QString addon_download_url;
    QString file_md5;
    QString addon_status;
    QString file_name;
};

enum UPDATE_TYPE
{
    AutoUpdate = 0,
    ManuUpdate = 1,

};


#endif // COMMONDEF_H
