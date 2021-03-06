/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
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

#include "qdeepintheme.h"
#include "qdeepinfiledialoghelper.h"
#include "diconproxyengine.h"
#include "filedialogmanager_interface.h"
#include "dthemesettings.h"

#include <DGuiApplicationHelper>
#include <DPlatformTheme>

#include <QGuiApplication>
#include <QIconEnginePlugin>

#include <XdgIcon>

#include <private/qicon_p.h>
#include <private/qiconloader_p.h>
#define private public
#include <private/qhighdpiscaling_p.h>
#undef private
#include <private/qwindow_p.h>
#include <private/qguiapplication_p.h>
#include <qpa/qwindowsysteminterface_p.h>
#include <qpa/qplatformscreen.h>
#include <qpa/qplatformcursor.h>

#undef signals
#include <X11/Xlib.h>


DGUI_USE_NAMESPACE

#if XDG_ICON_VERSION_MAR >= 3
namespace DEEPIN_QT_THEME {
QThreadStorage<QString> colorScheme;
void(*setFollowColorScheme)(bool);
bool(*followColorScheme)();
}
#endif

#define DISABLE_UPDATE_WINDOW_GEOMETRY "D_DISABLE_UPDATE_WINDOW_GEOMETRY_FOR_SCALE"
#define DNOT_UPDATE_WINDOW_GEOMETRY "_d_disable_update_geometry_for_scale"
#define HOOK_UPDATE_WINDOW_GEOMETRY_OBJECT "_d_hookUpdateGeometryForScaleObject"
#define UPDATE_WINDOW_GEOMETRY_ENTRY "_d_updateGeometryForScaleEntry"
#define UPDATE_WINDOW_GEOMETRY_GEOMETRY "_d_updateGeometryForScaleGeometry"
#define UPDATE_WINDOW_GEOMETRY_EXIT "_d_updateGeometryForScaleExit"

QT_BEGIN_NAMESPACE

const char *QDeepinTheme::name = "deepin";
bool QDeepinTheme::m_usePlatformNativeDialog = true;
QMimeDatabase QDeepinTheme::m_mimeDatabase;
DThemeSettings *QDeepinTheme::m_settings = 0;

extern void updateXdgIconSystemTheme();

static void onIconThemeSetCallback()
{
    QIconLoader::instance()->updateSystemTheme();
    updateXdgIconSystemTheme();

    if (qApp->inherits("Dtk::Widget::DApplication")) {
        // emit the signal: DApplication::iconThemeChanged
        qApp->metaObject()->invokeMethod(qApp, QT_STRINGIFY(iconThemeChanged));
    }

    // ???????????????????????????
    QEvent update(QEvent::UpdateRequest);
    for (QWindow *window : qGuiApp->allWindows()) {
        if (window->type() == Qt::Desktop)
            continue;

        qApp->sendEvent(window, &update);
    }
}

static void onFontChanged()
{
    // ???????????????font??????
    if (QGuiApplicationPrivate::app_font)
        delete QGuiApplicationPrivate::app_font;
    QGuiApplicationPrivate::app_font = nullptr;

    QEvent event(QEvent::ApplicationFontChange);
    qApp->sendEvent(qApp, &event);

    // ?????????????????????????????????
    for (QWindow *window : qGuiApp->allWindows()) {
        if (window->type() == Qt::Desktop)
            continue;

        qApp->sendEvent(window, &event);
    }

    qApp->sendEvent(DGuiApplicationHelper::instance(), &event);
    Q_EMIT qGuiApp->fontChanged(qGuiApp->font());
}

static void updateWindowGeometry(QWindow *w)
{
    if (w->type() == Qt::ForeignWindow || w->type() == Qt::Desktop) {
        return;
    }

    if (!w->handle() || !w->isTopLevel())
        return;

    if (w->property(DNOT_UPDATE_WINDOW_GEOMETRY).toBool()) {
        QWindowSystemInterfacePrivate::GeometryChangeEvent gce(w, QHighDpi::fromNativePixels(w->handle()->geometry(), w)
                                                   #if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
                                                               , QRect()
                                                   #endif
                                                               );
        QGuiApplicationPrivate::processGeometryChangeEvent(&gce);
    } else {// ??????????????????????????????
        const QRect currentGeo = QWindowPrivate::get(w)->geometry;

        if (!currentGeo.isValid())
            return;

        // ???????????????hook?????????????????????????????????
        QObject *hook_obj = qvariant_cast<QObject*>(w->property(HOOK_UPDATE_WINDOW_GEOMETRY_OBJECT));

        if (!hook_obj) {
            hook_obj = w;
        }

        bool accept = true;
        // ?????????????????????????????????geometry
        if (hook_obj->metaObject()->indexOfMethod(UPDATE_WINDOW_GEOMETRY_ENTRY "()") >= 0) {
            QMetaObject::invokeMethod(hook_obj, UPDATE_WINDOW_GEOMETRY_ENTRY,
                                      Qt::DirectConnection, Q_RETURN_ARG(bool, accept));
        }

        if (!accept) {
            // ????????????
            return;
        }

        QRect nativeGeo;
        // ?????????????????????????????????geometry
        if (hook_obj->metaObject()->indexOfMethod(UPDATE_WINDOW_GEOMETRY_GEOMETRY "()") >= 0) {
            QMetaObject::invokeMethod(hook_obj, UPDATE_WINDOW_GEOMETRY_GEOMETRY,
                                      Qt::DirectConnection, Q_RETURN_ARG(QRect, nativeGeo));
        }

        if (!nativeGeo.isValid()) {
            nativeGeo = w->handle()->geometry();

            qreal scale = QHighDpiScaling::factor(w);
            const QPoint &cursor_pos = w->screen()->handle()->cursor()->pos();
            // ???????????????active?????????????????????????????????????????????????????????????????????????????????????????????
            if (w->isActive()) {
                const QMargins &frame_margins = w->handle()->frameMargins();
                const QRect &frame_rect = nativeGeo.marginsAdded(frame_margins);

                if (frame_rect.contains(cursor_pos)) {
                    nativeGeo.moveTopLeft(cursor_pos + (nativeGeo.topLeft() - cursor_pos) * currentGeo.width() * scale / nativeGeo.width());
                }
            }

            nativeGeo.setSize(currentGeo.size() * scale);
        }

        bool positionAutomaticSave = QWindowPrivate::get(w)->positionAutomatic;
        QWindowPrivate::get(w)->positionAutomatic = false;
        w->handle()->setGeometry(nativeGeo);
        QWindowPrivate::get(w)->positionAutomatic = positionAutomaticSave;
        // ????????????
        QGuiApplication::sendEvent(w, new QEvent(QEvent::UpdateRequest));
        // ????????????geometry????????????
        if (hook_obj->metaObject()->indexOfMethod(UPDATE_WINDOW_GEOMETRY_EXIT "()") >= 0) {
            QMetaObject::invokeMethod(hook_obj, UPDATE_WINDOW_GEOMETRY_EXIT);
        }
    }
}

static void updateAllWindowGeometry()
{
    for (QWindow *w : qGuiApp->allWindows()) {
        updateWindowGeometry(w);
    }
}

// ??????????????????????????????geometry????????????????????????????????????????????????????????????????????????
static void updateAllWindowGeometryDelay(int interval = 500)
{
    static QTimer *t = new QTimer();

    t->setSingleShot(true);
    t->setInterval(interval);
    t->connect(t, &QTimer::timeout, t, updateAllWindowGeometry, Qt::UniqueConnection);
    t->start();
}

static void notifyScreenScaleUpdated()
{
    for (QScreen *s : qGuiApp->screens()) {
        Q_EMIT s->geometryChanged(s->geometry());

        // ????????????????????????????????????????????????DApplication?????????????????????
        if (qGuiApp->metaObject()->indexOfSignal("screenDevicePixelRatioChanged(QScreen*)")) {
            qGuiApp->metaObject()->invokeMethod(qGuiApp, "screenDevicePixelRatioChanged", Q_ARG(QScreen*, s));
        }
    }
}

static bool updateScaleFactor(qreal value)
{
    if (qIsNull(value)) {
        value = 1.0;
    }

    if (qFuzzyCompare(QHighDpiScaling::m_factor, value)) {
        return false;
    }

    QHighDpiScaling::setGlobalFactor(value);

    return true;
}

static void onScaleFactorChanged(qreal value)
{
    if (updateScaleFactor(value)) {
        notifyScreenScaleUpdated();
        updateAllWindowGeometryDelay();
    }
}

// ?????????????????????????????????????????????
class AutoScaleWindowObject : public QObject
{
    Q_OBJECT
public:
    explicit AutoScaleWindowObject(QObject *parent = nullptr)
        : QObject(parent) {
        qGuiApp->installEventFilter(this);
    }

    void onScreenChanged(QScreen *s)
    {
        Q_UNUSED(s);

        if (QWindow *w = qobject_cast<QWindow*>(sender())) {
            updateWindowGeometry(w);
        }
    }

private:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() != QEvent::PlatformSurface)
            return false;

        if (QWindow *w = qobject_cast<QWindow*>(watched)) {
            QPlatformSurfaceEvent *se = static_cast<QPlatformSurfaceEvent*>(event);

            if (se->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {
                // ????????????????????????????????????????????????????????????????????????????????????
                connect(w, &QWindow::screenChanged,
                        this, &AutoScaleWindowObject::onScreenChanged,
                        Qt::DirectConnection);
            } else {
                disconnect(w, &QWindow::screenChanged,
                           this, &AutoScaleWindowObject::onScreenChanged);
            }
        }

        return false;
    }
};

// ??????ScreenScaleFactor???????????????????????????????????????
static bool isGenericScreenScaleFactors(const QByteArray &value)
{
    for (char ch : value) {
        if (ch == '=' || ch == ';')
            return false;
    }

    return true;
}

static void onAutoScaleWindowChanged()
{
    bool on = QDeepinTheme::getSettings()->autoScaleWindow();

    if (on) {
        const QByteArray &multi_scale = QDeepinTheme::getSettings()->screenScaleFactors();

        // ??????????????????????????????????????????????????????????????????
        if (isGenericScreenScaleFactors(multi_scale)) {
            on = false;
        }
    }

    static AutoScaleWindowObject *event_fileter = nullptr;

    if (on) {
        if (event_fileter)
            return;

        event_fileter = new AutoScaleWindowObject(qGuiApp);
    } else {
        if (!event_fileter)
            return;

        event_fileter->deleteLater();
        event_fileter = nullptr;
    }
}

// ?????? QT_SCREEN_SCALE_FACTORS ????????????????????????????????????????????????Qt???????????? dpi ???????????????
// ????????????????????????????????????????????????font dpi??????????????????pixel size???QFont?????????????????????
// QGuiApplication::font() ????????????pixel size??????????????????????????????????????????????????????????????????
// ????????????????????????
// ???????????????????????? ScreenScaleFactors??????????????? ScaleLogcailDpi ?????????????????????????????????
// ??? logicalDpi???
static bool updateScaleLogcailDpi(const QPair<qreal, qreal> &dpi)
{
    bool ok = dpi.first >= 0 && dpi.second >= 0;
#if QT_VERSION < QT_VERSION_CHECK(5,14,0)
    if (dpi.first > 0) {
        QHighDpiScaling::m_logicalDpi.first = dpi.first;
    } else if (qIsNull(dpi.first)) {
        QHighDpiScaling::m_logicalDpi.first = qGuiApp->primaryScreen()->handle()->logicalDpi().first;
    }

    if (dpi.second > 0) {
        QHighDpiScaling::m_logicalDpi.second = dpi.second;
    } else if (qIsNull(dpi.second)) {
        QHighDpiScaling::m_logicalDpi.second = qGuiApp->primaryScreen()->handle()->logicalDpi().second;
    }
#else
    QHighDpiScaling::m_usePixelDensity = false;
#endif
    return ok;
}

static QByteArray getEnvValueByScreenScaleFactors(const QByteArray &data)
{
    QByteArray envValue;

    if (!isGenericScreenScaleFactors(data)) {
        envValue = data;
    } else if (!data.isEmpty()) {
        // ?????????????????????Qt????????????????????????????????????????????????????????????????????????????????????????????????????????????
        int screen_count = qApp->screens().count();

        // ?????????????????????????????????????????????????????????
        while (--screen_count) {
            envValue.append(data).append(';');
        }

        // ????????????????????????????????????
        envValue.append(data);
    }

    return envValue;
}

static bool updateScreenScaleFactors(DThemeSettings *s, const QByteArray &value, bool unsetenv = false)
{
    const QByteArray &envValue = getEnvValueByScreenScaleFactors(value);

    if (qgetenv("QT_SCREEN_SCALE_FACTORS") == envValue)
        return false;

    if (envValue.isEmpty()) {
        if (!unsetenv)
            return false;

        qunsetenv("QT_SCREEN_SCALE_FACTORS");
    } else {
        qputenv("QT_SCREEN_SCALE_FACTORS", envValue);
    }

    QHighDpiScaling::updateHighDpiScaling();
#if QT_VERSION < QT_VERSION_CHECK(5,14,0)
    qDebug() << QHighDpiScaling::logicalDpi();
#else
    qDebug() << QHighDpiScaling::logicalDpi(QGuiApplication::primaryScreen());
#endif
    updateScaleLogcailDpi(s->scaleLogicalDpi());

#if QT_VERSION < QT_VERSION_CHECK(5,14,0)
    qDebug() << QHighDpiScaling::logicalDpi();
#else
    qDebug() << QHighDpiScaling::logicalDpi(QGuiApplication::primaryScreen());
#endif

    return true;
}
static void onScreenScaleFactorsChanged(const QByteArray &value)
{
    if (updateScreenScaleFactors(QDeepinTheme::getSettings(), value, true)) {
        notifyScreenScaleUpdated();
        updateAllWindowGeometryDelay();
    }
}

static bool enabledRTScreenScale()
{
    // ????????????????????????????????????????????????????????????????????????????????????????????????????????????
    static bool enable = !qEnvironmentVariableIsSet("D_DISABLE_RT_SCREEN_SCALE") &&
                            !qEnvironmentVariableIsSet("QT_DEVICE_PIXEL_RATIO") &&
                            !qEnvironmentVariableIsSet("QT_SCALE_FACTOR") &&
                            !qEnvironmentVariableIsSet("QT_AUTO_SCREEN_SCALE_FACTOR") &&
                            !qEnvironmentVariableIsSet("QT_SCREEN_SCALE_FACTORS") &&
                            !QCoreApplication::testAttribute(Qt::AA_DisableHighDpiScaling) &&
                            !QCoreApplication::testAttribute(Qt::AA_EnableHighDpiScaling) &&
                            qGuiApp->platformName().endsWith("xcb");

    return enable;
}

QDeepinTheme::QDeepinTheme()
{
#if XDG_ICON_VERSION_MAR >= 3
    // ??????!!!, ???????????????????????????????????????, ??????Qt???????????????, ??????QTimer
    DEEPIN_QT_THEME::setFollowColorScheme = XdgIcon::setFollowColorScheme;
    DEEPIN_QT_THEME::followColorScheme = XdgIcon::followColorScheme;
#endif
    m_hints = new HintsSettings();
    if (enabledRTScreenScale()) {
        QScopedPointer<DThemeSettings> setting(new DThemeSettings(false));
        // ??????????????????????????????????????????
        updateScaleFactor(setting->scaleFactor());

        if (!updateScreenScaleFactors(setting.data(), setting->screenScaleFactors())) {
            updateScaleLogcailDpi(setting->scaleLogicalDpi());
        }
    }
}

QDeepinTheme::~QDeepinTheme()
{
    if (QDeepinFileDialogHelper::manager) {
        QDeepinFileDialogHelper::manager->deleteLater();
        QDeepinFileDialogHelper::manager = Q_NULLPTR;
    }
}

bool QDeepinTheme::usePlatformNativeDialog(DialogType type) const
{
    if (type == FileDialog) {
        if (qgetenv("_d_disableDBusFileDialog") == "true")
            return false;

        static bool dbusDialogManagerInitialized = false;

        if (!dbusDialogManagerInitialized) {
            dbusDialogManagerInitialized = true;
            QDeepinFileDialogHelper::initDBusFileDialogManager();
        }

        return m_usePlatformNativeDialog
                && QDeepinFileDialogHelper::manager
                && QDeepinFileDialogHelper::manager->isUseFileChooserDialog();
    }

    return QGenericUnixTheme::usePlatformNativeDialog(type);
}

QPlatformDialogHelper *QDeepinTheme::createPlatformDialogHelper(DialogType type) const
{
    if (type == FileDialog && usePlatformNativeDialog(type))
        return new QDeepinFileDialogHelper();

    return QGenericUnixTheme::createPlatformDialogHelper(type);
}

static QIconEnginePlugin *getIconEngineFactory(const QString &key)
{
    static QFactoryLoader loader(QIconEngineFactoryInterface_iid, QLatin1String("/iconengines"), Qt::CaseSensitive);
    int index = loader.indexOf(key);

    if (index != -1) {
        return qobject_cast<QIconEnginePlugin *>(loader.instance(index));
    }

    return nullptr;
}

static QIconEngine *createBuiltinIconEngine(const QString &iconName)
{
    static QIconEnginePlugin *plugin = getIconEngineFactory(QStringLiteral("DBuiltinIconEngine"));

    return plugin ? plugin->create(iconName) : nullptr;
}

static QIconEngine *createXdgProxyIconEngine(const QString &iconName)
{
    static QIconEnginePlugin *plugin = getIconEngineFactory(QStringLiteral("XdgIconProxyEngine"));

    return plugin ? plugin->create(iconName) : nullptr;
}

QIconEngine *QDeepinTheme::createIconEngine(const QString &iconName) const
{
#ifdef DTHEMED_ICON_LOOKUP
    if (iconName.contains("/"))
        // handle Qt-standard paths.
        return QGenericUnixTheme::createIconEngine(iconName);
    else
        return new DIconEngine(iconName);
#elif XDG_ICON_VERSION_MAR < 3
    return XdgIconEngineCreator::create(iconName);
#else
    static QSet<QString> non_builtin_icon_cache;

    if (!non_builtin_icon_cache.contains(iconName)) {
        // ???????????????????????????icon???????????????
        // ?????????????????????????????????????????????
        // ????????????????????????
        if (QIconEngine *engine = createBuiltinIconEngine(iconName)) {
            if (engine->isNull()) {
                non_builtin_icon_cache.insert(iconName);
                delete engine;
            } else {
                return engine;
            }
        } else {
            non_builtin_icon_cache.insert(iconName);
        }
    }

    return createXdgProxyIconEngine(iconName);
#endif
}

QPixmap QDeepinTheme::standardPixmap(QPlatformTheme::StandardPixmap sp, const QSizeF &size) const
{
    return QGenericUnixTheme::standardPixmap(sp, size);
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
QIcon QDeepinTheme::fileIcon(const QFileInfo &fileInfo, QPlatformTheme::IconOptions iconOptions) const
{
    Q_UNUSED(iconOptions);

    return XdgIcon::fromTheme(m_mimeDatabase.mimeTypeForFile(fileInfo).iconName());
}
#else
QPixmap QDeepinTheme::fileIconPixmap(const QFileInfo &fileInfo, const QSizeF &size, QPlatformTheme::IconOptions iconOptions) const
{
    Q_UNUSED(iconOptions);

    return XdgIcon::fromTheme(m_mimeDatabase.mimeTypeForFile(fileInfo).iconName()).pixmap(size.toSize());
}
#endif

static DPlatformTheme *appTheme()
{
    static QPointer<DPlatformTheme> theme;

    if (!theme) {
        theme = DGuiApplicationHelper::instance()->applicationTheme();
        QObject::connect(theme, &DPlatformTheme::iconThemeNameChanged, &onIconThemeSetCallback);
        QObject::connect(theme, &DPlatformTheme::fontNameChanged, &onFontChanged);
        QObject::connect(theme, &DPlatformTheme::fontPointSizeChanged, [] {
            if (theme->fontName().isEmpty())
                return;

            onFontChanged();
        });
        QObject::connect(theme, &DPlatformTheme::gtkFontNameChanged, [] {
            if (theme->fontName().isEmpty()) {
                onFontChanged();
            }
        });
    }

    return theme;
}

QVariant QDeepinTheme::themeHint(QPlatformTheme::ThemeHint hint) const
{
    switch (hint) {
    case QPlatformTheme::StyleNames: {
        return QStringList({"chameleon", "fusion","yoyo"});
    }
    case QPlatformTheme::IconThemeSearchPaths:
        return QVariant(QGenericUnixTheme::xdgIconThemePaths() << QDir::homePath() + "/.local/share/icons");
    case UseFullScreenForPopupMenu:
        return true;
    case ShowShortcutsInContextMenus:
        return false;
    default:
        break;
    }
    QVariant m_hint = m_hints->hint(hint);
    if (m_hint.isValid()) {
        return m_hint;
    } else {
        return QPlatformTheme::themeHint(hint);
    }
}

const QPalette *QDeepinTheme::palette(QPlatformTheme::Palette type) const
{
    if (type != SystemPalette) {
        return QGenericUnixTheme::palette(type);
    }

    static QPalette palette;
    palette = DGuiApplicationHelper::instance()->applicationPalette();

    return &palette;
}

const QFont *QDeepinTheme::font(QPlatformTheme::Font type) const
{
    if (!qApp->desktopSettingsAware())
        return QGenericUnixTheme::font(type);

    switch (type) {
    case SystemFont:
        if (DPlatformTheme *theme = appTheme()) {
            QByteArray font_name = theme->fontName();
            qreal font_size = 0;

            if (font_name.isEmpty()) {
                font_name = theme->gtkFontName();
                int font_size_index = font_name.lastIndexOf(' ');

                if (font_size_index <= 0)
                    break;

                font_size = font_name.mid(font_size_index + 1).toDouble();
                font_name = font_name.left(font_size_index);
            } else {
                font_size = theme->fontPointSize();
            }

            if (font_size <= 0) {
                font_size = 10.5;
            }

            static QFont font = QFont(QString());

            font.setFamily(font_name);
            font.setPointSizeF(font_size);

            return &font;
        }

        break;
    case FixedFont: {
        if (DPlatformTheme *theme = appTheme()) {
            QByteArray font_name = theme->monoFontName();

            if (font_name.isEmpty()) {
                break;
            }

            qreal font_size = theme->fontPointSize();

            if (font_size <= 0) {
                font_size = 10.5;
            }

            static QFont font = QFont(QString());

            font.setFamily(font_name);
            font.setPointSizeF(font_size);

            return &font;
        }

        break;
    }
    default:
        break;
    }

    return QGenericUnixTheme::font(type);
}

static void compelledUpdateScaleLogcailDpi() {
    updateScaleLogcailDpi(QDeepinTheme::getSettings()->scaleLogicalDpi());
}

static void onScreenAdded(QScreen *s) {
    if (QHighDpiScaling::m_screenFactorSet) {
        auto setting = QDeepinTheme::getSettings();
        auto value = setting->screenScaleFactors();

        if (!value.isEmpty() && isGenericScreenScaleFactors(value)) {
            const QByteArray &envValue = getEnvValueByScreenScaleFactors(value);

            qputenv("QT_SCREEN_SCALE_FACTORS", envValue);
            bool ok = false;
            qreal scale = value.toDouble(&ok);

            // ??????????????????????????????
            if (ok)
                QHighDpiScaling::setScreenFactor(s, scale);
        }
    }

    compelledUpdateScaleLogcailDpi();
}

DThemeSettings *QDeepinTheme::settings() const
{
    if (!m_settings) {
        m_settings = new DThemeSettings();

        qApp->setProperty("_d_theme_settings_object", (quintptr)m_settings);

        if (enabledRTScreenScale()) {
#ifdef QT_NO_DEBUG
            if (!qEnvironmentVariableIsSet("D_ENABLE_RT_SCALE"))
                return m_settings;
#endif

            QObject::connect(m_settings, &DThemeSettings::scaleFactorChanged,
                             m_settings, onScaleFactorChanged, Qt::UniqueConnection);
            QObject::connect(m_settings, &DThemeSettings::screenScaleFactorsChanged,
                             m_settings, onScreenScaleFactorsChanged, Qt::UniqueConnection);
            QObject::connect(m_settings, &DThemeSettings::scaleLogicalDpiChanged,
                             m_settings, updateScaleLogcailDpi, Qt::UniqueConnection);

            // ?????????connected??????Qt?????????????????????????????????dpi????????????????????????????????????????????????dpi??????
            // TODO(zccrs): ?????????????????????disconnectd??????Qt?????????????????????????????????????????????????????????
            //              ????????????Qt?????????????????????screen?????????????????????????????????screenAdded?????????
            //              ?????????????????????dpi?????????????????????????????????Qt api????????????????????????????????????
            //              ????????????????????????dpi???????????????????????????????????????????????????????????????
            //              !!!
            //              ?????? dtk ???????????????dxcb???????????????????????????dpi??????
            qApp->setProperty("_d_updateScaleLogcailDpi", (quintptr)&compelledUpdateScaleLogcailDpi);
            QObject::connect(qApp, &QGuiApplication::screenAdded,
                             m_settings, onScreenAdded,
                             Qt::ConnectionType(Qt::QueuedConnection | Qt::UniqueConnection));

            if (!qEnvironmentVariableIsSet(DISABLE_UPDATE_WINDOW_GEOMETRY)) {
                QObject::connect(m_settings, &DThemeSettings::autoScaleWindowChanged,
                                 m_settings, onAutoScaleWindowChanged, Qt::UniqueConnection);
                QObject::connect(m_settings, &DThemeSettings::screenScaleFactorsChanged,
                                 m_settings, onAutoScaleWindowChanged, Qt::UniqueConnection);

                onAutoScaleWindowChanged();
            }
        }
    }

    return m_settings;
}

DThemeSettings *QDeepinTheme::getSettings()
{
    return m_settings;
}

QT_END_NAMESPACE

#include "qdeepintheme.moc"
