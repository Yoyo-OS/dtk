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

#ifndef DAPPLICATION_P_H
#define DAPPLICATION_P_H

#include <DObjectPrivate>

#include <DApplication>
#include <DPathBuf>

#include <QIcon>
#include <QPointer>

class QLocalServer;
class QTranslator;

DCORE_USE_NAMESPACE
DWIDGET_BEGIN_NAMESPACE

class DAboutDialog;

class DApplicationPrivate : public DObjectPrivate
{
    D_DECLARE_PUBLIC(DApplication)

public:
    explicit DApplicationPrivate(DApplication *q);
    ~DApplicationPrivate();

    D_DECL_DEPRECATED QString theme() const;
    D_DECL_DEPRECATED void setTheme(const QString &theme);

    bool setSingleInstanceBySemaphore(const QString &key);
#ifdef Q_OS_UNIX
    bool setSingleInstanceByDbus(const QString &key);
#endif

    bool loadDtkTranslator(QList<QLocale> localeFallback);
    bool loadTranslator(QList<DPathBuf> translateDirs, const QString &name, QList<QLocale> localeFallback);
    void _q_onNewInstanceStarted();

    // 为控件适应当前虚拟键盘的位置
    void doAcclimatizeVirtualKeyboard(QWidget *window, QWidget *widget, bool allowResizeContentsMargins);
    void acclimatizeVirtualKeyboardForFocusWidget(bool allowResizeContentsMargins);
    void _q_panWindowContentsForVirtualKeyboard();
    void _q_resizeWindowContentsForVirtualKeyboard();

    static bool isUserManualExists();
public:
// int m_pidLockFD = 0;
    QLocalServer *m_localServer = nullptr;
#ifdef Q_OS_UNIX
    QStringList m_monitoredStartupApps;
#endif

    QString productName;
    QIcon   productIcon;
    QString appLicense;
    QString appDescription;
    QString homePage;
    QString acknowledgementPage;
    bool acknowledgementPageVisible = true;

    bool visibleMenuShortcutText   = false;
    bool visibleMenuCheckboxWidget = false;
    bool visibleMenuIcon           = false;
    bool autoActivateWindows       = false;

    DAppHandler *appHandler = Q_NULLPTR;
    DAboutDialog *aboutDialog = Q_NULLPTR;

    // 需要自适应虚拟键盘环境的窗口
    QPointer<QWidget> activeInputWindow;
    // 上一次为适配虚拟键盘所设置的值
    QPair<int, int> lastContentsMargins;
    QMargins activeInputWindowContentsMargins;
    QList<QWidget*> acclimatizeVirtualKeyboardWindows;
};

DWIDGET_END_NAMESPACE

#endif // DAPPLICATION_P_H
