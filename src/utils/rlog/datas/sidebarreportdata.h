// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIDEBARREPORTDATA_H
#define SIDEBARREPORTDATA_H

#include "reportdatainterface.h"

#include <QObject>
#include <QDateTime>

class SidebarReportData : public ReportDataInterface
{
public:
    QString type() const override;
    QJsonObject prepareData(const QVariantMap &args) const override;
};

#endif   // SIDEBARREPORTDATA_H
