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

#ifndef DSPINBOX_P_H
#define DSPINBOX_P_H

#include <DSpinBox>

#include <DObjectPrivate>

class QFrame;
class QLabel;

DWIDGET_BEGIN_NAMESPACE

class DLineEdit;
class DSpinBoxPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
    DSpinBoxPrivate(DSpinBox *parent);

    void init();

    DLineEdit *lineEdit = nullptr;
    D_DECLARE_PUBLIC(DSpinBox)
};

class DDoubleSpinBoxPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
    DDoubleSpinBoxPrivate(DDoubleSpinBox *parent);

    void init();

    DLineEdit *lineEdit = nullptr;
    D_DECLARE_PUBLIC(DDoubleSpinBox)
};

DWIDGET_END_NAMESPACE

#endif // DSPINBOX_P_H

