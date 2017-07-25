/*
 *  Copyright (c) 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef LIBKIS_PALETTE_H
#define LIBKIS_PALETTE_H

#include <QObject>
#include "kritalibkis_export.h"
#include "libkis.h"
#include "Resource.h"
#include "KoColorSet.h"

class ManagedColor;


/**
 * @brief The Palette class
 * Palette is a resource object that stores organised color data.
 * It's purpose is to allow artists to save colors and store them.
 *
 * An example for printing all the palettes and the entries:
 *
 * @code
import sys
from krita import *

resources = Application.resources("palette")

for (k, v) in resources.items():
    print(k)
    palette = Palette(v)
    for x in range(palette.numberOfEntries()):
        entry = palette.colorSetEntryByIndex(x)
        c = palette.colorForEntry(entry);
        print(x, entry.name, entry.id, entry.spotColor, c.toQString())
 * @endcode
 */

class KRITALIBKIS_EXPORT Palette : public QObject
{
public:
    Palette(Resource *resource);
    ~Palette() override;

    /**
     * @brief numberOfEntries
     * @return
     */
    int numberOfEntries() const;

    /**
     * @brief columnCount
     * @return the amount of columns this palette is set to use.
     */
    int columnCount();
    /**
     * @brief setColumnCount
     * Set the amount of columns this palette should use.
     */
    void setColumnCount(int columns);
    /**
     * @brief comment
     * @return the comment or description associated with the palette.
     */
    QString comment();
    /**
     * @brief setComment
     * set the comment or description associated with the palette.
     * @param comment
     */
    void setComment(QString comment);
    /**
     * @brief groupNames
     * @return the list of group names. This is list is in the order these groups are in the file.
     */
    QStringList groupNames();
    /**
     * @brief addGroup
     * @param name of the new group
     * @return whether adding the group was succesful.
     */
    bool addGroup(QString name);
    /**
     * @brief removeGroup
     * @param name the name of the group to remove.
     * @param keepColors whether or not to delete all the colors inside, or to move them to the default group.
     * @return
     */
    bool removeGroup(QString name, bool keepColors = true);

    /**
     * @brief colorsCountTotal
     * @return the total amount of entries in the whole group
     */
    int colorsCountTotal();
    /**
     * @brief colorsCountGroup
     * @param name of the group to check. Empty is the default group.
     * @return the amount of colors within that group.
     */
    int colorsCountGroup(QString name);

    KoColorSetEntry colorSetEntryByIndex(int index);
    KoColorSetEntry colorSetEntryFromGroup(int index, const QString &groupName);

    ManagedColor *colorForEntry(KoColorSetEntry entry);

    //getcolorgroup
    //Add
    //Remove
    //Insert

private:
    friend class PaletteView;
    struct Private;
    Private *const d;

    /**
     * @brief colorSet
     * @return gives qa KoColorSet object back
     */
    KoColorSet *colorSet();

};

#endif // LIBKIS_PALETTE_H
