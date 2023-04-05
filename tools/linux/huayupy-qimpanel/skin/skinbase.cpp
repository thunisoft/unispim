/*
 * Copyright (C) 2013 National University of Defense Technology(NUDT) & Kylin Ltd.
 *
 * Authors:
 *  lenky gao    lenky0401@gmail.com/gaoqunkai@ubuntukylin.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDebug>
#include "skinbase.h"
#include "../mainmodel.h"

SkinBase::SkinBase()
{
    init();
}

SkinBase::~SkinBase()
{

}

void SkinBase::init()
{
    //横排配置填默认值
    mTipsImg = "";
    mInputBackImg = "";
    mCustomImg0 = "";
    mCustomImg1 = "";
    mMarginLeft = 0;
    mMarginRight = 0;
    mMarginTop = 0;
    mMarginBottom = 0;
    mBackArrowImg = "";
    mForwardArrowImg = "";
    //如果为0,那么会提示：QFont::setPointSizeF: Point size <= 0 (0.000000), must be greater than 0
    //所以设置为13好了
    mFontSize = 13;
    mCandFontSize = 0;
    mInputColor = Qt::color0;
    mIndexColor = Qt::color0;
    mFirstCandColor = Qt::color0;
    mOtherColor = Qt::color0;

    mInputStringPosX = 0;
    mInputStringPosY = 0;
    mOutputCandPosX = 0;
    mOutputCandPosY = 0;
    mBackArrowPosX = 0;
    mBackArrowPosY = 0;
    mForwardArrowPosX = 0;
    mForwardArrowPosY = 0;

    mAdjustWidth = 0;
    mAdjustHeight = 0;
    mHorizontalTileMode = "Stretch";
    mVerticalTileMode = "Stretch";

    //竖排配置填写零值，而不是默认值，因为在实际使用时，如果判断其为零值，那么会自动返回对应的横排值
    mTipsImgVertical = "";
    mInputBackImgVertical = "";
    mCustomImgVertical0 = "";
    mCustomImgVertical1 = "";
    mMarginLeftVertical = 0;
    mMarginRightVertical = 0;
    mMarginTopVertical = 0;
    mMarginBottomVertical = 0;
    mBackArrowImgVertical = "";
    mForwardArrowImgVertical = "";
    mFontSizeVertical = 0;
    mCandFontSizeVertical = 0;
    mInputColorVertical = Qt::color0;
    mIndexColorVertical = Qt::color0;
    mFirstCandColorVertical = Qt::color0;
    mOtherColorVertical = Qt::color0;

    mInputStringPosXVertical = 0;
    mInputStringPosYVertical = 0;
    mOutputCandPosXVertical = 0;
    mOutputCandPosYVertical = 0;
    mBackArrowPosXVertical = 0;
    mBackArrowPosYVertical = 0;
    mForwardArrowPosXVertical = 0;
    mForwardArrowPosYVertical = 0;

    mAdjustWidthVertical = 0;
    mAdjustHeightVertical = 0;
    mHorizontalTileModeVertical = "";
    mVerticalTileModeVertical = "";
}

bool SkinBase::loadSkin(const QString skinPath)
{
    Q_UNUSED(skinPath);
    return true;
}

QColor SkinBase::value2color(const QString& value)
{
    QStringList list = value.split(' ');
    if(list.size() < 3) {
        return Qt::color0;
    }

    int r = list.at(0).toInt();
    int g = list.at(1).toInt();
    int b = list.at(2).toInt();
    return QColor(r, g, b);
}

void SkinBase::reloadSkin()
{
    emit inputBackImgChanged();
    emit customImg0Changed();
    emit customImg1Changed();
    emit tipsImgChanged();
    emit marginLeftChanged();
    emit marginRightChanged();
    emit marginTopChanged();
    emit marginBottomChanged();
    emit backArrowImgChanged();
    emit forwardArrowImgChanged();
    emit fontSizeChanged();
    emit candFontSizeChanged();
    emit inputColorChanged();
    emit indexColorChanged();
    emit fontSizeChanged();
    emit otherColorChanged();
    emit inputStringPosXChanged();
    emit inputStringPosYChanged();
    emit outputCandPosXChanged();
    emit outputCandPosYChanged();
    emit backArrowPosXChanged();
    emit backArrowPosYChanged();
    emit forwardArrowPosXChanged();
    emit forwardArrowPosYChanged();
    emit adjustWidthChanged();
    emit adjustHeightChanged();
    emit horizontalTileModeChanged();
    emit verticalTileModeChanged();
    emit mainBackImg();
    emit mainSymbolsImgChanged();
    emit mainPinImgChanged();
}

void SkinBase::setInputBackImg(const QString inputBackImg)
{
    mInputBackImg = inputBackImg;
}

QString SkinBase::inputBackImg() const
{
    if (MainModel::self()->isHorizontal() || mInputBackImgVertical == "")
        return mInputBackImg;
    else
        return mInputBackImgVertical;
}

void SkinBase::setCustomImg0(const QString customImg)
{
    mCustomImg0 = customImg;
}

QString SkinBase::customImg0() const
{
    if (MainModel::self()->isHorizontal() || mCustomImgVertical0 == "")
        return mCustomImg0;
    else
        return mCustomImgVertical0;
}

void SkinBase::setCustomImg1(const QString customImg)
{
    mCustomImg1 = customImg;
}

QString SkinBase::customImg1() const
{
    if (MainModel::self()->isHorizontal() || mCustomImgVertical1 == "")
        return mCustomImg1;
    else
        return mCustomImgVertical1;
}

void SkinBase::setTipsImg(const QString tipsImg)
{
    mTipsImg = tipsImg;
}

QString SkinBase::tipsImg() const
{
    if (MainModel::self()->isHorizontal() || mTipsImgVertical == "")
        return mTipsImg;
    else
        return mTipsImgVertical;
}

void SkinBase::setMarginLeft(const int marginLeft)
{
    mMarginLeft = marginLeft;
}

int SkinBase::marginLeft() const
{
    if (MainModel::self()->isHorizontal() || mMarginLeftVertical == 0)
        return mMarginLeft;
    else
        return mMarginLeftVertical;
}

void SkinBase::setMarginRight(const int marginRight)
{
    mMarginRight = marginRight;
}

int SkinBase::marginRight() const
{
    if (MainModel::self()->isHorizontal() || mMarginRightVertical == 0)
        return mMarginRight;
    else
        return mMarginRightVertical;
}

void SkinBase::setMarginTop(const int marginTop)
{
    mMarginTop = marginTop;
}

int SkinBase::marginTop() const
{
    if (MainModel::self()->isHorizontal() || mMarginTopVertical == 0)
        return mMarginTop;
    else
        return mMarginTopVertical;
}

void SkinBase::setMarginBottom(const int marginBottom)
{
    mMarginBottom = marginBottom;
}

int SkinBase::marginBottom() const
{
    if (MainModel::self()->isHorizontal() || mMarginBottomVertical == 0)
        return mMarginBottom;
    else
        return mMarginBottomVertical;
}

void SkinBase::setBackArrowImg(const QString backArrowImg)
{
    mBackArrowImg = backArrowImg;
}

QString SkinBase::backArrowImg() const
{
    if (MainModel::self()->isHorizontal() || mBackArrowImgVertical == "")
        return mBackArrowImg;
    else
        return mBackArrowImgVertical;
}

void SkinBase::setForwardArrowImg(const QString forwardArrowImg)
{
    mForwardArrowImg = forwardArrowImg;
}

QString SkinBase::forwardArrowImg() const
{
    if (MainModel::self()->isHorizontal() || mForwardArrowImgVertical == "")
        return mForwardArrowImg;
    else
        return mForwardArrowImgVertical;
}

void SkinBase::setFontSize(const int fontSize)
{
    mFontSize = fontSize;
}

int SkinBase::fontSize() const
{
    if (MainModel::self()->isHorizontal() || mFontSizeVertical == 0)
        return mFontSize;
    else
        return mFontSizeVertical;
}

void SkinBase::setCandFontSize(const int candFontSize)
{
    mCandFontSize = candFontSize;
}

int SkinBase::candFontSize() const
{
    if (MainModel::self()->isHorizontal() || mCandFontSizeVertical == 0)
        return mCandFontSize;
    else
        return mCandFontSizeVertical;
}

void SkinBase::setInputColor(const QColor inputColor)
{
    mInputColor = inputColor;
}

QColor SkinBase::inputColor() const
{
    if (MainModel::self()->isHorizontal() || mInputColorVertical == Qt::color0)
        return mInputColor;
    else
        return mInputColorVertical;
}

void SkinBase::setIndexColor(const QColor indexColor)
{
    mIndexColor = indexColor;
}

QColor SkinBase::indexColor() const
{
    if (MainModel::self()->isHorizontal() || mIndexColorVertical == Qt::color0)
        return mIndexColor;
    else
        return mIndexColorVertical;
}

void SkinBase::setFirstCandColor(const QColor firstCandColor)
{
    mFirstCandColor = firstCandColor;
}

QColor SkinBase::firstCandColor() const
{
    if (MainModel::self()->isHorizontal() || mFirstCandColorVertical == Qt::color0)
        return mFirstCandColor;
    else
        return mFirstCandColorVertical;
}

void SkinBase::setOtherColor(const QColor otherColor)
{
    mOtherColor = otherColor;
}

QColor SkinBase::otherColor() const
{
    if (MainModel::self()->isHorizontal() || mOtherColorVertical == Qt::color0)
        return mOtherColor;
    else
        return mOtherColorVertical;
}

void SkinBase::setMainBackImg(const QString mainBackImg)
{
    mMainBackImg = mainBackImg;
}

QString SkinBase::mainBackImg() const
{
    return mMainBackImg;
}

void SkinBase::setMainLogoImg(const QString mainLogo)
{
    mMainLogo = mainLogo;
}

QString SkinBase::mainLogoImg() const
{
    return mMainLogo;
}

void SkinBase::setMainEngImg(const QString mainEng)
{
    mMainEngImg = mainEng;
}

QString SkinBase::mainEngImg() const
{
    return mMainEngImg;
}

void SkinBase::setMainCnImg(const QString mainCn)
{
    mMainCnImg = mainCn;
}

QString SkinBase::mainCnImg() const
{
    return mMainCnImg;
}

void SkinBase::setMainSimplifiedImg(const QString mainSimplified)
{
    mMainSimplifiedImg = mainSimplified;
}

QString SkinBase::mainSimplifiedImg() const
{
    return mMainSimplifiedImg;
}

void SkinBase::setMainTraditionalImg(const QString mainTraditional)
{
    mMainTraditionalImg = mainTraditional;
}

QString SkinBase::mainTraditionalImg() const
{
    return mMainTraditionalImg;
}

void SkinBase::setMainFullWidthImg(const QString mainFullWidth)
{
    mMainFullWidthImg = mainFullWidth;
}

QString SkinBase::mainFullWidthImg() const
{
    return mMainFullWidthImg;
}

void SkinBase::setMainHalfWidthImg(const QString mainHalfWidth)
{
    mMainHalfWidthImg = mainHalfWidth;
}

QString SkinBase::mainHalfWidthImg() const
{
    return mMainHalfWidthImg;
}

void SkinBase::setMainEngMarkImg(const QString mainEngMark)
{
    mMainEngMarkImg = mainEngMark;
}

QString SkinBase::mainEngMarkImg() const
{
    return mMainEngMarkImg;
}

void SkinBase::setMainCnMarkImg(const QString mainCnMark)
{
    mMainCnMarkImg = mainCnMark;
}

QString SkinBase::mainCnMarkImg() const
{
    return mMainCnMarkImg;
}

void SkinBase::setMainSoftKbdImg(const QString mainSoftKbd)
{
    mMainSoftKbdImg = mainSoftKbd;
}

QString SkinBase::mainSoftKbdImg() const
{
    return mMainSoftKbdImg;
}

void SkinBase::setMainSettingImg(const QString mainSetting)
{
    mMainSettingImg = mainSetting;
}

QString SkinBase::mainSettingImg() const
{
    return mMainSettingImg;
}

void SkinBase::setMainSymbolsImg(const QString mainSymbolsImg)
{
    mMainSymbolsImg = mainSymbolsImg;
}

QString SkinBase::mainSymbolsImg() const
{
    return mMainSymbolsImg;
}

void SkinBase::setMainPinImg(const QString mainPinImg)
{
    mMainPinImg = mainPinImg;
}

QString SkinBase::mainPinImg() const
{
    return mMainPinImg;
}

QString SkinBase::mainWbImg() const
{
    return mMainWbImg;
}
void SkinBase::setMainWbImg(const QString mainWbImg)
{
    mMainWbImg = mainWbImg;
}

#define DEFINE_PROPERTY(read, type, property, zero) \
    void SkinBase::set##property(const type read) { \
        m##property = read; \
    } \
    type SkinBase::read() const { \
        if (MainModel::self()->isHorizontal() || m##property##Vertical == zero) \
            return m##property; \
        else \
            return m##property##Vertical; \
    }

    DEFINE_PROPERTY(inputStringPosX, int, InputStringPosX, 0)
    DEFINE_PROPERTY(inputStringPosY, int, InputStringPosY, 0)
    DEFINE_PROPERTY(outputCandPosX, int, OutputCandPosX, 0)
    DEFINE_PROPERTY(outputCandPosY, int, OutputCandPosY, 0)
    DEFINE_PROPERTY(backArrowPosX, int, BackArrowPosX, 0)
    DEFINE_PROPERTY(backArrowPosY, int, BackArrowPosY, 0)
    DEFINE_PROPERTY(forwardArrowPosX, int, ForwardArrowPosX, 0)
    DEFINE_PROPERTY(forwardArrowPosY, int, ForwardArrowPosY, 0)

    DEFINE_PROPERTY(adjustWidth, int, AdjustWidth, 0)
    DEFINE_PROPERTY(adjustHeight, int, AdjustHeight, 0)

    DEFINE_PROPERTY(horizontalTileMode, QString, HorizontalTileMode, "")
    DEFINE_PROPERTY(verticalTileMode, QString, VerticalTileMode, "")

#undef DEFINE_PROPERTY

#define DEFINE_SET_PROPERTY(read, type, property) \
    void SkinBase::set##property(const type read) { \
        m##property = read; \
    }

    DEFINE_SET_PROPERTY(inputBackImgVertical, QString, InputBackImgVertical)
    DEFINE_SET_PROPERTY(tipsImgVertical, QString, TipsImgVertical)
    DEFINE_SET_PROPERTY(marginLeftVertical, int, MarginLeftVertical)

    DEFINE_SET_PROPERTY(marginRightVertical, int, MarginRightVertical)
    DEFINE_SET_PROPERTY(marginTopVertical, int, MarginTopVertical)
    DEFINE_SET_PROPERTY(marginBottomVertical, int, MarginBottomVertical)
    DEFINE_SET_PROPERTY(backArrowImgVertical, QString, BackArrowImgVertical)
    DEFINE_SET_PROPERTY(forwardArrowImgVertical, QString, ForwardArrowImgVertical)
    DEFINE_SET_PROPERTY(fontSizeVertical, int, FontSizeVertical)
    DEFINE_SET_PROPERTY(candFontSizeVertical, int, CandFontSizeVertical)
    DEFINE_SET_PROPERTY(inputColorVertical, QColor, InputColorVertical)
    DEFINE_SET_PROPERTY(indexColorVertical, QColor, IndexColorVertical)
    DEFINE_SET_PROPERTY(firstCandColorVertical, QColor, FirstCandColorVertical)
    DEFINE_SET_PROPERTY(otherColorVertical, QColor, OtherColorVertical)


    DEFINE_SET_PROPERTY(inputStringPosXVertical, int, InputStringPosXVertical)
    DEFINE_SET_PROPERTY(inputStringPosYVertical, int, InputStringPosYVertical)
    DEFINE_SET_PROPERTY(outputCandPosXVertical, int, OutputCandPosXVertical)
    DEFINE_SET_PROPERTY(outputCandPosYVertical, int, OutputCandPosYVertical)
    DEFINE_SET_PROPERTY(backArrowPosXVertical, int, BackArrowPosXVertical)
    DEFINE_SET_PROPERTY(backArrowPosYVertical, int, BackArrowPosYVertical)
    DEFINE_SET_PROPERTY(forwardArrowPosXVertical, int, ForwardArrowPosXVertical)
    DEFINE_SET_PROPERTY(forwardArrowPosYVertical, int, ForwardArrowPosYVertical)

    DEFINE_SET_PROPERTY(adjustWidthVertical, int, AdjustWidthVertical)
    DEFINE_SET_PROPERTY(adjustHeightVertical, int, AdjustHeightVertical)

    DEFINE_SET_PROPERTY(horizontalTileModeVertical, QString, HorizontalTileModeVertical)
    DEFINE_SET_PROPERTY(verticalTileModeVertical, QString, VerticalTileModeVertical)

    #undef DEFINE_SET_PROPERTY
