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

#ifndef DSEARCHEDIT_H
#define DSEARCHEDIT_H

#include <DLineEdit>

DWIDGET_BEGIN_NAMESPACE

class DSearchEditPrivate;
class LIBDTKWIDGETSHARED_EXPORT DSearchEdit : public DLineEdit
{
    Q_OBJECT
    Q_PROPERTY(bool voiceInput READ isVoiceInput NOTIFY voiceChanged)

public:
    explicit DSearchEdit(QWidget *parent = nullptr);
    ~DSearchEdit();

    void setPlaceHolder(QString placeHolder);
    QString placeHolder() const;

    void clear();
    void clearEdit();

    bool isVoiceInput() const;

    void setPlaceholderText(const QString &text);
    QString placeholderText() const;

Q_SIGNALS:
    void voiceInputFinished();
    void searchAborted();
    void voiceChanged();

protected:
    Q_DISABLE_COPY(DSearchEdit)
    D_DECLARE_PRIVATE(DSearchEdit)
    Q_PRIVATE_SLOT(d_func(), void _q_toEditMode(bool))
    D_PRIVATE_SLOT(void _q_onVoiceActionTrigger(bool))
    D_PRIVATE_SLOT(void _q_clearFocus())
};

DWIDGET_END_NAMESPACE

#endif // DSEARCHEDIT_H
