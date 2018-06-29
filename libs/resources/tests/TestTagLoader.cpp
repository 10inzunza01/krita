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

#include "TestTagLoader.h"
#include <QTest>

#include <KisTagLoader.h>
#include <KisResourceLoader.h>
#include <KoResource.h>
#include <KisResourceLoaderRegistry.h>

#ifndef FILES_DATA_DIR
#error "FILES_DATA_DIR not set. A directory with the data used for testing installing resources"
#endif

void TestTagLoader ::testLoadTag()
{
    KisTagLoader tagLoader;
    QFile f(QString(FILES_DATA_DIR) + "/paintoppresets/test.deskop");
    f.open(QFile::ReadOnly);

    {
        bool r = tagLoader.load(f);
        f.close();
        QVERIFY(r);
        QVERIFY(tagLoader.name() == "* Favorites");
        QVERIFY(tagLoader.comment() == "Your favorite brush presets");
        QVERIFY(tagLoader.url() == "* Favorites");
    }

    QLocale nl(QLocale::Dutch, QLocale::Netherlands);
    QLocale::setDefault(nl);

    {
        bool r = tagLoader.load(f);
        f.close();
        QVERIFY(r);
        QVERIFY(tagLoader.name() == "* Favorieten");
        QVERIFY(tagLoader.comment() == "Jouw favoriete penseel presets");
        QVERIFY(tagLoader.url() == "* Favorites");
    }
}

QTEST_MAIN(TestTagLoader)

