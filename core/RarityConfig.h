#pragma once
#include <QString>
#include <QMap>

// ─── RarityConfig ─────────────────────────────────────────────────────────────
// Loads symbol rarity percentages from rarity.cfg (key = value pairs).
// Falls back to built-in defaults if the file is missing or a key is absent.
// Singleton — call RarityConfig::instance() anywhere.
class RarityConfig {
public:
    static RarityConfig& instance();

    // Returns the rarity % string for display (e.g. "14.3%")
    // key is the symbol name as it appears in rarity.cfg
    QString rarityStr(const QString& symbolName) const;

    // Returns the raw double value (for Reel strip weight calculation)
    double rarityValue(const QString& symbolName) const;

    // Reload from disk (call if file changes at runtime)
    void reload();

    // Path to the config file (resolved relative to executable)
    static QString configPath();

private:
    RarityConfig();
    void loadDefaults();
    void parse(const QString& path);

    QMap<QString, double> m_values;
};
