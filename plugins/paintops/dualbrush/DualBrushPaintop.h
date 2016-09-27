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

#ifndef KIS_DUALBRUSH_PAINTOP_H_
#define KIS_DUALBRUSH_PAINTOP_H_

#include <brushengine/kis_paintop.h>
#include <kis_types.h>
#include "DualBrushPaintopSettings.h"
#include "StackedPreset.h"

class KisPainter;

class DualBrushPaintOp : public KisPaintOp
{

public:

    DualBrushPaintOp(KisPaintOpSettingsSP settings, KisPainter *painter, KisNodeSP node, KisImageSP image);
    virtual ~DualBrushPaintOp();

    KisSpacingInformation paintAt(const KisPaintInformation& info);
    void paintLine(const KisPaintInformation &pi1,
                           const KisPaintInformation &pi2,
                           KisDistanceInformation *currentDistance);
    void paintBezierCurve(const KisPaintInformation &pi1,
                                  const QPointF &control1,
                                  const QPointF &control2,
                                  const KisPaintInformation &pi2,
                                  KisDistanceInformation *currentDistance);
private:
    QVector<KisDistanceInformation*> effectiveDistances(KisDistanceInformation *distance);

private:
    KisPaintDeviceSP m_dab;
    KisPressureOpacityOption m_opacityOption;
    QVector<KisPaintOp*> m_paintopStack;
    QVector<StackedPreset> m_presetStack;
    QVector<KisDistanceInformation> m_distancesStore;

};

#endif // KIS_DUALBRUSH_PAINTOP_H_
