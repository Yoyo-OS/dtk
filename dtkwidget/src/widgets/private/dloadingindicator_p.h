/*
 * Copyright (C) 2015 ~ 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DLOADINGINDICATOR_P
#define DLOADINGINDICATOR_P

#include <DObjectPrivate>

#include <dloadingindicator.h>

DWIDGET_BEGIN_NAMESPACE

class DLoadingIndicatorPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
    DLoadingIndicatorPrivate(DLoadingIndicator *qq);

    void init();
    void setLoadingItem(QGraphicsItem *item);

    QVariantAnimation rotateAni;
    bool loading;
    QWidget *widgetSource = NULL;
    bool smooth = false;
    DLoadingIndicator::RotationDirection direction = DLoadingIndicator::Clockwise;

    D_DECLARE_PUBLIC(DLoadingIndicator)
};

DWIDGET_END_NAMESPACE

#endif // DLOADINGINDICATOR_P

