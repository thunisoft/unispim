#ifndef EDITSPFILEDLG_H
#define EDITSPFILEDLG_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QFile>
#include "customize_ui/customize_qwidget.h"
#include <vector>
#include <QStringList>

#define KEY_VALUE_LEN 6
using namespace std;

enum ERROR_INFO
{
    OK,
    ERR_KEY, //error key
    EMPTY,    //don't config the key
    REPEAT,
};

enum KEY_TYPE
{
    KEY_INITIAL,
    KEY_FINAL,
    KEY_ZERO_FINAL,
};

typedef struct tagKeyMap
{
   KEY_TYPE type;
   char key[KEY_VALUE_LEN];
   char value[KEY_VALUE_LEN];
   ERROR_INFO info;

   tagKeyMap(KEY_TYPE type, const char* key)
   {
        this->type = type;
        memset(this->key, 0, KEY_VALUE_LEN);
        memcpy(this->key, key, strlen(key));
        memset(value, 0, KEY_VALUE_LEN);
        info = EMPTY;
   }
   tagKeyMap(KEY_TYPE type, const char* key, const char* value)
   {
        this->type = type;
        memset(this->key, 0, KEY_VALUE_LEN);
        memcpy(this->key, key, strlen(key) > KEY_VALUE_LEN - 1 ? KEY_VALUE_LEN - 1 : strlen(key));
        memset(this->value, 0, KEY_VALUE_LEN);
        memcpy(this->value, value, strlen(value) > KEY_VALUE_LEN - 1 ? KEY_VALUE_LEN - 1 : strlen(value));
        info = EMPTY;
   }
}KeyMap;

namespace Ui {
    class EditSPFileDialog;
}

class SpSchemeModel;

class EditSPFileDlg : public CustomizeQWidget
{
    Q_OBJECT
public:
    EditSPFileDlg(QWidget *parent = nullptr);
    EditSPFileDlg(const QString& file_name, QWidget *parent = nullptr);
    ~EditSPFileDlg();    
    void SetNameAndContent(const QString& file_name, SpSchemeModel* sp_chema_model);
signals:
    void Saved();
private slots:
    void ToClose();
    void SaveFile();

private:
    void Init();
    void SetCustomizeUI();
    void SetStyleSheet();
    void InitAllKeys();
    void ConnectSignalToSlot();
    void FillText();
    bool Valid();
    void InitKeyContainer();
    void InitKeyContainerWithOutModel();
    void ResetKeyContainer();
    bool FillKeyMap(const QString& key, const QString& value, KEY_TYPE key_type);
    void InsertKeyToContainer(const QMap<QString,QString>& key_values, KEY_TYPE key_type);
    QString ShowErrorInfo();
    QString MakeShowText();
    void FillSPContent();
    void ParseProjectToKeyContainer(const QString& content);

private:
    Ui::EditSPFileDialog *ui;
    QString m_file_name;
    QString m_sp_content;

    vector<KeyMap> m_initial;
    vector<KeyMap> m_final;
    vector<KeyMap> m_zero_final;

    SpSchemeModel* m_sp_schema_model; //needn't delete

    QStringList m_all_initial_key;
    QStringList m_all_final_key;
    QStringList m_all_zero_final_key;

    QString m_invalid_keys;

    QString m_orignal_content; // 最开始文件的内容
};

#endif // EDITSPFILEDLG_H
