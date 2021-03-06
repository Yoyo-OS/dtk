/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
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

#include "daboutdialog.h"
#include "private/daboutdialog_p.h"

#include <dwidgetutil.h>
#include <DSysInfo>

#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QKeyEvent>
#include <QApplication>
#include <QImageReader>
#include <DSysInfo>
#include <QScrollArea>

#ifdef Q_OS_UNIX
#include <unistd.h>
#include <pwd.h>
#endif

DCORE_USE_NAMESPACE
DWIDGET_BEGIN_NAMESPACE

#ifdef Q_OS_UNIX
class EnvReplaceGuard
{
public:
    EnvReplaceGuard(const int uid);
    ~EnvReplaceGuard();

    char *m_backupLogName;
    char *m_backupHome;
};

EnvReplaceGuard::EnvReplaceGuard(const int uid)
{
    m_backupLogName = getenv("LOGNAME");
    m_backupHome = getenv("HOME");

    struct passwd *pwd = getpwuid(uid);

    setenv("LOGNAME", pwd->pw_name, 1);
    setenv("HOME", pwd->pw_dir, 1);
}

EnvReplaceGuard::~EnvReplaceGuard()
{
    setenv("LOGNAME", m_backupLogName, 1);
    setenv("HOME", m_backupHome, 1);
}
#endif

const QString DAboutDialogPrivate::websiteLinkTemplate = "<a href='%1' style='text-decoration: none; font-size:13px; color: #004EE5;'>%2</a>";

DAboutDialogPrivate::DAboutDialogPrivate(DAboutDialog *qq)
    : DDialogPrivate(qq)
{

}

void DAboutDialogPrivate::init()
{
    D_Q(DAboutDialog);

    q->setMinimumWidth(360);

    // overwrite default info if distribution config file existed.
    loadDistributionInfo();

    logoLabel = new QLabel();
    logoLabel->setContentsMargins(0, 0, 0, 0);

    productNameLabel = new QLabel();
    productNameLabel->setObjectName("ProductNameLabel");

    versionLabel = new QLabel();
    versionLabel->setObjectName("VersionLabel");

    companyLogoLabel = new QLabel();
    companyLogoLabel->setPixmap(loadPixmap(logoPath));

    websiteLabel = new QLabel();
    websiteLabel->setObjectName("WebsiteLabel");
    websiteLabel->setContextMenuPolicy(Qt::NoContextMenu);
    websiteLabel->setOpenExternalLinks(false);
    updateWebsiteLabel();

    acknowledgementLabel = new QLabel();
    acknowledgementLabel->setObjectName("AcknowledgementLabel");
    acknowledgementLabel->setContextMenuPolicy(Qt::NoContextMenu);
    acknowledgementLabel->setOpenExternalLinks(false);
    updateAcknowledgementLabel();

    descriptionLabel = new QLabel();
    descriptionLabel->setObjectName("DescriptionLabel");
    descriptionLabel->setAlignment(Qt::AlignHCenter);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    licenseLabel = new QLabel();
    licenseLabel->setObjectName("LicenseLabel");
    licenseLabel->setAlignment(Qt::AlignHCenter);
    licenseLabel->setWordWrap(true);
    licenseLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    licenseLabel->hide();

    q->connect(websiteLabel, SIGNAL(linkActivated(QString)), q, SLOT(_q_onLinkActivated(QString)));
    q->connect(acknowledgementLabel, SIGNAL(linkActivated(QString)), q, SLOT(_q_onLinkActivated(QString)));
    q->connect(descriptionLabel, SIGNAL(linkActivated(QString)), q, SLOT(_q_onLinkActivated(QString)));
    q->connect(licenseLabel, SIGNAL(linkActivated(QString)), q, SLOT(_q_onLinkActivated(QString)));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(11, 20, 11, 10);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(logoLabel);
    mainLayout->setAlignment(logoLabel, Qt::AlignCenter);
    mainLayout->addSpacing(3);
    mainLayout->addWidget(productNameLabel);
    mainLayout->setAlignment(productNameLabel, Qt::AlignCenter);
    mainLayout->addSpacing(6);
    mainLayout->addWidget(versionLabel);
    mainLayout->setAlignment(versionLabel, Qt::AlignCenter);
    mainLayout->addSpacing(8);
    mainLayout->addWidget(companyLogoLabel);
    mainLayout->setAlignment(companyLogoLabel, Qt::AlignCenter);
//    mainLayout->addSpacing(6);
    mainLayout->addWidget(websiteLabel);
    mainLayout->setAlignment(websiteLabel, Qt::AlignCenter);
    mainLayout->addSpacing(5);
//    mainLayout->addWidget(acknowledgementLabel);
//    mainLayout->setAlignment(acknowledgementLabel, Qt::AlignCenter);
    mainLayout->addSpacing(12);
    mainLayout->addWidget(descriptionLabel, Qt::AlignHCenter);
    mainLayout->addSpacing(7);
    mainLayout->addWidget(licenseLabel, Qt::AlignHCenter);

    QScrollArea *mainScrollArea = new QScrollArea;
    QWidget  *mainContent = new QWidget;
    QPalette scrollPalette;

    scrollPalette.setBrush(QPalette::Background, Qt::transparent);
    mainScrollArea->setFrameShape(QFrame::NoFrame);
    mainScrollArea->setWidget(mainContent);
    mainScrollArea->setWidgetResizable(true);
    mainScrollArea->setPalette(scrollPalette);

    mainContent->setLayout(mainLayout);
    q->addContent(mainScrollArea);

    // make active
    q->setFocus();
}

void DAboutDialogPrivate::loadDistributionInfo()
{
    logoPath = DSysInfo::distributionOrgLogo(DSysInfo::Distribution, DSysInfo::Light, ":/assets/images/deepin-logo.svg");
    auto websiteInfo = DSysInfo::distributionOrgWebsite(DSysInfo::Distribution); // will always return a valid result.
    websiteName = websiteInfo.first;
    websiteLink = websiteInfo.second;
}

void DAboutDialogPrivate::updateWebsiteLabel()
{
    QString websiteText = QString(websiteLinkTemplate).arg(websiteLink).arg(websiteName);
    websiteLabel->setText(websiteText);
}

void DAboutDialogPrivate::updateAcknowledgementLabel()
{
    QString acknowledgementText = QString(websiteLinkTemplate).arg(acknowledgementLink).arg(QApplication::translate("DAboutDialog", "Acknowledgements"));
    acknowledgementLabel->setText(acknowledgementText);
}

void DAboutDialogPrivate::_q_onLinkActivated(const QString &link)
{
#ifdef Q_OS_UNIX
    // workaround for pkexec apps
    bool ok = false;
    const int pkexecUid = qEnvironmentVariableIntValue("PKEXEC_UID", &ok);

    if (ok)
    {
        EnvReplaceGuard _env_guard(pkexecUid);
        Q_UNUSED(_env_guard);

        QDesktopServices::openUrl(QUrl(link));
    }
    else
#endif
    {
        QDesktopServices::openUrl(QUrl(link));
    }
}

QPixmap DAboutDialogPrivate::loadPixmap(const QString &file)
{
    D_Q(DAboutDialog);

    qreal ratio = 1.0;

    const qreal devicePixelRatio = q->devicePixelRatioF();

    QPixmap pixmap;

    if (!qFuzzyCompare(ratio, devicePixelRatio)) {
        QImageReader reader;
        reader.setFileName(qt_findAtNxFile(file, devicePixelRatio, &ratio));
        if (reader.canRead()) {
            reader.setScaledSize(reader.size() * (devicePixelRatio / ratio));
            pixmap = QPixmap::fromImage(reader.read());
            pixmap.setDevicePixelRatio(devicePixelRatio);
        }
    } else {
        pixmap.load(file);
    }

    return pixmap;
}

/*!
  \class Dtk::Widget::DAboutDialog
  \inmodule dtkwidget
  \brief DAboutDialog ????????????????????????????????????????????????????????? deepin ??????????????????????????????????????? Deepin ??????.
  
  ?????? DMainWindow ?????????????????????????????????????????????????????????????????????????????????????????????
  
  ???????????????????????????????????? DApplication ???????????????????????????????????????
 */

DAboutDialog::DAboutDialog(QWidget *parent)
    : DDialog(*new DAboutDialogPrivate(this), parent)
{
    D_D(DAboutDialog);

    d->init();
}

/*!
  \property DAboutDialog::windowTitle
  
  \brief the title of the dialog.
  \brief ????????????????????????????????????.
 */
QString DAboutDialog::windowTitle() const
{
    return title();
}

/*!
  \property DAboutDialog::productName
  
  \brief the product name to be shown on the dialog.
  \brief ????????????????????????????????????.
 */
QString DAboutDialog::productName() const
{
    D_DC(DAboutDialog);

    return d->productNameLabel->text();
}

/*!
  \property DAboutDialog::version
  
  \brief the version number to be shown on the dialog.
  \brief ????????????????????????????????????.
 */
QString DAboutDialog::version() const
{
    D_DC(DAboutDialog);

    return d->versionLabel->text();
}

/*!
  \property DAboutDialog::description

  \brief the description to be show on the dialog.
  \brief ????????????????????????????????????.
 */
QString DAboutDialog::description() const
{
    D_DC(DAboutDialog);

    return d->descriptionLabel->text();
}

/*!
  \brief the vendor logo to be shown on the dialog.
  \return ???????????????????????????/?????? logo ??????.
 */
const QPixmap *DAboutDialog::companyLogo() const
{
    D_DC(DAboutDialog);

    return d->companyLogoLabel->pixmap();
}

/*!
  \property DAboutDialog::websiteName
  \brief the vendor website name to be shown on the dialog.
  \brief ?????????????????????????????????/??????????????????.
  
  Usually be in form like www.deepin.org.
  ???????????? www.deepin.org ????????????
 */
QString DAboutDialog::websiteName() const
{
    D_DC(DAboutDialog);

    return d->websiteName;
}

/*!
  \property DAboutDialog::websiteLink
  \brief the corresponding web address of websiteName().
  \brief ?????? websiteName() ???????????????.
  
  The website link will be open in the browser if the user clicks on
  the website text shown on the dialog.
  ??????????????????????????????????????????????????????????????????????????????
 */
QString DAboutDialog::websiteLink() const
{
    D_DC(DAboutDialog);

    return d->websiteLink;
}

/*!
  \property DAboutDialog::acknowledgementLink

  \brief the web address to be open open when user clicks on the "Acknowlegement"
  text show on the dialog.
  \brief ????????????????????????.
 */
QString DAboutDialog::acknowledgementLink() const
{
    D_DC(DAboutDialog);

    return d->acknowledgementLink;
}

/*!
  \property DAboutDialog::license

  \brief the license to be shown on the dialog.
  \brief ???????????????????????????.
 */
QString DAboutDialog::license() const
{
    D_DC(DAboutDialog);

    return d->licenseLabel->text();
}

/*!
  \brief ???????????????????????????.

  \a windowTitle ?????????????????????.
 */
void DAboutDialog::setWindowTitle(const QString &windowTitle)
{
    setTitle(windowTitle);
}

/*!
  \brief ??????????????? \a icon ??????.
  
  ?????????????????????????????????.
 */
void DAboutDialog::setProductIcon(const QIcon &icon)
{
    D_D(DAboutDialog);

    d->logoLabel->setPixmap(icon.pixmap(windowHandle(), QSize(96, 96)));
}

/*!
  \brief ??????????????????.
  \a productName ????????????.
 */
void DAboutDialog::setProductName(const QString &productName)
{
    D_D(DAboutDialog);

    d->productNameLabel->setText(productName);
}

/*!
  \brief ?????????????????????????????? \a version ????????????.
 */
void DAboutDialog::setVersion(const QString &version)
{
    D_D(DAboutDialog);

    d->versionLabel->setText(version);
}

/*!
  \brief ?????????????????????????????? \a description ????????????.
 */
void DAboutDialog::setDescription(const QString &description)
{
    D_D(DAboutDialog);

    d->descriptionLabel->setText(description);
}

/*!
  \brief ?????????????????????????????? \a companyLogo ????????????.
 */
void DAboutDialog::setCompanyLogo(const QPixmap &companyLogo)
{
    D_D(DAboutDialog);

    d->companyLogoLabel->setPixmap(companyLogo);
}

/*!
  \brief ?????????????????????????????? \a websiteName ????????????
 */
void DAboutDialog::setWebsiteName(const QString &websiteName)
{
    D_D(DAboutDialog);

    if (d->websiteName == websiteName) {
        return;
    }

    d->websiteName = websiteName;
    d->updateWebsiteLabel();
}

/*!
  \brief ?????????????????????????????? \a websiteLink ????????????
 */
void DAboutDialog::setWebsiteLink(const QString &websiteLink)
{
    D_D(DAboutDialog);

    if (d->websiteLink == websiteLink) {
        return;
    }

    d->websiteLink = websiteLink;
    d->updateWebsiteLabel();
}

/*!
  \brief ?????????????????????????????? \a acknowledgementLink ????????????
 */
void DAboutDialog::setAcknowledgementLink(const QString &acknowledgementLink)
{
    D_D(DAboutDialog);

    d->acknowledgementLink = acknowledgementLink;
    d->updateAcknowledgementLabel();
}

/*!
  \brief ?????????????????????????????? \a visible ??????????????????????????????
 */
void DAboutDialog::setAcknowledgementVisible(bool visible)
{
    Q_UNUSED(visible)
    D_D(DAboutDialog);
//    d->acknowledgementLabel->setVisible(visible);
}

/*!
  \brief ?????????????????????????????? \a license ?????????.
 */
void DAboutDialog::setLicense(const QString &license)
{
    D_D(DAboutDialog);

    d->licenseLabel->setText(license);
    d->licenseLabel->setVisible(!license.isEmpty());
}

void DAboutDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
        event->accept();
    }

    DDialog::keyPressEvent(event);
}

void DAboutDialog::showEvent(QShowEvent *event)
{
    DDialog::showEvent(event);

    if (minimumWidth() == maximumWidth()) {
        resize(width(), heightForWidth(width()));
    } else {
        adjustSize();
    }
}

DWIDGET_END_NAMESPACE

#include "moc_daboutdialog.cpp"
