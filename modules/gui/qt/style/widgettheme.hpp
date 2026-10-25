/*****************************************************************************
 * Copyright (C) 2026 VLC authors and VideoLAN
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * ( at your option ) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/
#ifndef VLC_QT_WIDGETTHEME_HPP
#define VLC_QT_WIDGETTHEME_HPP

#include <QColor>

class QApplication;

/**
 * Applies the VLC redesign look (Figma "Dark/..." and "Light/..." styles) to
 * the Qt Widgets part of the interface: menus, the preferences and the other
 * classic dialogs. The QML part is themed by SystemPalette.
 */
namespace WidgetTheme
{
    /**
     * @param dark use the dark variant
     * @param customAccent accent color, invalid to use the default one
     */
    void apply(bool dark, const QColor& customAccent = QColor());

    /** whether the system (platform) palette is dark, captured at first call */
    bool systemIsDark();
}

#endif // VLC_QT_WIDGETTHEME_HPP
