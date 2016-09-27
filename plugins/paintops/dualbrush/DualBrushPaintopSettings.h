/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_DUALBRUSH_PAINTOP_SETTINGS_H_
#define KIS_DUALBRUSH_PAINTOP_SETTINGS_H_

#include <brushengine/kis_paintop_settings.h>
#include <kis_types.h>

#include "DualBrushPaintopSettingsWidget.h"

#include <kis_pressure_opacity_option.h>


class DualBrushPaintOpSettings : public KisPaintOpSettings
{

public:
    DualBrushPaintOpSettings();
    virtual ~DualBrushPaintOpSettings() {}

    QPainterPath brushOutline(const KisPaintInformation &info, OutlineMode mode);

    bool paintIncremental();
    bool isAirbrushing() const;
    int rate() const;

    virtual void setPaintOpSize(qreal value);
    virtual qreal paintOpSize() const;

};

typedef KisSharedPtr<DualBrushPaintOpSettings> DualBrushPaintOpSettingsSP;

#endif
