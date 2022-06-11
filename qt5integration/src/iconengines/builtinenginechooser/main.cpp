/*
 * Copyright (C) 2021 ~ 2022 Deepin Technology Co., Ltd.
 *
 * Author:     ck <chenke@uniontech.com>
 *
 * Maintainer: ck <chenke@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <qiconengineplugin.h>
#include "dpluginloader.h"
QT_BEGIN_NAMESPACE

class DBuiltinIconEnginePlugin : public QIconEnginePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QIconEngineFactoryInterface" FILE "builtinengine.json")

public:
    DBuiltinIconEnginePlugin(QObject *parent = nullptr);
    QIconEngine *create(const QString &iconName = QString());

private:
    QScopedPointer<QIconEnginePlugin> m_pluginProxy;
};

DBuiltinIconEnginePlugin::DBuiltinIconEnginePlugin(QObject *parent)
    : QIconEnginePlugin(parent)
{
    m_pluginProxy.reset(DPluginLoader::load<QIconEnginePlugin>(TARGET));
}

QIconEngine *DBuiltinIconEnginePlugin::create(const QString &iconName)
{
    return m_pluginProxy ? m_pluginProxy->create(iconName) : nullptr;
}

QT_END_NAMESPACE

#include "main.moc"
