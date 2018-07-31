/*
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
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


#ifndef PALETTEDOCKER_DOCK_H
#define PALETTEDOCKER_DOCK_H

#include <QDockWidget>
#include <QModelIndex>
#include <QScopedPointer>
#include <QVector>
#include <QPointer>
#include <QPair>
#include <QAction>
#include <QMenu>

#include <KoCanvasObserverBase.h>
#include <KoResourceServerAdapter.h>
#include <KoResourceServerObserver.h>
#include <resources/KoColorSet.h>

#include <kis_canvas2.h>
#include <kis_mainwindow_observer.h>
#include <KisView.h>

#include "PaletteListSaver.h"

class KisViewManager;
class KisCanvasResourceProvider;
class KisWorkspaceResource;
class KisPaletteListWidget;
class KisPaletteModel;
class Ui_WdgPaletteDock;

class PaletteDockerDock : public QDockWidget, public KisMainwindowObserver
{
    Q_OBJECT
public:
    PaletteDockerDock();
    ~PaletteDockerDock() override;

public: // QDockWidget
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

public: // KisMainWindowObserver
    void setViewManager(KisViewManager* kisview) override;

Q_SIGNALS:
    void sigPaletteModified(const KoColorSet *);

private Q_SLOTS:
    void slotImportPalette();
    void slotAddColor();
    void slotRemoveColor();
    void slotEditEntry();

    void slotPaletteIndexSelected(const QModelIndex &index);
    void slotSetEntryByForeground(const QModelIndex &index);
    void slotSetFGColorByPalette(const KisSwatch &entry);
    void slotNameListSelection(const KoColor &color);

    void slotSetColorSet(KoColorSet* colorSet);

    void saveToWorkspace(KisWorkspaceResource* workspace);
    void loadFromWorkspace(KisWorkspaceResource* workspace);

private /* friends */:
    /**
     * @brief PaletteListSaver
     * saves non-global palette list to KisDocument.
     * Actually, this should be implemented in
     * KisPaletteListWidget, but that class is in the
     * library kritawidgets, while KisDocument is in
     * kritaui, which depends on kritawidgets...
     *
     * Hope one day kritaui can finally be cleaned up...
     */
    friend class PaletteListSaver;

private /* member variables */:
    Ui_WdgPaletteDock* m_ui;
    KisPaletteModel *m_model;
    KoColorSet *m_currentColorSet;
    KisPaletteListWidget *m_paletteChooser;
    KisViewManager *m_view;
    KisCanvasResourceProvider *m_resourceProvider;
    QPointer<KisCanvas2> m_canvas;
    QScopedPointer<PaletteListSaver> m_saver;

    QScopedPointer<QAction> m_actAdd;
    QScopedPointer<QAction> m_actAddWithDlg;
    QScopedPointer<QAction> m_actSwitch;
    QScopedPointer<QAction> m_actRemove;
    QScopedPointer<QAction> m_actModify;
    QMenu m_viewContextMenu;
};


#endif
