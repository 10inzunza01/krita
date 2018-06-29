/*
 * Copyright (C) 2018 Boudewijn Rempt <boud@valdyas.org>
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

#include "KisFolderStorage.h"

#include <QDirIterator>
#include <KisMimeDatabase.h>
#include <kis_debug.h>

#include <KisResourceLoaderRegistry.h>

class FolderTagIterator : public KisResourceStorage::TagIterator
{
public:

    FolderTagIterator(const QString &location, const QString &resourceType)
        : m_location(location)
        , m_resourceType(resourceType)
    {
        m_dirIterator.reset(new QDirIterator(location + '/' + resourceType,
                                             QStringList() << "*.desktop",
                                             QDir::Files | QDir::Readable,
                                             QDirIterator::Subdirectories));
    }

    bool hasNext() const override { return m_dirIterator->hasNext(); }
    void next() const override { m_dirIterator->next(); }

    QString url() const override { return QString(); }
    QString name() const override { return QString(); }
    QString comment() const override {return QString(); }


private:

    QScopedPointer<QDirIterator> m_dirIterator;
    QString m_location;
    QString m_resourceType;
};


class FolderItem : public KisResourceStorage::ResourceItem
{
public:
    ~FolderItem() override {}

    QByteArray md5sum() const override
    {
        KisResourceLoaderBase *loader = KisResourceLoaderRegistry::instance()->loader(folder, KisMimeDatabase::mimeTypeForFile(url));
        if (loader) {
            QFile f(url);
            f.open(QFile::ReadOnly);
            KoResourceSP res = loader->load(url, f);
            f.close();
            if (res) {
                return res->md5();
            }
        }
        return QByteArray();
    }
};

class FolderIterator : public KisResourceStorage::ResourceIterator
{
public:
    FolderIterator(const QString &location, const QString &resourceType)
        : m_location(location)
        , m_resourceType(resourceType)
    {
        m_dirIterator.reset(new QDirIterator(location + '/' + resourceType,
                                             KisResourceLoaderRegistry::instance()->filters(resourceType),
                                             QDir::Files | QDir::Readable,
                                             QDirIterator::Subdirectories));
    }

    ~FolderIterator() override {}

    bool hasNext() const override
    {
        return m_dirIterator->hasNext();
    }

    void next() const override
    {
        m_dirIterator->next();
    }

    QString url() const override
    {
        return m_dirIterator->filePath();
    }

    QString type() const override
    {
        return m_resourceType;
    }

    QDateTime lastModified() const override
    {
        return m_dirIterator->fileInfo().lastModified();
    }

    QByteArray md5sum() const override
    {
        if (!loadResourceInternal()) {
            qWarning() << "Could not load resource" << m_dirIterator->filePath();
            return QByteArray();
        }
        return m_resource->md5();

    }

    KoResourceSP resource() const override
    {
        if (!loadResourceInternal()) {
            qWarning() << "Could not load resource" << m_dirIterator->filePath();
        }
        return m_resource;
    }

protected:

    bool loadResourceInternal() const {
        if (!m_resource || (m_resource && m_resource->filename() != m_dirIterator->filePath())) {
            QFile f(m_dirIterator->filePath());
            f.open(QFile::ReadOnly);
            if (!m_loader) {
                const_cast<FolderIterator*>(this)->m_loader = KisResourceLoaderRegistry::instance()->loader(m_resourceType, KisMimeDatabase::mimeTypeForFile(m_dirIterator->filePath()));
            }
            if (m_loader) {
                const_cast<FolderIterator*>(this)->m_resource = m_loader->load(m_dirIterator->filePath(), f);
            }
            f.close();
        }
        return !m_resource.isNull();
    }

    KisResourceLoaderBase *m_loader {0};
    KoResourceSP m_resource;
    QScopedPointer<QDirIterator> m_dirIterator;
    const QString m_location;
    const QString m_resourceType;
};


KisFolderStorage::KisFolderStorage(const QString &location)
    : KisStoragePlugin(location)
{
}

KisFolderStorage::~KisFolderStorage()
{
}

KisResourceStorage::ResourceItem KisFolderStorage::resourceItem(const QString &url)
{
    QFileInfo fi(url);
    FolderItem item;
    item.url = url;
    item.folder = fi.path().split("/").last();
    item.lastModified = fi.lastModified();
    return item;
}

KoResourceSP KisFolderStorage::resource(const QString &url)
{
    QFileInfo fi(url);
    const QString resourceType = fi.path().split("/").last();
    KisResourceLoaderBase *loader = KisResourceLoaderRegistry::instance()->get(resourceType);
    QFile f(url);
    f.open(QFile::ReadOnly);
    KoResourceSP res = loader->load(url, f);
    f.close();
    return res;
}

QSharedPointer<KisResourceStorage::ResourceIterator> KisFolderStorage::resources(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::ResourceIterator>(new FolderIterator(location(), resourceType));
}

QSharedPointer<KisResourceStorage::TagIterator> KisFolderStorage::tags(const QString &resourceType)
{
    return QSharedPointer<KisResourceStorage::TagIterator>(new FolderTagIterator(location(), resourceType));
}
