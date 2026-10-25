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
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFontDatabase>
#include <QPainter>
#include <QPixmap>

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


// Figma: Toggle (68x32). The "ON" / "OFF" labels are kept as vector paths.
const char* const TOGGLE_ON_LABEL = "M18.42 16.63C18.42 17.55 18.25 18.35 17.92 19.02C17.58 19.69 17.13 20.21 16.55 20.57C15.97 20.93 15.31 21.11 14.56 21.11C13.82 21.11 13.16 20.93 12.58 20.57C12.00 20.21 11.54 19.69 11.21 19.02C10.88 18.35 10.71 17.55 10.71 16.63C10.71 15.71 10.88 14.92 11.21 14.25C11.54 13.57 12.00 13.06 12.58 12.69C13.16 12.33 13.82 12.15 14.56 12.15C15.31 12.15 15.97 12.33 16.55 12.69C17.13 13.06 17.58 13.57 17.92 14.25C18.25 14.92 18.42 15.71 18.42 16.63ZM17.39 16.63C17.39 15.88 17.27 15.24 17.01 14.72C16.76 14.20 16.42 13.80 16 13.54C15.57 13.27 15.09 13.14 14.56 13.14C14.03 13.14 13.56 13.27 13.13 13.54C12.70 13.80 12.36 14.20 12.11 14.72C11.86 15.24 11.73 15.88 11.73 16.63C11.73 17.39 11.86 18.02 12.11 18.54C12.36 19.06 12.70 19.46 13.13 19.73C13.56 19.99 14.03 20.13 14.56 20.13C15.09 20.13 15.57 19.99 16 19.73C16.42 19.46 16.76 19.06 17.01 18.54C17.27 18.02 17.39 17.39 17.39 16.63ZM27.11 12.27V21H26.09L21.33 14.14H21.25V21H20.19V12.27H21.22L25.99 19.14H26.07V12.27H27.11Z";
const char* const TOGGLE_OFF_LABEL = "M44.42 16.63C44.42 17.55 44.25 18.35 43.92 19.02C43.58 19.69 43.13 20.21 42.55 20.57C41.97 20.93 41.31 21.11 40.56 21.11C39.82 21.11 39.16 20.93 38.58 20.57C38.00 20.21 37.54 19.69 37.21 19.02C36.88 18.35 36.71 17.55 36.71 16.63C36.71 15.71 36.88 14.92 37.21 14.25C37.54 13.57 38.00 13.06 38.58 12.69C39.16 12.33 39.82 12.15 40.56 12.15C41.31 12.15 41.97 12.33 42.55 12.69C43.13 13.06 43.58 13.57 43.92 14.25C44.25 14.92 44.42 15.71 44.42 16.63ZM43.39 16.63C43.39 15.88 43.27 15.24 43.01 14.72C42.76 14.20 42.42 13.80 42 13.54C41.57 13.27 41.09 13.14 40.56 13.14C40.03 13.14 39.56 13.27 39.13 13.54C38.70 13.80 38.36 14.20 38.11 14.72C37.86 15.24 37.73 15.88 37.73 16.63C37.73 17.39 37.86 18.02 38.11 18.54C38.36 19.06 38.70 19.46 39.13 19.73C39.56 19.99 40.03 20.13 40.56 20.13C41.09 20.13 41.57 19.99 42 19.73C42.42 19.46 42.76 19.06 43.01 18.54C43.27 18.02 43.39 17.39 43.39 16.63ZM46.19 21V12.27H51.43V13.21H47.25V16.15H51.03V17.09H47.25V21H46.19ZM53.24 21V12.27H58.47V13.21H54.29V16.15H58.08V17.09H54.29V21H53.24Z";

// writes the toggle images for the given tokens, returns their paths
std::pair<QString, QString> writeToggleImages(const Tokens& t)
{
    const QString dirPath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                            + QStringLiteral("/redesign");
    QDir().mkpath(dirPath);

    const auto c = [](const QColor& color) { return color.name(QColor::HexRgb); };
    const QColor knobOn = QColor::fromRgbF(t.accent.redF() * 0.81f, t.accent.greenF() * 0.81f, t.accent.blueF() * 0.81f);

    const QString on = QStringLiteral(
        "<svg width=\"68\" height=\"32\" viewBox=\"0 0 68 32\" xmlns=\"http://www.w3.org/2000/svg\">"
        "<rect width=\"68\" height=\"32\" rx=\"6\" fill=\"%1\"/>"
        "<rect x=\"3\" y=\"3\" width=\"33\" height=\"26\" rx=\"3\" fill=\"%2\"/>"
        "<rect x=\"39\" y=\"3\" width=\"26\" height=\"26\" rx=\"3\" fill=\"%3\"/>"
        "<path d=\"%4\" fill=\"%5\"/></svg>")
        .arg(c(t.bgB2), c(t.accentBg), c(knobOn), QLatin1String(TOGGLE_ON_LABEL), c(t.accent));
    const QString off = QStringLiteral(
        "<svg width=\"68\" height=\"32\" viewBox=\"0 0 68 32\" xmlns=\"http://www.w3.org/2000/svg\">"
        "<rect width=\"68\" height=\"32\" rx=\"6\" fill=\"%1\"/>"
        "<rect x=\"32\" y=\"3\" width=\"33\" height=\"26\" rx=\"3\" fill=\"%2\"/>"
        "<rect x=\"3\" y=\"3\" width=\"26\" height=\"26\" rx=\"3\" fill=\"%3\"/>"
        "<path d=\"%4\" fill=\"%5\"/></svg>")
        .arg(c(t.bgB2), c(t.bgA4), c(t.fgA4), QLatin1String(TOGGLE_OFF_LABEL), c(t.fgA3));

    const QString onPath = dirPath + QStringLiteral("/toggle-on-%1.svg").arg(c(t.accent).mid(1));
    const QString offPath = dirPath + QStringLiteral("/toggle-off-%1.svg").arg(c(t.bgB2).mid(1));
    for (const auto& [path, data] : { std::pair{onPath, on}, std::pair{offPath, off} })
    {
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
            file.write(data.toUtf8());
    }
    return { onPath, offPath };
}

// rules overriding the base style sheet for the redesign buttons and toggles
QString makeControlsStyleSheet(const Tokens& t)
{
    const auto c = [](const QColor& color) { return color.name(QColor::HexRgb); };
    const auto [onPath, offPath] = writeToggleImages(t);

    return QStringLiteral(R"QSS(
QPushButton {
    background: %1;
    color: %2;
    border-radius: 6px;
    font-weight: 600;
    padding: 6px 16px;
    min-width: 68px;
}
QPushButton:hover {
    background: %3;
    color: %4;
}
QPushButton:default {
    background: %5;
    color: %6;
}
QGroupBox {
    border: none;
    border-top: 1px solid %9;
    border-radius: 0px;
    margin-top: 28px;
    padding: 14px 0px 8px 0px;
    font-size: 16px;
    font-weight: 500;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 0px;
    padding: 0px;
    color: %10;
}
QCheckBox::indicator {
    width: 68px;
    height: 32px;
    border: none;
    background: transparent;
    image: url(%7);
}
QCheckBox::indicator:checked {
    background: transparent;
    image: url(%8);
}
)QSS")
        .arg(c(t.bgA3), c(t.fgA2), c(t.bgA1), c(t.fg1), c(t.accentBg), c(t.accent), offPath, onPath, c(t.dividerDim))
        .arg(c(t.fg2));
}

Tokens s_tokens = darkTokens(QColor());

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
    s_tokens = tokens;
    QApplication::setPalette(makePalette(tokens));
    qApp->setStyleSheet(makeStyleSheet(tokens) + makeControlsStyleSheet(tokens));
}

QColor WidgetTheme::accent() { return s_tokens.accent; }
QColor WidgetTheme::textPrimary() { return s_tokens.fg1; }
QColor WidgetTheme::textSecondary() { return s_tokens.fgA2; }
QColor WidgetTheme::textTertiary() { return s_tokens.fgA3; }
QColor WidgetTheme::selection() { return s_tokens.bgA1; }
QColor WidgetTheme::panel() { return s_tokens.bgA4; }

QIcon WidgetTheme::glyphIcon(QChar glyph, int size, const QColor& normal, const QColor& checked)
{
    static bool fontLoaded = false;
    if (!fontLoaded)
    {
        if (!QFontDatabase::families().contains(QStringLiteral("VLCIcons")))
            QFontDatabase::addApplicationFont(QStringLiteral(":/VLCIcons.ttf"));
        fontLoaded = true;
    }

    const qreal dpr = qApp ? qApp->devicePixelRatio() : 1.0;
    const auto render = [&](const QColor& color) {
        QPixmap pixmap(QSize(size, size) * dpr);
        pixmap.setDevicePixelRatio(dpr);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        QFont font(QStringLiteral("VLCIcons"));
        font.setPixelSize(size);
        painter.setFont(font);
        painter.setPen(color);
        painter.drawText(QRect(0, 0, size, size), Qt::AlignCenter, QString(glyph));
        return pixmap;
    };

    QIcon icon;
    icon.addPixmap(render(normal), QIcon::Normal, QIcon::Off);
    icon.addPixmap(render(checked), QIcon::Normal, QIcon::On);
    icon.addPixmap(render(checked), QIcon::Active, QIcon::On);
    return icon;
}
