/**
 * @file ThemeManager.h
 * @brief Theme manager header
 */

#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QString>
#include <QMap>

class ThemeManager
{
public:
    static ThemeManager& instance();

    void initialize();
    void applyTheme(const QString& themeName);
    QString currentTheme() const;
    QStringList availableThemes() const;

private:
    ThemeManager() = default;
    ~ThemeManager() = default;
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    QString loadStyleSheet(const QString& themeName);
    QString generateDarkTheme();
    QString generateLightTheme();

private:
    QString m_currentTheme;
    QMap<QString, QString> m_themeCache;
};

#endif // THEMEMANAGER_H
