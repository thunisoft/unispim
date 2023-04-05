// 原与 QML 交互的数据部分，现在仍需要

#include "toolbarmodel.h"
#include <QDesktopWidget>
#include <QApplication>
#include "../public/utils.h"
#include <QSettings>
#include "../public/config.h"

ToolBarModel::ToolBarModel(QObject *parent)
    : QObject(parent), mLeft(0), mTop(0)
    , mIsCn(true), mIsTrad(false), mIsFullWidth(false)
    , mIsSoftKbdShow(false), mIsCnMark(true),mIsWb(false),mToolbarConfig(127)
{
    Init();
}

void ToolBarModel::Init()
{
    QDesktopWidget* pDesktopWidget = QApplication::desktop();
    //获取可用桌面大小
    QRect deskRect = pDesktopWidget->availableGeometry();
    int left = deskRect.right() - 192 - 20;
    int top = deskRect.bottom() - 31 - 100;
    mLeft = left;
    mTop = top;
}

void ToolBarModel::setLeft(int left)
{
    mLeft = left;
}

int ToolBarModel::left()
{
//    QRect deskRect = QApplication::desktop()->availableGeometry();
//    return deskRect.right() - 192 - 20;
    return mLeft;
}

void ToolBarModel::setTop(int top)
{
    mTop = top;
}

int ToolBarModel::top()
{
//    QRect deskRect = QApplication::desktop()->availableGeometry();
//    return deskRect.bottom() - 31 - 100;
    return mTop;
}

void ToolBarModel::setCn(bool isCn)
{
    mIsCn = isCn;
    emit cnChanged();
}

bool ToolBarModel::cn()
{
    return mIsCn;
}

void ToolBarModel::setWb(bool isWb)
{
    mIsWb = isWb;
    emit wbChanged();
}

bool ToolBarModel::wb()
{
    return mIsWb;
}

void ToolBarModel::setTrad(bool isTrad)
{
    mIsTrad = isTrad;
    emit tradChanged();
}

bool ToolBarModel::trad()
{
    return mIsTrad;
}

void ToolBarModel::setFullWidth(bool isFullWidth)
{
    mIsFullWidth = isFullWidth;
    emit fullwidthChanged();
}

bool ToolBarModel::fullwidth()
{
    return mIsFullWidth;
}

void ToolBarModel::setCnMark(bool isCnMark)
{
    mIsCnMark = isCnMark;
    //emit cnMarkChanged();
}

bool ToolBarModel::cnMark()
{
    return mIsCnMark;
}

void ToolBarModel::setSoftKbd(bool show)
{
    mIsSoftKbdShow = show;
}

bool ToolBarModel::softKbd()
{
    return mIsSoftKbdShow;
}

int ToolBarModel::bottomLimit()
{
    QRect deskRect = QApplication::desktop()->availableGeometry();
    mBottomLimit = deskRect.bottom() - 31;
    return mBottomLimit;
}

int ToolBarModel::rightLimit()
{
    int startpos = 42;
    for(int index=0; index<7; ++index)
    {
        if(mToolbarConfig & (1<<index))
        {
            startpos += 25;
        }
    }

    QRect deskRect = QApplication::desktop()->availableGeometry();
    mRightLimit= deskRect.right() - startpos - 10;
    return mRightLimit;
}

void ToolBarModel::toresetWindow()
{
    emit resetWindow();
}

void ToolBarModel::setToolbarConfig(int toolConfig)
{
    mToolbarConfig = toolConfig;
    emit toolbarChanged();
    emit resetWindow();
}

int ToolBarModel::toolbarConfig()
{
    return mToolbarConfig;
}

void ToolBarModel::loadConfig()
{
    QString config_file_path = Config::Instance()->configFilePath();
    QSettings config(config_file_path, QSettings::IniFormat);
    int startup_with_english_input = config.value("basic/startup_with_english_input", 0).toInt();
    setCn(startup_with_english_input != 1);
    m_using_english_punctuation_in_chinese_mode =
            config.value("advanced/using_english_punctuation_in_chinese_mode", 0).toInt();
    if (startup_with_english_input != 1 && using_english_punctuation_in_chinese_mode())
    {
        setCnMark(false);
    }
    else
    {
        setCnMark(startup_with_english_input != 1);
    }

    int hz_option = config.value("basic/hz_option", HZ_SYMBOL_CHINESE).toInt();
    setFullWidth(!(hz_option & HZ_SYMBOL_HALFSHAPE));
    int hz_output_mode = config.value("basic/hz_output_mode", HZ_OUTPUT_SIMPLIFIED).toInt();
    setTrad(hz_output_mode & HZ_OUTPUT_TRADITIONAL);

    int default_chinese_input_mode = config.value("basic/default_chinese_input_mode",
                                                DEFAULT_CHINESE_INPUT_MODE_PINYIN).toInt();
    setWb(default_chinese_input_mode == DEFAULT_CHINESE_INPUT_MODE_WUBI);
}

int ToolBarModel::using_english_punctuation_in_chinese_mode()
{
    return m_using_english_punctuation_in_chinese_mode;
}

void ToolBarModel::set_using_english_punctuation_in_chinese_mode(int using_english_punctuation_in_chinese_mode)
{
    m_using_english_punctuation_in_chinese_mode = using_english_punctuation_in_chinese_mode;
}
