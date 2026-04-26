#include "RarityConfig.h"
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QFileInfo>

// ── Singleton ─────────────────────────────────────────────────────────────────
RarityConfig& RarityConfig::instance() {
    static RarityConfig inst;
    return inst;
}

RarityConfig::RarityConfig() {
    loadDefaults();
    reload();
}

// ── Built-in defaults (used if file is missing or key absent) ─────────────────
void RarityConfig::loadDefaults() {
    m_values = {
        { QStringLiteral("Cherry"),     14.3 },
        { QStringLiteral("Lemon"),      12.9 },
        { QStringLiteral("Orange"),     11.4 },
        { QStringLiteral("Watermelon"), 10.0 },
        { QStringLiteral("Grape"),       8.6 },
        { QStringLiteral("Strawberry"),  7.1 },
        { QStringLiteral("Bell"),       10.0 },
        { QStringLiteral("Bar"),         7.1 },
        { QStringLiteral("Bar2"),        4.3 },
        { QStringLiteral("Bar3"),        2.9 },
        { QStringLiteral("Seven"),       4.3 },
        { QStringLiteral("Wild"),        4.3 },
    };
}

// ── File path ─────────────────────────────────────────────────────────────────
QString RarityConfig::configPath() {
    // Look next to the executable first, then current working directory
    QString exeDir = QCoreApplication::applicationDirPath();
    QString candidate = exeDir + QStringLiteral("/rarity.cfg");
    if (QFileInfo::exists(candidate)) return candidate;
    return QStringLiteral("rarity.cfg");
}

// ── Parse ─────────────────────────────────────────────────────────────────────
void RarityConfig::parse(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith(QLatin1Char('#'))) continue;

        int eq = line.indexOf(QLatin1Char('='));
        if (eq < 0) continue;

        QString key = line.left(eq).trimmed();
        bool ok = false;
        double val = line.mid(eq + 1).trimmed().toDouble(&ok);
        if (ok && val >= 0.0)
            m_values.insert(key, val);
    }
}

void RarityConfig::reload() {
    loadDefaults();          // reset to defaults first
    parse(configPath());     // overlay with file values
}

// ── Accessors ─────────────────────────────────────────────────────────────────
double RarityConfig::rarityValue(const QString& symbolName) const {
    return m_values.value(symbolName, 0.0);
}

QString RarityConfig::rarityStr(const QString& symbolName) const {
    double v = rarityValue(symbolName);
    if (v <= 0.0) return QStringLiteral("?");
    return QStringLiteral("%1%").arg(v, 0, 'f', 1);
}
