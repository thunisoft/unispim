#include "editspfiledlg.h"
#include "ui_editspfile.h"
#include "customize_ui/generalstyledefiner.h"
#include "../public/utils.h"
#include "../public/configmanager.h"
#include "msgboxutils.h"
#include <QTextStream>
#include <QDir>
#include <QStringList>
#include "spschememodel.h"

#define INITIAL_COUNT 3
#define FINAL_COUNT 29
#define ZERO_FINAL_COUNT 12


EditSPFileDlg::EditSPFileDlg(QWidget *parent):
    CustomizeQWidget(parent),
    ui(new Ui::EditSPFileDialog)
{
    ui->setupUi(this);
    Init();
}

EditSPFileDlg::EditSPFileDlg(const QString& file_name, QWidget *parent):
                                                CustomizeQWidget(parent),
                                                ui(new Ui::EditSPFileDialog)
{
    ui->setupUi(this);
    Init();

    m_file_name = file_name;
    ui->sp_name_label->setText(m_file_name);
    FillSPContent();    
}


EditSPFileDlg::~EditSPFileDlg()
{
    delete ui;
}


void EditSPFileDlg::SetNameAndContent(const QString& file_name, SpSchemeModel* sp_schema_model)
{
    m_file_name = file_name;
    m_sp_schema_model = sp_schema_model;
    ui->sp_name_label->setText(m_file_name);
    FillText();
}


void EditSPFileDlg::Init()
{
    SetCustomizeUI();
    ConnectSignalToSlot();
    InitAllKeys();
}

void EditSPFileDlg::SetCustomizeUI()
{
    ui->setupUi(this);
    SetStyleSheet();
}

void EditSPFileDlg::SetStyleSheet()
{
    this->setAttribute(Qt::WA_TranslucentBackground,true);
    ui->close_pushButton->setProperty("type","closeBtn");
    ui->close_pushButton->setFocusPolicy(Qt::NoFocus);
    ui->ok_pushButton->setProperty("type","normalBtn");
    ui->ok_pushButton->setFocusPolicy(Qt::NoFocus);
    QFile file(":/feedback/basicstyle.qss");
    if(file.open(QIODevice::ReadOnly))
    {
        this->setStyleSheet(file.readAll());
        file.close();
    }
    this->setStyleSheet(QString(""
                        "#EditSPFileDialog  { "
                        " %1 "
                        " } "
                        "").arg(GeneralStyleDefiner::BorderStyle()));
}

void EditSPFileDlg::InitAllKeys()
{
    m_all_initial_key.clear();
    m_all_initial_key << "zh" << "ch" << "sh";
    m_all_final_key.clear();
    m_all_final_key << "ai"<<"an"<<"ang"<<"ao"<<"ei"<<"en"
                    <<"eng"<<"er"<<"ia"<<"ian"<<"iang"
                    <<"iao"<<"ie"<<"in"<<"ing"<<"iong"
                    <<"iu"<<"ong"<<"ou"<<"ua"<<"uai"
                    <<"uan"<<"uang"<<"ue"<<"ui"<<"un"<<"uo"<<"v"
                    <<"ve";

    m_all_zero_final_key.clear();
    m_all_zero_final_key << "a"<< "ai"<< "an"<< "ang"<< "ao"<< "e"<< "ei"
                         << "en"<< "eng"<< "er"<< "o"
                         << "ou";

}

void EditSPFileDlg::ConnectSignalToSlot()
{
    connect(ui->close_pushButton, SIGNAL(clicked(bool)), this, SLOT(ToClose()));
    connect(ui->ok_pushButton, SIGNAL(clicked(bool)), this, SLOT(SaveFile()));
}

void EditSPFileDlg::ToClose()
{    
    this->close();
}

//保存方案至文件
void EditSPFileDlg::SaveFile()
{
    if(!Valid())
    {
        MsgBoxUtils::NoticeMsgBox(ShowErrorInfo(), QString::fromLocal8Bit("提示"), false);
        return;
    }

    QString path = ConfigManager::Instance()->GetSpSchemeDir() + m_file_name + ".ini";
    QFile sp(path);
    QTextStream out(&sp);
    sp.open(QIODevice::WriteOnly);
    out << QString::fromLocal8Bit(";华宇双拼自定义方案!请不要编辑本行\n");
    out << QString("[Initial]\n");
    for(auto it = m_initial.begin(); it != m_initial.end(); it ++)
    {
        out << QString(it->key) << QString("=") << QString(it->value) << "\n";
    }
    out << QString("[Final]\n");
    for(auto it = m_final.begin(); it != m_final.end(); it ++)
    {
        out << QString(it->key) << QString("=") << QString(it->value) << "\n";
    }
    out << QString("[ZeroFinal]\n");
    out << QString::fromLocal8Bit(";韵母单独成音时，键位对应关系\n");
    for(auto it = m_zero_final.begin(); it != m_zero_final.end(); it ++)
    {
        out << QString(it->key) << QString("=") << QString(it->value) << "\n";
    }
    out.flush();
    sp.close();

    emit Saved();

    this->close();
}

void EditSPFileDlg::FillText()
{
    this->ui->sp_name_label->setText(m_file_name);
    ResetKeyContainer();
    if(m_sp_schema_model->m_configurable_initial_map.isEmpty())
    {
        m_sp_content = m_sp_schema_model->GetSPText();
    }
    else
    {
        m_sp_content = MakeShowText();
    }
    this->ui->sp_content_textEdit->setText(m_sp_content);
}

void EditSPFileDlg::InitKeyContainer()
{
    InsertKeyToContainer(m_sp_schema_model->m_configurable_initial_map, KEY_INITIAL);
    InsertKeyToContainer(m_sp_schema_model->m_configurable_final_map, KEY_FINAL);
    InsertKeyToContainer(m_sp_schema_model->m_zero_final_map, KEY_ZERO_FINAL);
}

void EditSPFileDlg::InitKeyContainerWithOutModel()
{
    for(int i = 0; i < INITIAL_COUNT; i++)
    {
        KeyMap key_map(KEY_INITIAL, m_all_initial_key[i].toStdString().c_str());
        m_initial.push_back(key_map);
    }

    for(int i = 0; i < FINAL_COUNT; i++)
    {
        KeyMap key_map(KEY_FINAL, m_all_final_key[i].toStdString().c_str());
        m_final.push_back(key_map);
    }

    for(int i = 0; i < ZERO_FINAL_COUNT; i++)
    {
        KeyMap key_map(KEY_ZERO_FINAL, m_all_zero_final_key[i].toStdString().c_str());
        m_zero_final.push_back(key_map);
    }

}

void EditSPFileDlg::ResetKeyContainer()
{
    m_initial.clear();
    m_final.clear();
    m_zero_final.clear();
    if(m_sp_schema_model->m_configurable_initial_map.isEmpty())
    {
        InitKeyContainerWithOutModel();
    }
    else
    {
        InitKeyContainer();
    }
}

bool EditSPFileDlg::Valid()
{
    ParseProjectToKeyContainer(m_orignal_content);

    bool has_faild = false;
    unsigned int success_count = 0;
    m_invalid_keys.clear();
    KEY_TYPE type;
    QString content = ui->sp_content_textEdit->toPlainText();
    QStringList contents = content.split('\n');
    for(int i = 0; i < contents.size(); i++)
    {
        QString line = contents.at(i);
        if(line.startsWith(";"))
            continue;
        else if(line.compare("[Initial]") == 0)
        {
            type = KEY_INITIAL;
        }
        else if(line.compare("[Final]") == 0)
        {
            type = KEY_FINAL;
        }
        else if(line.compare("[ZeroFinal]") == 0)
        {
            type = KEY_ZERO_FINAL;
        }
        else
        {
            QStringList key_value_ = line.split("=");
            if(key_value_.size() == 2)
            {
                if(!FillKeyMap(key_value_.at(0), key_value_.at(1), type))
                    has_faild = true;
                else
                    success_count++;
            }
        }
    }


    if(has_faild || success_count < m_initial.size() + m_final.size() + m_zero_final.size())
    {
        return false;
    }    

    auto it1 = m_initial.begin();
    for(; it1 != m_initial.end(); it1 ++)
    {
        if(it1->info != OK)
            return false;
    }

    auto it2 = m_final.begin();
    for(; it2 != m_final.end(); it2 ++)
    {
        if(it2->info != OK)
            return false;
    }

    auto it3 = m_zero_final.begin();
    for(; it3 != m_zero_final.end(); it3 ++)
    {
        if(it3->info != OK)
            return false;
    }
    return true;
}
/*
 * 键值，在参考方案里没有，但配置正确，是否应该保存到文件？
 * 当前的做法为放入
 * */
#define IS_VALID_CHAR(c) ((QChar('A') <= c && c <= QChar('Z')) || (QChar('a') <= c && c <= QChar('z')) || c == QChar(';'))
bool EditSPFileDlg::FillKeyMap(const QString& key, const QString& value, KEY_TYPE key_type)
{

    switch(key_type)
    {
        case KEY_INITIAL:
        {
            auto key_map = m_initial.begin();
            for(; key_map!= m_initial.end(); key_map++)
            {
                if(key.compare(key_map->key) == 0)
                {
                    if(value.isEmpty())
                    {
                        key_map->info = EMPTY;
                    }
                    else if(value.length() == 1 && IS_VALID_CHAR(value.at(0)))
                    {
                        memcpy(key_map->value, value.toStdString().c_str(), value.length());
                        if(key_map->info != EMPTY)
                            key_map->info = REPEAT;
                        else
                        {
                             key_map->info = OK;
                             return true;
                        }
                    }
                    else
                        key_map->info = ERR_KEY;

                    return false;
                }
            }
            if(m_all_initial_key.contains(key))
            {
                KeyMap key_map(KEY_INITIAL, key.toStdString().c_str());
                if(value.isEmpty())
                {
                    key_map.info = EMPTY;
                }
                else if(value.length() == 1 && IS_VALID_CHAR(value.at(0)))
                {
                    memcpy(key_map.value, value.toStdString().c_str(), value.length());
                    key_map.info = OK;
                }
                else
                    key_map.info = ERR_KEY;

                m_initial.push_back(key_map);

                if(key_map.info == OK)
                    return true;
            }
            else
            {
                m_invalid_keys.append(key).append(",");
            }
            break;
        }
        case KEY_FINAL:
        {
            auto key_map = m_final.begin();
            for(; key_map!= m_final.end(); key_map++)
            {
                if(key.compare(key_map->key) == 0)
                {
                    if(value.length() == 1 && IS_VALID_CHAR(value.at(0)))
                    {
                        memcpy(key_map->value, value.toStdString().c_str(), value.length());
                        if(key_map->info != EMPTY)
                            key_map->info = REPEAT;
                        else
                        {
                             key_map->info = OK;
                             return true;
                        }
                    }
                    else
                        key_map->info = ERR_KEY;

                    return false;
                }  
            }
            if(m_all_final_key.contains(key))
            {
                KeyMap key_map(KEY_FINAL, key.toStdString().c_str());
                if(value.isEmpty())
                {
                    key_map.info = EMPTY;
                }
                else if(value.length() == 1 && IS_VALID_CHAR(value.at(0)))
                {
                    memcpy(key_map.value, value.toStdString().c_str(), value.length());
                    key_map.info = OK;
                }
                else
                    key_map.info = ERR_KEY;

                m_final.push_back(key_map);
                if(key_map.info == OK)
                    return true;
            }
            else
            {
                m_invalid_keys.append(key).append(",");
            }

        break;
        }
        case KEY_ZERO_FINAL:
        {
            auto key_map = m_zero_final.begin();
            for(; key_map!= m_zero_final.end(); key_map++)
            {
                if(key.compare(key_map->key) == 0)
                {
                    if(value.length() == 2 &&
                            IS_VALID_CHAR(value.at(0)) &&
                                          IS_VALID_CHAR(value.at(1)))
                    {
                        memcpy(key_map->value, value.toStdString().c_str(), value.length());
                        if(key_map->info != EMPTY)
                            key_map->info = REPEAT;
                        else
                            key_map->info = OK;
                        return true;
                    }
                    else
                    {
                        key_map->info = ERR_KEY;
                    }

                    return false;
                }
            }
            if(m_all_zero_final_key.contains(key))
            {
                KeyMap key_map(KEY_ZERO_FINAL, key.toStdString().c_str());
                if(value.isEmpty())
                {
                    key_map.info = EMPTY;
                }
                else if(value.length() == 2 && IS_VALID_CHAR(value.at(0)) &&
                        IS_VALID_CHAR(value.at(1)))
                {
                    memcpy(key_map.value, value.toStdString().c_str(), value.length());
                    key_map.info = OK;
                }
                else
                    key_map.info = ERR_KEY;

                m_zero_final.push_back(key_map);
                if(key_map.info == OK)
                    return true;
            }
            else
                m_invalid_keys.append(key).append(",");
            break;
        }
    default:
        break;
    }
    return false;
}

void EditSPFileDlg::InsertKeyToContainer(const QMap<QString,QString>& key_values, KEY_TYPE key_type)
{
    if(key_type == KEY_FINAL)
    {
        auto it = key_values.begin();
        for(; it != key_values.end(); it ++)
        {
            if(it.key().isEmpty() || it.value().isEmpty())
                continue;
            KeyMap key_map(KEY_FINAL, it.key().toStdString().c_str(), it.value().toStdString().c_str());
            m_final.push_back(key_map);
        }
    }
    else if(key_type == KEY_INITIAL)
    {
        auto it = key_values.begin();
        for(; it != key_values.end(); it ++)
        {
            if(it.key().isEmpty() || it.value().isEmpty())
                continue;
            KeyMap key_map(KEY_INITIAL, it.key().toStdString().c_str(), it.value().toStdString().c_str());
            m_initial.push_back(key_map);
        }
    }
    else if(key_type == KEY_ZERO_FINAL)
    {
        auto it = key_values.begin();
        for(; it != key_values.end(); it ++)
        {
            if(it.key().isEmpty() || it.value().isEmpty())
                continue;
            KeyMap key_map(KEY_ZERO_FINAL, it.key().toStdString().c_str(), it.value().toStdString().c_str());
            m_zero_final.push_back(key_map);
        }
    }
}

QString EditSPFileDlg::ShowErrorInfo()
{
    QString error_info;
    QString empty_keys;
    QString error_keys;
    QString repeat_keys;
    int empty_keys_count = 0;
    int error_keys_count = 0;
    int repeat_keys_count = 0;
    auto GetErroInfo = [&](vector<KeyMap>& keymap){
        for(auto it = keymap.begin(); it != keymap.end(); it ++)
        {
            switch(it->info)
            {
            case EMPTY:
            {   empty_keys_count++;
                if(empty_keys.isEmpty())
                    empty_keys.append(it->key);
                else
                {
                    empty_keys.append(",");
                    if(empty_keys_count % 11 == 0)
                        empty_keys.append("\n");
                    empty_keys.append(it->key);
                }

                break;
            }
            case ERR_KEY:
            {
                error_keys_count ++;
                if(error_keys.isEmpty())
                    error_keys.append(it->key);
                else
                {
                    error_keys.append(",");
                    if(error_keys_count % 11 == 0)
                        error_keys.append("\n");
                    error_keys.append(it->key);
                }
                break;
            }
            case REPEAT:
            {
                repeat_keys_count++;
                if(repeat_keys.isEmpty())
                    repeat_keys.append(it->key);
                else
                {
                    repeat_keys.append(",");
                    if(repeat_keys_count % 11 == 0)
                        repeat_keys.append("\n");
                    repeat_keys.append(it->key);
                }
                break;
            }
            default:
                break;

            }
        }
    };

    GetErroInfo(m_initial);
    GetErroInfo(m_final);
    GetErroInfo(m_zero_final);

    if(!empty_keys.isEmpty())
        error_info.append(empty_keys).append(QString::fromLocal8Bit(": 键值未配置！\n"));
    if(!error_keys.isEmpty())
        error_info.append(error_keys).append(QString::fromLocal8Bit(": 键值配置错误！\n"));
    if(!repeat_keys.isEmpty())
        error_info.append(repeat_keys).append(QString::fromLocal8Bit(": 重复配置！\n"));
    if(!m_invalid_keys.isEmpty())
        error_info.append(m_invalid_keys.remove(m_invalid_keys.length() - 1, 1)).append(": 非法！\n");
    return error_info;
}

QString EditSPFileDlg::MakeShowText()
{
    QString content;
    content.append(";华宇双拼自定义方案!请不要编辑本行\n");
    content.append("[Initial]").append("\n");
    auto it1 = m_initial.begin();
    for(; it1 != m_initial.end(); it1++)
    {
        content.append(it1->key).append("=").append(it1->value).append("\n");
    }
    content.append("[Final]").append("\n");
    auto it2 = m_final.begin();
    for(; it2 != m_final.end(); it2++)
    {
        content.append(it2->key).append("=").append(it2->value).append("\n");
    }
    content.append("[ZeroFinal]").append("\n");
    content.append(";韵母单独成音时，键位对应关系\n");
    auto it3 = m_zero_final.begin();
    for(; it3 != m_zero_final.end(); it3++)
    {
        content.append(it3->key).append("=").append(it3->value).append("\n");
    }
    return content;
}

void EditSPFileDlg::FillSPContent()
{
    QString sp_file_path = ConfigManager::Instance()->GetSpSchemeDir() + m_file_name + ".ini";
    QFile sp(sp_file_path);
    if(!sp.exists())
    {
        MsgBoxUtils::NoticeMsgBox(QString::fromLocal8Bit("双拼文件不存在！"), QString::fromLocal8Bit("提示"), false);
        return;
    }

    sp.open(QIODevice::ReadOnly);
    ui->sp_content_textEdit->setText(sp.readAll());
    sp.close();

    m_orignal_content = ui->sp_content_textEdit->toPlainText();

}

void EditSPFileDlg::ParseProjectToKeyContainer(const QString& content_)
{
    m_initial.clear();
    m_final.clear();
    m_zero_final.clear();

    KEY_TYPE type;
    QString content = content_;
    QStringList contents = content.split('\n');
    for(int i = 0; i < contents.size(); i++)
    {
        QString line = contents.at(i);
        if(line.startsWith(";"))
            continue;
        else if(line.compare("[Initial]") == 0)
        {
            type = KEY_INITIAL;
        }
        else if(line.compare("[Final]") == 0)
        {
            type = KEY_FINAL;
        }
        else if(line.compare("[ZeroFinal]") == 0)
        {
            type = KEY_ZERO_FINAL;
        }
        else
        {
            QStringList key_value_ = line.split("=");
            if(key_value_.size() == 2)
            {
                KeyMap key_map(type, key_value_[0].toStdString().c_str(), key_value_[1].toStdString().c_str());
                if(type == KEY_INITIAL)
                {
                    m_initial.push_back(key_map);
                }
                else if(type ==  KEY_FINAL)
                {
                    m_final.push_back(key_map);
                }
                else
                {
                    m_zero_final.push_back(key_map);
                }
            }
        }
    }
}
