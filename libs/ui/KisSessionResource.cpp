/*
 *  Copyright (c) 2018 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "KisSessionResource.h"

#include <QDomElement>

#include <KisPart.h>
#include <kis_properties_configuration.h>
#include <KisDocument.h>

struct KisSessionResource::Private
{
    struct View
    {
        QUuid windowId;
        QUrl file;
        KisPropertiesConfiguration viewConfig;

        KisMainWindow *getWindow() const {
            Q_FOREACH(KisMainWindow *window, KisPart::instance()->mainWindows()) {
                if (window->id() == this->windowId) return window;
            }

            return nullptr;
        }
    };

    QVector<View> views;
};

KisSessionResource::KisSessionResource(const QString &filename)
        : KisWindowLayoutResource(filename)
          , d(new Private)
{}

KisSessionResource::~KisSessionResource()
{}

void KisSessionResource::restore()
{
    auto *kisPart = KisPart::instance();

    applyLayout();

    QMap<QUrl, KisDocument *> documents;

    Q_FOREACH(auto &viewData, d->views) {
        QUrl url = viewData.file;

        KisMainWindow *window = viewData.getWindow();

        if (!window) {
            qDebug() << "Warning: session file contains inconsistent data.";
        } else {
            auto document = documents.value(url);

            if (!document) {
                document = kisPart->createDocument();
                kisPart->addDocument(document);
                documents.insert(url, document);

                bool ok = document->openUrl(url);
                // TODO: warn if failing to re-open document
            }

            KisView *view = window->newView(document);
            view->restoreViewState(viewData.viewConfig);
        }
    }

    kisPart->setCurrentSession(this);
}


QString KisSessionResource::defaultFileExtension() const
{
    return ".ksn";
}

void KisSessionResource::storeCurrentWindows()
{
    KisPart *kisPart = KisPart::instance();
    const auto &windows = kisPart->mainWindows();
    setWindows(windows);

    d->views.clear();
    Q_FOREACH(const KisView *view, kisPart->views()) {
        auto viewData = Private::View();
        viewData.windowId = view->mainWindow()->id();
        viewData.file = view->document()->url();
        view->saveViewState(viewData.viewConfig);
        d->views.append(viewData);
    }

    setValid(true);
}

void KisSessionResource::saveXml(QDomDocument &doc, QDomElement &root) const
{
    KisWindowLayoutResource::saveXml(doc, root);

    Q_FOREACH(const auto view, d->views) {
        QDomElement elem = doc.createElement("view");

        elem.setAttribute("window", view.windowId.toString());
        elem.setAttribute("src", view.file.toString());
        view.viewConfig.toXML(doc, elem);

        root.appendChild(elem);
    }
}

void KisSessionResource::loadXml(const QDomElement &root) const
{
    KisWindowLayoutResource::loadXml(root);

    d->views.clear();
    for (auto viewElement = root.firstChildElement("view");
         !viewElement.isNull();
         viewElement = viewElement.nextSiblingElement("view")) {
        Private::View view;

        view.file = QUrl(viewElement.attribute("src"));
        view.windowId = QUuid(viewElement.attribute("window"));
        view.viewConfig.fromXML(viewElement);

        d->views.append(view);
    }
}
