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

#ifndef __SKIN_FCITX_H__
#define __SKIN_FCITX_H__

#include "skinbase.h"

class SkinFcitx : public SkinBase
{
public:
    SkinFcitx();
    virtual ~SkinFcitx();
    virtual bool loadSkin(const QString skinPath);

private:
    QString getFilePath(const QString& path);

private:
    QString mSkinPath;
};

#endif // __SKIN_FCITX_H__
