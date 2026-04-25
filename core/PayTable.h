#pragma once
#include "Symbol.h"
#include <vector>
#include <QString>

// ─── PaylineWin ───────────────────────────────────────────────────────────────
// Describes a single winning payline (one row in the 5-reel grid).
struct PaylineWin {
    int     row;            // 0=top  1=centre  2=bottom
    int     winCount;       // consecutive matching reels from left
    int     multiplier;
    QString description;
};

// ─── WinResult ────────────────────────────────────────────────────────────────
struct WinResult {
    bool    isWin       { false };
    int     multiplier  { 0 };      // total across all winning paylines
    QString description;            // combined summary string

    // Easy (3-reel, 1 payline) — single-line fields
    int     winCount    { 0 };
    int     paylineRow  { 1 };

    // Advanced (5-reel, 3 paylines) — one entry per winning row
    std::vector<PaylineWin> paylineWins;
};

// ─── PayTable ─────────────────────────────────────────────────────────────────
class PayTable {
public:
    PayTable();

    // 3-reel / 1-payline (Easy)
    WinResult evaluate3Reels(const std::vector<Symbol>& symbols) const;

    // 5-reel / 3-paylines (Advanced) — returns ALL winning paylines summed
    WinResult evaluate5Reels(const std::vector<std::vector<Symbol>>& grid) const;

private:
    struct Entry {
        SymbolType  symbol;
        int         count;
        int         multiplier;
        QString     description;
    };

    std::vector<Entry> m_table3;
    std::vector<Entry> m_table5;

    void      initTables();
    // Evaluates a single payline; returns isWin=false if no match
    WinResult evalPayline(const std::vector<Symbol>& line,
                          const std::vector<Entry>& table) const;
    int       countFromLeft(const std::vector<Symbol>& line,
                            SymbolType target) const;
};
