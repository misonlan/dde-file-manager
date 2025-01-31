// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchreportdata.h"

#include <QDateTime>

QString SearchReportData::type() const
{
    return "Search";
}

QJsonObject SearchReportData::prepareData(const QVariantMap &args) const
{
    QVariantMap temArgs = args;
    temArgs.insert("tid", 1000500002);
    temArgs.insert("sysTime", QDateTime::currentDateTime().toTime_t());
    return QJsonObject::fromVariantMap(temArgs);
}
