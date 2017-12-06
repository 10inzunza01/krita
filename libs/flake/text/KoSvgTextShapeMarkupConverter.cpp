/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KoSvgTextShapeMarkupConverter.h"

#include "klocalizedstring.h"
#include "kis_assert.h"
#include "kis_debug.h"

#include <QBuffer>
#include <KoSvgTextShape.h>

#include <SvgWriter.h>
#include <SvgSavingContext.h>
#include <html/HtmlSavingContext.h>
#include <html/HtmlWriter.h>

#include <KoXmlReader.h>
#include <SvgParser.h>
#include <KoDocumentResourceManager.h>


struct KoSvgTextShapeMarkupConverter::Private {
    Private(KoSvgTextShape *_shape) : shape(_shape) {}

    KoSvgTextShape *shape;

    QStringList errors;
    QStringList warnings;

    void clearErrors() {
        errors.clear();
        warnings.clear();
    }
};

KoSvgTextShapeMarkupConverter::KoSvgTextShapeMarkupConverter(KoSvgTextShape *shape)
    : m_d(new Private(shape))
{
}

KoSvgTextShapeMarkupConverter::~KoSvgTextShapeMarkupConverter()
{
}

bool KoSvgTextShapeMarkupConverter::convertToSvg(QString *svgText, QString *stylesText)
{
    m_d->clearErrors();

    QBuffer shapesBuffer;
    QBuffer stylesBuffer;

    shapesBuffer.open(QIODevice::WriteOnly);
    stylesBuffer.open(QIODevice::WriteOnly);

    {
        SvgSavingContext savingContext(shapesBuffer, stylesBuffer);
        savingContext.setStrippedTextMode(true);
        SvgWriter writer({m_d->shape});
        writer.saveDetached(savingContext);
    }

    shapesBuffer.close();
    stylesBuffer.close();

    *svgText = QString(shapesBuffer.data());
    *stylesText = QString(stylesBuffer.data());

    return true;
}

bool KoSvgTextShapeMarkupConverter::convertFromSvg(const QString &svgText, const QString &stylesText,
                                                   const QRectF &boundsInPixels, qreal pixelsPerInch)
{

    m_d->clearErrors();

    KoXmlDocument doc;
    QString errorMessage;
    int errorLine = 0;
    int errorColumn = 0;

    const QString fullText = QString("<svg>\n%1\n%2\n</svg>\n").arg(stylesText).arg(svgText);

    if (!doc.setContent(fullText, &errorMessage, &errorLine, &errorColumn)) {
        m_d->errors << QString("line %1, col %2: %3").arg(errorLine).arg(errorColumn).arg(errorMessage);
        return false;
    }

    m_d->shape->resetTextShape();

    KoDocumentResourceManager resourceManager;
    SvgParser parser(&resourceManager);
    parser.setResolution(boundsInPixels, pixelsPerInch);

    KoXmlElement root = doc.documentElement();
    KoXmlNode node = root.firstChild();

    bool textNodeFound = false;

    for (; !node.isNull(); node = node.nextSibling()) {
        KoXmlElement el = node.toElement();
        if (el.isNull()) continue;

        if (el.tagName() == "defs") {
            parser.parseDefsElement(el);
        } else if (el.tagName() == "text") {
            if (textNodeFound) {
                m_d->errors << i18n("More than one 'text' node found!");
                return false;
            }


            KoShape *shape = parser.parseTextElement(el, m_d->shape);
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(shape == m_d->shape, false);
            textNodeFound = true;
            break;
        } else {
            m_d->errors << i18n("Unknown node of type \'%1\' found!", el.tagName());
            return false;
        }
    }

    if (!textNodeFound) {
        m_d->errors << i18n("No \'text\' node found!");
        return false;
    }

    return true;

}

bool KoSvgTextShapeMarkupConverter::convertToHtml(QString *htmlText)
{
    m_d->clearErrors();

    QBuffer shapesBuffer;
    shapesBuffer.open(QIODevice::WriteOnly);
    {
        HtmlWriter writer({m_d->shape});
        if (!writer.save(shapesBuffer)) {
            m_d->errors = writer.errors();
            m_d->warnings = writer.warnings();
            return false;
        }
    }

    shapesBuffer.close();

    *htmlText = QString(shapesBuffer.data());

    return true;
}

bool KoSvgTextShapeMarkupConverter::convertFromHtml(const QString &htmlText)
{
    m_d->clearErrors();

    KoXmlDocument doc;
    QString errorMessage;
    int errorLine = 0;
    int errorColumn = 0;

    const bool xmlResult = doc.setContent(htmlText, &errorMessage, &errorLine, &errorColumn);
    if (!xmlResult) {
        m_d->errors << QString("line %1, col %2: %3").arg(errorLine).arg(errorColumn).arg(errorMessage);
        return false;
    }

    m_d->shape->resetTextShape();
    KoXmlElement root = doc.documentElement();

    return m_d->shape->loadHtml(root);
}

QStringList KoSvgTextShapeMarkupConverter::errors() const
{
    return m_d->errors;
}

QStringList KoSvgTextShapeMarkupConverter::warnings() const
{
    return m_d->warnings;
}

