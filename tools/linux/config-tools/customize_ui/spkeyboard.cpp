#include <QGridLayout>
#include "spkeyboard.h"
#include "singlekey.h"
#include "../spschememodel.h"

SPKeyBoard::SPKeyBoard(QWidget *parent) : QWidget(parent)
{
    InitWidget();
}

void SPKeyBoard::RefreshTheWidget()
{
    QMap<QString, Key_Syllable_Table_Stru> key_map = m_sp_scheme_model.GetKeySyllableMap();
    QVector<QString> all_key_vector = m_sp_scheme_model.m_all_key_vector;
    int size = all_key_vector.size();

    for(int index=0; index<size; ++index)
    {
        QString key_name = all_key_vector.at(index);
        Key_Syllable_Table_Stru key_syllable_map = key_map[key_name];

        SingleKey* key = qobject_cast<SingleKey*>(this->findChild<QWidget*>(key_name));
        key->SetConStr(key_syllable_map.initial_syllable);
        key->SetVowStr(key_syllable_map.final_syllable_Set);
    }
}

void SPKeyBoard::ResetTheWidget()
{

}

void SPKeyBoard::SetSpSchemeModel(SpSchemeModel scheme_model)
{
    m_sp_scheme_model = scheme_model;
    RefreshTheWidget();
}


void SPKeyBoard::InitWidget()
{
    //将键盘按键按顺序放到容器中去
    QGridLayout* mainLayout = new QGridLayout(this);
    SpSchemeModel* temp_model = SpSchemeModel::CreateSpSchemeMoel(false);
    m_sp_scheme_model = *temp_model;
    delete temp_model;
    QMap<QString, Key_Syllable_Table_Stru> key_map = m_sp_scheme_model.GetKeySyllableMap();
    QVector<QString> all_key_vector = m_sp_scheme_model.m_all_key_vector;
    int size = all_key_vector.size();
    mainLayout->setSpacing(3);

    for(int index=0; index<size; ++index)
    {
        QString key_name = all_key_vector.at(index);
        Key_Syllable_Table_Stru key_syllable_map = key_map[key_name];
        SingleKey* key = new SingleKey(this);

        m_key_vector.push_back(key);
        key->setObjectName(key_name);
        key->SetConStr(key_syllable_map.initial_syllable);
        key->SetVowStr(key_syllable_map.final_syllable_Set);
        key->SetKeyName(key_name);
        key->setFixedSize(50,50);
        QString string = key->property("type").toString();

        if(index<=9)
        {
            mainLayout->addWidget(key,0,index*2,1,2);
        }
        else if(index>=10 && index<=19)
        {
            mainLayout->addWidget(key,1,(index-10)*2+1,1,2);
        }
        else if(index > 19)
        {
            mainLayout->addWidget(key,2,(index-20)*2+2,1,2);
        }
        mainLayout->setVerticalSpacing(1);
    }
    this->setLayout(mainLayout);
}


