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
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "widgettheme.hpp"

#include <QApplication>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>

namespace {

// Tokens of the VLC redesign (Figma "Dark/..." and "Light/..." styles).
// Keep in sync with SystemPalette.
struct Tokens
{
    QColor bgA1, bgA2, bgA3, bgA4, bgA5, bgB1, bgB2;
    QColor fg1, fg2, fgA2, fgA3, fgA4, fgB1;
    QColor dividerBright, dividerDim;
    QColor accent, accentBright, accentBg;
};

Tokens darkTokens(const QColor& customAccent)
{
    Tokens t;
    t.bgA1 = QColor("#222222"); t.bgA2 = QColor("#1F1F1F"); t.bgA3 = QColor("#1B1B1B");
    t.bgA4 = QColor("#191919"); t.bgA5 = QColor("#151515");
    t.bgB1 = QColor("#0E0E0E"); t.bgB2 = QColor("#090909");
    t.fg1 = QColor("#EAEAEA"); t.fg2 = QColor("#C6C6C6"); t.fgA2 = QColor("#8F8F8F");
    t.fgA3 = QColor("#6B6B6B"); t.fgA4 = QColor("#404040"); t.fgB1 = QColor("#161616");
    t.dividerBright = QColor("#373737"); t.dividerDim = QColor("#222222");
    t.accent = customAccent.isValid() ? customAccent : QColor("#D37918");
    t.accentBright = customAccent.isValid() ? customAccent.lighter(150) : QColor("#FC9729");
    // Dark/BG/Accent Bright (#371F06) ~= accent * 0.26
    t.accentBg = QColor::fromRgbF(t.accent.redF() * 0.26f, t.accent.greenF() * 0.26f, t.accent.blueF() * 0.26f);
    return t;
}

Tokens lightTokens(const QColor& customAccent)
{
    Tokens t;
    t.bgA1 = QColor("#EDEDED"); t.bgA2 = QColor("#DDDDDD"); t.bgA3 = QColor("#D5D5D5");
    t.bgA4 = QColor("#B3B3B3"); t.bgA5 = QColor("#E7E7E7");
    t.bgB1 = QColor("#F4F4F4"); t.bgB2 = QColor("#FFFFFF");
    t.fg1 = QColor("#161616"); t.fg2 = QColor("#2E2E2E"); t.fgA2 = QColor("#3C3C3C");
    t.fgA3 = QColor("#6B6B6B"); t.fgA4 = QColor("#ACACAC"); t.fgB1 = QColor("#161616");
    t.dividerBright = QColor("#9B9B9B"); t.dividerDim = QColor("#D5D5D5");
    t.accent = customAccent.isValid() ? customAccent : QColor("#E8851A");
    t.accentBright = customAccent.isValid() ? customAccent.lighter(150) : QColor("#FFA13B");
    t.accentBg = customAccent.isValid() ? customAccent.lighter(170) : QColor("#D8BA9A");
    return t;
}

QPalette makePalette(const Tokens& t)
{
    QPalette p;
    p.setColor(QPalette::Window, t.bgA5);
    p.setColor(QPalette::WindowText, t.fg2);
    p.setColor(QPalette::Base, t.bgB1);
    p.setColor(QPalette::AlternateBase, t.bgA4);
    p.setColor(QPalette::ToolTipBase, t.bgA1);
    p.setColor(QPalette::ToolTipText, t.fg1);
    p.setColor(QPalette::PlaceholderText, t.fgA3);
    p.setColor(QPalette::Text, t.fg2);
    p.setColor(QPalette::Button, t.bgA1);
    p.setColor(QPalette::ButtonText, t.fg2);
    p.setColor(QPalette::BrightText, t.fg1);
    p.setColor(QPalette::Light, t.dividerBright);
    p.setColor(QPalette::Midlight, t.dividerBright);
    p.setColor(QPalette::Mid, t.dividerDim);
    p.setColor(QPalette::Dark, t.bgB1);
    p.setColor(QPalette::Shadow, Qt::black);
    p.setColor(QPalette::Highlight, t.accent);
    p.setColor(QPalette::HighlightedText, t.fgB1);
    p.setColor(QPalette::Link, t.accentBright);
    p.setColor(QPalette::LinkVisited, t.accent);
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    p.setColor(QPalette::Accent, t.accent);
#endif

    for (const auto role : { QPalette::WindowText, QPalette::Text, QPalette::ButtonText })
        p.setColor(QPalette::Disabled, role, t.fgA4);
    p.setColor(QPalette::Disabled, QPalette::Button, t.bgA2);
    p.setColor(QPalette::Disabled, QPalette::Base, t.bgA4);
    return p;
}

QString makeStyleSheet(const Tokens& t)
{
    const auto c = [](const QColor& color) { return color.name(QColor::HexRgb); };

    // Figma references: menus & context menus (6px radius, 48px options in
    // context menus, compact in menu bars), Button (Cancel / Save), Dropdown,
    // Toggle, Settings dialog (sidebar categories, sections, fields).
    return QStringLiteral(R"QSS(
QMenu {
    background: %1;
    border: 1px solid %2;
    border-radius: 6px;
    padding: 4px;
    color: %3;
}
QMenu::item {
    padding: 7px 28px 7px 12px;
    border-radius: 4px;
    background: transparent;
}
QMenu::item:selected {
    background: %4;
    color: %5;
}
QMenu::item:disabled {
    color: %6;
}
QMenu::separator {
    height: 1px;
    background: %2;
    margin: 4px 8px;
}
QMenu::icon {
    padding-left: 8px;
}
QMenuBar {
    background: %1;
    color: %3;
}
QMenuBar::item:selected {
    background: %4;
    border-radius: 4px;
}
QToolTip {
    background: %4;
    color: %5;
    border: 1px solid %2;
    border-radius: 4px;
    padding: 4px 8px;
}
QPushButton, QToolButton {
    background: %4;
    color: %3;
    border: none;
    border-radius: 4px;
    padding: 6px 16px;
    min-height: 20px;
}
QToolButton {
    padding: 4px;
}
QPushButton:hover, QToolButton:hover {
    background: %2;
    color: %5;
}
QPushButton:pressed, QToolButton:pressed, QToolButton:checked {
    background: %7;
}
QPushButton:default {
    background: %8;
    color: %9;
}
QPushButton:default:hover {
    color: %10;
}
QPushButton:disabled, QToolButton:disabled {
    background: %11;
    color: %6;
}
QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox, QDoubleSpinBox, QTimeEdit, QComboBox, QKeySequenceEdit {
    background: %12;
    color: %3;
    border: 1px solid %2;
    border-radius: 4px;
    padding: 5px 8px;
    selection-background-color: %8;
    selection-color: %5;
}
QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QTimeEdit:focus, QComboBox:focus {
    border-color: %9;
}
QLineEdit:disabled, QSpinBox:disabled, QDoubleSpinBox:disabled, QComboBox:disabled {
    color: %6;
    background: %11;
}
QComboBox {
    background: %4;
    padding-right: 28px;
}
QComboBox::drop-down {
    border: none;
    width: 24px;
}
QComboBox QAbstractItemView {
    background: %1;
    color: %3;
    border: 1px solid %2;
    selection-background-color: %4;
    selection-color: %5;
    outline: none;
}
QSpinBox::up-button, QSpinBox::down-button, QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {
    border: none;
    width: 16px;
}
QCheckBox, QRadioButton {
    color: %3;
    spacing: 8px;
}
QCheckBox::indicator {
    width: 16px;
    height: 16px;
    border-radius: 4px;
    border: 1px solid %2;
    background: %12;
}
QCheckBox::indicator:checked {
    background: %9;
    border-color: %9;
}
QRadioButton::indicator {
    width: 14px;
    height: 14px;
    border-radius: 8px;
    border: 1px solid %2;
    background: %12;
}
QRadioButton::indicator:checked {
    background: %9;
    border: 4px solid %8;
}
QGroupBox {
    border: 1px solid %2;
    border-radius: 6px;
    margin-top: 18px;
    padding: 12px 8px 8px 8px;
    color: %5;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 8px;
    padding: 0 4px;
    color: %5;
}
QTabWidget::pane {
    border: 1px solid %2;
    border-radius: 6px;
    top: -1px;
}
QTabBar::tab {
    background: transparent;
    color: %13;
    padding: 8px 14px;
    border: none;
    border-bottom: 2px solid transparent;
}
QTabBar::tab:selected {
    color: %5;
    border-bottom: 2px solid %9;
}
QTabBar::tab:hover {
    color: %5;
}
QListView, QTreeView, QTableView, QListWidget, QTreeWidget, QTableWidget {
    background: %14;
    alternate-background-color: %15;
    color: %3;
    border: 1px solid %16;
    border-radius: 6px;
    outline: none;
    selection-background-color: %4;
    selection-color: %5;
}
QListView::item, QTreeView::item {
    padding: 4px;
    border-radius: 4px;
}
QListView::item:hover, QTreeView::item:hover {
    background: %11;
}
QListView::item:selected, QTreeView::item:selected {
    background: %4;
    color: %5;
}
QHeaderView::section {
    background: %1;
    color: %13;
    border: none;
    border-bottom: 1px solid %16;
    padding: 6px 8px;
}
QScrollBar:vertical {
    background: transparent;
    width: 10px;
    margin: 2px;
}
QScrollBar:horizontal {
    background: transparent;
    height: 10px;
    margin: 2px;
}
QScrollBar::handle:vertical, QScrollBar::handle:horizontal {
    background: %7;
    border-radius: 3px;
    min-height: 24px;
    min-width: 24px;
}
QScrollBar::handle:hover {
    background: %13;
}
QScrollBar::add-line, QScrollBar::sub-line, QScrollBar::add-page, QScrollBar::sub-page {
    background: none;
    border: none;
    width: 0px;
    height: 0px;
}
QSlider::groove:horizontal {
    height: 6px;
    background: %17;
    border-radius: 3px;
}
QSlider::sub-page:horizontal {
    background: %8;
    border-radius: 3px;
}
QSlider::handle:horizontal {
    background: %9;
    width: 8px;
    margin: -6px 0;
    border-radius: 3px;
}
QSlider::groove:vertical {
    width: 6px;
    background: %17;
    border-radius: 3px;
}
QSlider::add-page:vertical {
    background: %8;
    border-radius: 3px;
}
QSlider::handle:vertical {
    background: %9;
    height: 8px;
    margin: 0 -6px;
    border-radius: 3px;
}
QProgressBar {
    background: %17;
    border: none;
    border-radius: 4px;
    text-align: center;
    color: %5;
}
QProgressBar::chunk {
    background: %9;
    border-radius: 4px;
}
QSplitter::handle {
    background: %16;
}
QStatusBar {
    color: %13;
}
)QSS")
        .arg(c(t.bgA3), c(t.dividerBright), c(t.fg2), c(t.bgA1), c(t.fg1), c(t.fgA4),
             c(t.fgA4), c(t.accentBg), c(t.accent))
        .arg(c(t.accentBright), c(t.bgA2), c(t.bgB1), c(t.fgA2), c(t.bgA2), c(t.bgA4),
             c(t.dividerDim), c(t.bgB2));
}

bool s_systemDarkCaptured = false;
bool s_systemDark = false;

} // namespace

bool WidgetTheme::systemIsDark()
{
    if (!s_systemDarkCaptured && qApp)
    {
        const QPalette palette = QApplication::palette();
        s_systemDark = palette.color(QPalette::Window).lightness() < palette.color(QPalette::WindowText).lightness();
        s_systemDarkCaptured = true;
    }
    return s_systemDark;
}

void WidgetTheme::apply(bool dark, const QColor& customAccent)
{
    if (!qApp)
        return;

    // capture the platform palette before overriding it
    systemIsDark();

    // Fusion honours palettes and style sheets consistently on all platforms
    if (QStyle* fusion = QStyleFactory::create(QStringLiteral("Fusion")))
        QApplication::setStyle(fusion);

    const Tokens tokens = dark ? darkTokens(customAccent) : lightTokens(customAccent);
    QApplication::setPalette(makePalette(tokens));
    qApp->setStyleSheet(makeStyleSheet(tokens));
}
