/*
 * Copyright (C) 2017 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "TestResourceCacheDb.h"
#include <QTest>
#include <QtSql>
#include <QStandardPaths>
#include <QDir>

#include <KisResourceCacheDb.h>
#include <KisResourceLoaderRegistry.h>

void TestResourceCacheDb::initTestCase()
{
    QDir dbLocation(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if (dbLocation.exists()) {
        QFile(dbLocation.path() + "/" + KisResourceCacheDb::resourceCacheDbFilename).remove();
        dbLocation.rmpath(dbLocation.path());
    }
}

void TestResourceCacheDb::testCreateDatabase()
{
    bool res = KisResourceCacheDb::initialize(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QVERIFY(res);
    QVERIFY(KisResourceCacheDb::isValid());

    QSqlDatabase sqlDb = QSqlDatabase::database();

    QStringList tables = QStringList() << "version_information"
                                       << "storage_types"
                                       << "resource_types"
                                       << "storages"
                                       << "tags"
                                       << "resources"
                                       << "versioned_resources"
                                       << "resource_tags";
    QStringList dbTables = sqlDb.tables();

    Q_FOREACH(const QString &table, tables) {
        QVERIFY2(dbTables.contains(table), table.toLatin1());
    }

    res = KisResourceCacheDb::initialize(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QVERIFY(res);
    QVERIFY(KisResourceCacheDb::isValid());
}

void TestResourceCacheDb::testLookupTables()
{
    QSqlQuery query;
    bool r = query.exec("SELECT COUNT(*) FROM storage_types");
    QVERIFY(r);
    QVERIFY(query.lastError() == QSqlError());
    query.first();
    QVERIFY(query.value(0).toInt() == 5);

    r = query.exec("SELECT COUNT(*) FROM resource_types");
    QVERIFY(r);
    QVERIFY(query.lastError() == QSqlError());
    query.first();
    QVERIFY(query.value(0).toInt() == KisResourceLoaderRegistry::instance()->resourceTypes().count());
}

void TestResourceCacheDb::cleanupTestCase()
{
    QDir dbLocation(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    bool res = QFile(dbLocation.path() + "/" + KisResourceCacheDb::resourceCacheDbFilename).remove();
    Q_ASSERT(res);
    res = dbLocation.rmpath(dbLocation.path());
    Q_ASSERT(res);
}

QTEST_MAIN(TestResourceCacheDb)

