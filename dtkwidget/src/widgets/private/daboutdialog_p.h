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

#ifndef DABOUTDIALOG_P_H
#define DABOUTDIALOG_P_H

#include <DAboutDialog>
#include "ddialog_p.h"

DWIDGET_BEGIN_NAMESPACE

class DAboutDialogPrivate : public DDialogPrivate
{
public:
    DAboutDialogPrivate(DAboutDialog *qq);

    void init();
    void loadDistributionInfo();
    void updateWebsiteLabel();
    void updateAcknowledgementLabel();
    void _q_onLinkActivated(const QString &link);
    QPixmap loadPixmap(const QString &file);

    static const QString websiteLinkTemplate;

    QPixmap windowIcon;
    QLabel *logoLabel = nullptr;
    QLabel *productNameLabel = nullptr;
    QLabel *versionLabel = nullptr;
    QLabel *descriptionLabel = nullptr;
    QLabel *licenseLabel = nullptr;
    QLabel *companyLogoLabel = nullptr;
    QLabel *websiteLabel = nullptr;
    QLabel *acknowledgementLabel = nullptr;

    QString logoPath;
    QString websiteName;
    QString websiteLink;
    QString acknowledgementLink;

    Q_DECLARE_PUBLIC(DAboutDialog)
};

DWIDGET_END_NAMESPACE

#endif // DABOUTDIALOG_P_H
