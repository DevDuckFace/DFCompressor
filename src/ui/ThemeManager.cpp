/**
 * @file ThemeManager.cpp
 * @brief Theme manager implementation
 */

#include "ThemeManager.h"
#include <QApplication>
#include <QFile>
#include <QPalette>
#include <QStyleFactory>

ThemeManager& ThemeManager::instance()
{
    static ThemeManager instance;
    return instance;
}

void ThemeManager::initialize()
{
    // Pre-generate themes
    m_themeCache["dark"] = generateDarkTheme();
    m_themeCache["light"] = generateLightTheme();
}

void ThemeManager::applyTheme(const QString& themeName)
{
    QString styleSheet;
    
    if (m_themeCache.contains(themeName)) {
        styleSheet = m_themeCache[themeName];
    } else {
        styleSheet = loadStyleSheet(themeName);
    }
    
    qApp->setStyleSheet(styleSheet);
    m_currentTheme = themeName;
}

QString ThemeManager::currentTheme() const
{
    return m_currentTheme;
}

QStringList ThemeManager::availableThemes() const
{
    return {"dark", "light", "system"};
}

QString ThemeManager::loadStyleSheet(const QString& themeName)
{
    QString resourcePath = QString(":/styles/%1.qss").arg(themeName);
    QFile file(resourcePath);
    
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return file.readAll();
    }
    
    // Fallback to dark theme
    return generateDarkTheme();
}

QString ThemeManager::generateDarkTheme()
{
    return R"(
/* MediaForge Dark Theme */
* {
    font-family: "Inter", "Segoe UI", sans-serif;
}

QMainWindow {
    background-color: #121212;
}

QWidget {
    background-color: #1e1e1e;
    color: #ffffff;
}

QMenuBar {
    background-color: #1e1e1e;
    border-bottom: 1px solid #3d3d3d;
    padding: 4px;
}

QMenuBar::item {
    padding: 6px 12px;
    border-radius: 4px;
}

QMenuBar::item:selected {
    background-color: #3d3d3d;
}

QMenu {
    background-color: #2d2d2d;
    border: 1px solid #3d3d3d;
    border-radius: 8px;
    padding: 4px;
}

QMenu::item {
    padding: 8px 24px;
    border-radius: 4px;
}

QMenu::item:selected {
    background-color: #0078d4;
}

QToolBar {
    background-color: #1e1e1e;
    border: none;
    spacing: 8px;
    padding: 8px;
}

QToolButton {
    background-color: transparent;
    border: none;
    border-radius: 8px;
    padding: 8px 12px;
    color: #ffffff;
}

QToolButton:hover {
    background-color: #3d3d3d;
}

QToolButton:pressed {
    background-color: #4d4d4d;
}

QStatusBar {
    background-color: #1e1e1e;
    border-top: 1px solid #3d3d3d;
}

QPushButton {
    background-color: #3d3d3d;
    border: none;
    border-radius: 6px;
    padding: 8px 16px;
    color: #ffffff;
    font-weight: 500;
}

QPushButton:hover {
    background-color: #4d4d4d;
}

QPushButton:pressed {
    background-color: #5d5d5d;
}

QPushButton:disabled {
    background-color: #2d2d2d;
    color: #666666;
}

QLineEdit, QSpinBox, QComboBox {
    background-color: #2d2d2d;
    border: 1px solid #3d3d3d;
    border-radius: 6px;
    padding: 8px;
    color: #ffffff;
}

QLineEdit:focus, QSpinBox:focus, QComboBox:focus {
    border-color: #0078d4;
}

QComboBox::drop-down {
    border: none;
    width: 24px;
}

QComboBox::down-arrow {
    image: url(:/icons/arrow_down.svg);
    width: 12px;
    height: 12px;
}

QComboBox QAbstractItemView {
    background-color: #2d2d2d;
    border: 1px solid #3d3d3d;
    border-radius: 8px;
    selection-background-color: #0078d4;
}

QCheckBox {
    spacing: 8px;
}

QCheckBox::indicator {
    width: 18px;
    height: 18px;
    border-radius: 4px;
    border: 2px solid #3d3d3d;
    background-color: transparent;
}

QCheckBox::indicator:checked {
    background-color: #0078d4;
    border-color: #0078d4;
}

QTabWidget::pane {
    border: 1px solid #3d3d3d;
    border-radius: 8px;
    background-color: #1e1e1e;
}

QTabBar::tab {
    background-color: #2d2d2d;
    border: none;
    padding: 10px 20px;
    margin-right: 2px;
    border-top-left-radius: 8px;
    border-top-right-radius: 8px;
}

QTabBar::tab:selected {
    background-color: #1e1e1e;
}

QTabBar::tab:hover:!selected {
    background-color: #3d3d3d;
}

QGroupBox {
    border: 1px solid #3d3d3d;
    border-radius: 8px;
    margin-top: 16px;
    padding-top: 16px;
    font-weight: 600;
}

QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    left: 12px;
    padding: 0 8px;
    color: #ffffff;
}

QScrollBar:vertical {
    background-color: #1e1e1e;
    width: 10px;
    border-radius: 5px;
}

QScrollBar::handle:vertical {
    background-color: #3d3d3d;
    border-radius: 5px;
    min-height: 30px;
}

QScrollBar::handle:vertical:hover {
    background-color: #4d4d4d;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0;
}

QProgressBar {
    border: none;
    border-radius: 4px;
    background-color: #2d2d2d;
    text-align: center;
}

QProgressBar::chunk {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #667eea, stop:1 #764ba2);
    border-radius: 4px;
}

QSplitter::handle {
    background-color: #3d3d3d;
}

QSplitter::handle:horizontal {
    width: 2px;
}

QSplitter::handle:vertical {
    height: 2px;
}
)";
}

QString ThemeManager::generateLightTheme()
{
    return R"(
/* MediaForge Light Theme */
* {
    font-family: "Inter", "Segoe UI", sans-serif;
}

QMainWindow {
    background-color: #f5f5f5;
}

QWidget {
    background-color: #ffffff;
    color: #1a1a1a;
}

QMenuBar {
    background-color: #ffffff;
    border-bottom: 1px solid #e0e0e0;
    padding: 4px;
}

QMenuBar::item {
    padding: 6px 12px;
    border-radius: 4px;
}

QMenuBar::item:selected {
    background-color: #e0e0e0;
}

QMenu {
    background-color: #ffffff;
    border: 1px solid #e0e0e0;
    border-radius: 8px;
    padding: 4px;
}

QMenu::item {
    padding: 8px 24px;
    border-radius: 4px;
}

QMenu::item:selected {
    background-color: #0078d4;
    color: #ffffff;
}

QToolBar {
    background-color: #ffffff;
    border: none;
    spacing: 8px;
    padding: 8px;
}

QToolButton {
    background-color: transparent;
    border: none;
    border-radius: 8px;
    padding: 8px 12px;
    color: #1a1a1a;
}

QToolButton:hover {
    background-color: #e0e0e0;
}

QPushButton {
    background-color: #e0e0e0;
    border: none;
    border-radius: 6px;
    padding: 8px 16px;
    color: #1a1a1a;
    font-weight: 500;
}

QPushButton:hover {
    background-color: #d0d0d0;
}

QLineEdit, QSpinBox, QComboBox {
    background-color: #ffffff;
    border: 1px solid #e0e0e0;
    border-radius: 6px;
    padding: 8px;
    color: #1a1a1a;
}

QLineEdit:focus, QSpinBox:focus, QComboBox:focus {
    border-color: #0078d4;
}

QStatusBar {
    background-color: #ffffff;
    border-top: 1px solid #e0e0e0;
}

QTabWidget::pane {
    border: 1px solid #e0e0e0;
    border-radius: 8px;
    background-color: #ffffff;
}

QTabBar::tab {
    background-color: #f5f5f5;
    border: none;
    padding: 10px 20px;
    margin-right: 2px;
}

QTabBar::tab:selected {
    background-color: #ffffff;
}

QGroupBox {
    border: 1px solid #e0e0e0;
    border-radius: 8px;
    margin-top: 16px;
    padding-top: 16px;
}

QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    left: 12px;
    padding: 0 8px;
}

QScrollBar:vertical {
    background-color: #f5f5f5;
    width: 10px;
}

QScrollBar::handle:vertical {
    background-color: #c0c0c0;
    border-radius: 5px;
}

QProgressBar {
    border: none;
    border-radius: 4px;
    background-color: #e0e0e0;
}

QProgressBar::chunk {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
        stop:0 #667eea, stop:1 #764ba2);
    border-radius: 4px;
}
)";
}
