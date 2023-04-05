/*
 * Copyright (C) 2013 Canonical Ltd  All rights reserved.
 *
 * Authors:
 *         FJKong    kroodywar3@gmail.com/fanjun.kong@canonical.com
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
#include <QBitmap>
#include <QColor>
#include <QDir>
#include <QFile>
#include <QFontMetrics>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QTextStream>

#include "skinfcitx.h"

SkinFcitx::SkinFcitx() : SkinBase()
{

}

SkinFcitx::~SkinFcitx()
{

}

bool SkinFcitx::loadSkin(const QString skinPath)
{
    init();

    QFile fcitxSkinConfFile(skinPath + "fcitx_skin.conf");
    if (!fcitxSkinConfFile.exists())
        return false;

    if (!fcitxSkinConfFile.open(QIODevice::ReadOnly))
        return false;

    mSkinPath = skinPath;

    QString line;
    QString key, value;

    bool skinFont = false;
    bool skinInputBar = false;
    bool skinFontVertical = false;
    bool skinInputBarVertical = false;
    bool skinMainBar = false;

    QTextStream textStream(fcitxSkinConfFile.readAll());

    do {
        line = textStream.readLine();

        if (line.isEmpty() || line.at(0) == '#')
            continue;

        if (line.at(0) == '[') {
            skinFont = (line == "[SkinFont]");
            skinInputBar = (line == "[SkinInputBar]");
            skinFontVertical = (line == "[SkinFontVertical]");
            skinInputBarVertical = (line == "[SkinInputBarVertical]");
            skinMainBar = (line == "[SkinMainBar]");
            continue;
        }

        if (!line.contains('='))
            continue;

        key = line.split('=').at(0);
        value = line.split('=').at(1);

        if (value.isEmpty())
            continue;

        if (skinFont)
        {
            if (key == "FontSize")
                setFontSize(value.toInt());
            else if (key == "CandFontSize")
                setCandFontSize(value.toInt());
            else if (key == "InputColor")
                setInputColor(value2color(value));
            else if (key == "IndexColor")
                setIndexColor(value2color(value));
            else if (key == "FirstCandColor")
                setFirstCandColor(value2color(value));
            else if (key == "OtherColor")
                setOtherColor(value2color(value));
        }
        else if (skinInputBar)
        {
            if (key == "BackImg")
                setInputBackImg(skinPath + value);
            else if (key == "TipsImg")
                setTipsImg(skinPath + value);
            else if (key == "AdjustWidth")
                setAdjustWidth(value.toInt());
            else if (key == "AdjustHeight")
                setAdjustHeight(value.toInt());
            else if (key == "MarginLeft")
                setMarginLeft(value.toInt());
            else if (key == "MarginTop")
                setMarginTop(value.toInt());
            else if (key == "MarginRight")
                setMarginRight(value.toInt());
            else if (key == "MarginBottom")
                setMarginBottom(value.toInt());
            else if (key == "horizontalTileMode")
                setHorizontalTileMode(value);
            else if (key == "verticalTileMode")
                setVerticalTileMode(value);
            else if (key == "InputStringPosX")
                setInputStringPosX(value.toInt());
            else if (key == "InputStringPosY")
                setInputStringPosY(value.toInt());
            else if (key == "OutputCandPosX")
                setOutputCandPosX(value.toInt());
            else if (key == "OutputCandPosY")
                setOutputCandPosY(value.toInt());
            else if (key == "BackArrowPosX")
                setBackArrowPosX(value.toInt());
            else if (key == "BackArrowPosY")
                setBackArrowPosY(value.toInt());
            else if (key == "ForwardArrowPosX")
                setForwardArrowPosX(value.toInt());
            else if (key == "ForwardArrowPosY")
                setForwardArrowPosY(value.toInt());
            else if (key == "BackArrow")
                setBackArrowImg(skinPath + value);
            else if (key == "ForwardArrow")
                setForwardArrowImg(skinPath + value);
        }
        else if (skinFontVertical)
        {
            if (key == "FontSize")
                setFontSizeVertical(value.toInt());
            else if (key == "CandFontSize")
                setCandFontSizeVertical(value.toInt());
            else if (key == "InputColor")
                setInputColorVertical(value2color(value));
            else if (key == "IndexColor")
                setIndexColorVertical(value2color(value));
            else if (key == "FirstCandColor")
                setFirstCandColorVertical(value2color(value));
            else if (key == "OtherColor")
                setOtherColorVertical(value2color(value));
        }
        else if (skinInputBarVertical)
        {
            if (key == "BackImg")
                setInputBackImgVertical(skinPath + value);
            else if (key == "TipsImg")
                setTipsImgVertical(skinPath + value);
            else if (key == "AdjustWidth")
                setAdjustWidthVertical(value.toInt());
            else if (key == "AdjustHeight")
                setAdjustHeightVertical(value.toInt());
            else if (key == "MarginLeft")
                setMarginLeftVertical(value.toInt());
            else if (key == "MarginTop")
                setMarginTopVertical(value.toInt());
            else if (key == "MarginRight")
                setMarginRightVertical(value.toInt());
            else if (key == "MarginBottom")
                setMarginBottomVertical(value.toInt());
            else if (key == "horizontalTileMode")
                setHorizontalTileModeVertical(value);
            else if (key == "verticalTileMode")
                setVerticalTileModeVertical(value);
            else if (key == "InputStringPosX")
                setInputStringPosXVertical(value.toInt());
            else if (key == "InputStringPosY")
                setInputStringPosYVertical(value.toInt());
            else if (key == "OutputCandPosX")
                setOutputCandPosXVertical(value.toInt());
            else if (key == "OutputCandPosY")
                setOutputCandPosYVertical(value.toInt());
            else if (key == "BackArrowPosX")
                setBackArrowPosXVertical(value.toInt());
            else if (key == "BackArrowPosY")
                setBackArrowPosYVertical(value.toInt());
            else if (key == "ForwardArrowPosX")
                setForwardArrowPosXVertical(value.toInt());
            else if (key == "ForwardArrowPosY")
                setForwardArrowPosYVertical(value.toInt());
            else if (key == "BackArrow")
                setBackArrowImgVertical(skinPath + value);
            else if (key == "ForwardArrow")
                setForwardArrowImgVertical(skinPath + value);
        }
        else if (skinMainBar)
        {
            if (key == "BackImg")
                setMainBackImg(getFilePath(value));
            else if (key == "Logo")
                setMainLogoImg(getFilePath(value));
            else if (key == "EngImg")
                setMainEngImg(getFilePath(value));
            else if (key == "CnImg")
                setMainCnImg(getFilePath(value));
            else if (key == "WbImg")
                setMainWbImg(getFilePath(value));
            else if (key == "SimplifiedImg")
                setMainSimplifiedImg(getFilePath(value));
            else if (key == "TraditionalImg")
                setMainTraditionalImg(getFilePath(value));
            else if (key == "FullWidthImg")
                setMainFullWidthImg(getFilePath(value));
            else if (key == "HalfWidthImg")
                setMainHalfWidthImg(getFilePath(value));
            else if (key == "CnMarkImg")
                setMainCnMarkImg(getFilePath(value));
            else if (key == "EngMarkImg")
                setMainEngMarkImg(getFilePath(value));
            else if (key == "SoftKbdImg")
                setMainSoftKbdImg(getFilePath(value));
            else if (key == "SettingImg")
                setMainSettingImg(getFilePath(value));
            else if (key == "SymbolsImg")
                setMainSymbolsImg(getFilePath(value));
            else if(key == "PinImg")
                setMainPinImg(getFilePath(value));
        }
    } while (!line.isNull());

    fcitxSkinConfFile.close();
    return true;
}

QString SkinFcitx::getFilePath(const QString &path)
{
    return mSkinPath + path;
}
