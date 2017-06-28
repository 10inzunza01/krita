/* This file is part of the KDE project
 *
 * Copyright 2017 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef TEXTNGSHAPECONFIGWIDGET_H
#define TEXTNGSHAPECONFIGWIDGET_H

#include <QWidget>

#include "ui_WdgSvgTextEditor.h"

#include <KoDialog.h>
#include <KoColor.h>

class KoSvgTextShape;
class KisFileNameRequester;

class SvgTextEditor : public KoDialog
{
    Q_OBJECT
public:
    SvgTextEditor(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~SvgTextEditor();

    void setShape(KoSvgTextShape *shape);

private Q_SLOTS:

    void save();

    void setTextBold();
    void setTextItalic();
    void setTextUnderline();
    void setTextStrikeThrough();
    void setTextFill();
    void setTextStroke();
    void setFont();
    void setSize();
    void setSuperscript();
    void setSubscript();

Q_SIGNALS:

    void textUpdated(const QString &svg, const QString &defs);
protected:
    void wheelEvent(QWheelEvent *event) override;
private:
    Ui_WdgSvgTextEditor widget;
    QWidget *m_page;
    KoSvgTextShape *m_shape;
};

#endif //TEXTNGSHAPECONFIGWIDGET_H
