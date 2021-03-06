/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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

#include "dwindowgroupleader.h"

#include <QWindow>
#include <QGuiApplication>
#include <QPointer>

DGUI_BEGIN_NAMESPACE

#define DEFINE_CONST_CHAR(Name) const char _##Name[] = "_d_" #Name

DEFINE_CONST_CHAR(groupLeader);
DEFINE_CONST_CHAR(createGroupWindow);
DEFINE_CONST_CHAR(destoryGroupWindow);
DEFINE_CONST_CHAR(setWindowGroup);
DEFINE_CONST_CHAR(clientLeader);

class DWindowGroupLeaderPrivate
{
public:
    explicit DWindowGroupLeaderPrivate(quint32 groupLeader)
        : groupLeader(groupLeader) {
        QFunctionPointer clientLeaderFun = qApp->platformFunction(_clientLeader);

        if (clientLeaderFun) {
            clientLeader = reinterpret_cast<quint32(*)()>(clientLeaderFun)();
        }
    }

    quint32 groupLeader;
    quint32 clientLeader = 0;
    bool groupLeaderFromUser = false;

    QList<QPointer<QWindow>> windowList;

    void ensureGroupLeader();
    bool setWindowGroupLeader(quint32 window, quint32 groupLeader);
};

void DWindowGroupLeaderPrivate::ensureGroupLeader()
{
    if (groupLeader != 0)
        return;

    QFunctionPointer createGroupWindow = qApp->platformFunction(_createGroupWindow);

    if (!createGroupWindow)
        return;

    groupLeader = reinterpret_cast<quint32(*)()>(createGroupWindow)();
}

bool DWindowGroupLeaderPrivate::setWindowGroupLeader(quint32 window, quint32 groupLeader)
{
    QFunctionPointer setWindowGroup = qApp->platformFunction(_setWindowGroup);

    if (!setWindowGroup) {
        return false;
    }

    reinterpret_cast<void(*)(quint32, quint32)>(setWindowGroup)(window, groupLeader);

    return true;
}

/*!
  \class Dtk::Gui::DWindowGroupLeader
  \inmodule dtkgui
  
  \brief DWindowGroupLeader ????????????????????????????????????????????????????????????????????????.

  ???dxcb????????????????????????????????????????????????????????? DWindowGroupLeader::clientLeaderId
  ?????????????????????????????????????????????????????????????????? Qt::Dialog???Qt::Sheet???Qt::Tool???
  Qt::SplashScreen???Qt::ToolTip???Qt::Drawer???Qt::Popup ?????????????????????
  WM_TRANSIENT_FOR ???????????????????????????????????????????????????????????? QWindow::setTransientParent ???
  ???????????? WM_TRANSIENT_FOR ????????????????????????id?????????????????????????????????????????????????????????????????????????????????
  ??????????????????
  \code
  DWindowGroupLeader leader;
  QWindow wa, wb;
  QWindow topWindow;
  
  leader.addWindow(&wa);
  leader.addWindow(&wb);
  leader.addWindow(&topWindow);
  
  wa.setTitle("??????wa");
  wa.resize(300, 100);
  wa.show();
  wb.setTitle("??????wb");
  wb.resize(300, 100);
  wb.show();
  topWindow.setTitle("??????topWindow");
  topWindow.setFlag(Qt::Dialog);
  topWindow.resize(300, 300);
  topWindow.show();
  \endcode
  ??????topWindow??????????????????wa???wb??????
  \image wa_wb_topWindow.gif
  ???????????????????????????????????????????????? QWindow::setTransientParent ??? Qt::Dialog???????????????????????? WM_TRANSIENT_FOR
  ?????????????????????????????????????????????????????????????????????????????????????????????????????????????????? X11 ICCCM ???????????????
  \l {https://tronche.com/gui/x/icccm/sec-4.html#WM_TRANSIENT_FOR}
  \sa DWindowGroupLeader::clientLeaderId
  \sa Dtk::Widget::DApplication::loadDXcbPlugin
 */

/*!
  \brief DWindowGroupLeader::DWindowGroupLeader
  \a groupId ???0????????????????????????????????????????????? groupLeaderId
  \sa DWindowGroupLeader::groupLeaderId
 */
DWindowGroupLeader::DWindowGroupLeader(quint32 groupId)
    : d_ptr(new DWindowGroupLeaderPrivate(groupId))
{
    if (groupId != 0)
        d_ptr->groupLeaderFromUser = true;
}

/*!
  \brief DWindowGroupLeader::~DWindowGroupLeader
  ???????????????????????????????????????????????? groupLeaderId
  \sa DWindowGroupLeader::groupLeaderId
 */
DWindowGroupLeader::~DWindowGroupLeader()
{
    Q_D(DWindowGroupLeader);

    for (auto window : d->windowList)
        removeWindow(window);

    if (!d->groupLeaderFromUser) {
        QFunctionPointer destoryGroupWindow = qApp->platformFunction(_destoryGroupWindow);

        if (!destoryGroupWindow)
            return;

        reinterpret_cast<void(*)(quint32)>(destoryGroupWindow)(d->groupLeader);
    }
}

/*!
  \brief DWindowGroupLeader::groupLeaderId
  \return ????????????id???????????? QWindow::winId
  \warning ???????????????????????????id?????????????????????????????????id
 */
quint32 DWindowGroupLeader::groupLeaderId() const
{
    Q_D(const DWindowGroupLeader);

    const_cast<DWindowGroupLeaderPrivate*>(d)->ensureGroupLeader();

    return d->groupLeader;
}

/*!
  \brief DWindowGroupLeader::clientLeaderId
  \return ??????????????????????????????id
 */
quint32 DWindowGroupLeader::clientLeaderId() const
{
    Q_D(const DWindowGroupLeader);

    return d->clientLeader;
}

/*!
  \brief DWindowGroupLeader::addWindow
  ???????????????????????????
  \a window
  \warning ??????????????????????????????????????????????????????????????????????????????????????????????????????
 */
void DWindowGroupLeader::addWindow(QWindow *window)
{
    Q_ASSERT(window);
    Q_D(DWindowGroupLeader);

    d->ensureGroupLeader();

    window->setProperty(_groupLeader, d->groupLeader);

    if (window->handle()) {
        d->setWindowGroupLeader(window->winId(), d->groupLeader);
    }

    d->windowList << window;
}

/*!
  \brief DWindowGroupLeader::removeWindow
  ??????????????????????????????
  \a window
  \warning ?????????????????????????????????????????????????????????
 */
void DWindowGroupLeader::removeWindow(QWindow *window)
{
    if (!window)
        return;

    window->setProperty(_groupLeader, QVariant());

    Q_D(DWindowGroupLeader);

    if (window->handle()) {
        d->setWindowGroupLeader(window->winId(), d->clientLeader);
    }
}

DGUI_END_NAMESPACE
