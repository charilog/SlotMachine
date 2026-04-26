#include "PayTable.h"
#include <QStringList>
#include <algorithm>

PayTable::PayTable() { initTables(); }

void PayTable::initTables() {
    // ── 3-reel table (Easy) ───────────────────────────────────────────────────
    m_table3 = {
        { SymbolType::Wild,        3, 1000, "★★★ JACKPOT! 3× WILD ★★★"      },
        { SymbolType::Seven,       3,  200, "7  7  7 — Lucky Sevens!"          },
        { SymbolType::Bar3,        3,  100, "3BAR 3BAR 3BAR"                   },
        { SymbolType::Bar2,        3,   60, "2BAR 2BAR 2BAR"                   },
        { SymbolType::Bar,         3,   30, "BAR BAR BAR"                       },
        { SymbolType::Bell,        3,   20, "3× Bell 🔔"                        },
        { SymbolType::Strawberry,  3,   12, "3× Strawberry 🍓"                  },
        { SymbolType::Grape,       3,   10, "3× Grape 🍇"                       },
        { SymbolType::Watermelon,  3,    8, "3× Watermelon 🍉"                  },
        { SymbolType::Orange,      3,    6, "3× Orange 🍊"                      },
        { SymbolType::Lemon,       3,    4, "3× Lemon 🍋"                       },
        { SymbolType::Cherry,      3,    3, "3× Cherry 🍒"                      },
        { SymbolType::Cherry,      2,    2, "any  🍒  🍒"                       },
    };

    // ── 5-reel table (Advanced) ───────────────────────────────────────────────
    m_table5 = {
        { SymbolType::Wild,        5, 5000, "★★★★★ MEGA JACKPOT! ★★★★★"     },
        { SymbolType::Seven,       5, 1000, "5× 7 — Spectacular!"              },
        { SymbolType::Seven,       4,  300, "4× 7"                             },
        { SymbolType::Seven,       3,  100, "3× 7"                             },
        { SymbolType::Bar3,        5,  500, "5× 3BAR"                          },
        { SymbolType::Bar3,        4,  200, "4× 3BAR"                          },
        { SymbolType::Bar3,        3,   80, "3× 3BAR"                          },
        { SymbolType::Bar2,        5,  300, "5× 2BAR"                          },
        { SymbolType::Bar2,        4,  120, "4× 2BAR"                          },
        { SymbolType::Bar2,        3,   50, "3× 2BAR"                          },
        { SymbolType::Bar,         5,  150, "5× BAR"                           },
        { SymbolType::Bar,         4,   60, "4× BAR"                           },
        { SymbolType::Bar,         3,   25, "3× BAR"                           },
        { SymbolType::Bell,        5,  100, "5× Bell 🔔"                        },
        { SymbolType::Bell,        4,   40, "4× Bell 🔔"                        },
        { SymbolType::Bell,        3,   15, "3× Bell 🔔"                        },
        { SymbolType::Strawberry,  5,   70, "5× Strawberry 🍓"                  },
        { SymbolType::Strawberry,  4,   28, "4× Strawberry 🍓"                  },
        { SymbolType::Strawberry,  3,   10, "3× Strawberry 🍓"                  },
        { SymbolType::Grape,       5,   60, "5× Grape 🍇"                       },
        { SymbolType::Grape,       4,   24, "4× Grape 🍇"                       },
        { SymbolType::Grape,       3,    8, "3× Grape 🍇"                       },
        { SymbolType::Watermelon,  5,   50, "5× Watermelon 🍉"                  },
        { SymbolType::Watermelon,  4,   20, "4× Watermelon 🍉"                  },
        { SymbolType::Watermelon,  3,    7, "3× Watermelon 🍉"                  },
        { SymbolType::Orange,      5,   40, "5× Orange 🍊"                      },
        { SymbolType::Orange,      4,   16, "4× Orange 🍊"                      },
        { SymbolType::Orange,      3,    6, "3× Orange 🍊"                      },
        { SymbolType::Lemon,       5,   30, "5× Lemon 🍋"                       },
        { SymbolType::Lemon,       4,   12, "4× Lemon 🍋"                       },
        { SymbolType::Lemon,       3,    4, "3× Lemon 🍋"                       },
        { SymbolType::Cherry,      5,   20, "5× Cherry 🍒"                      },
        { SymbolType::Cherry,      4,    8, "4× Cherry 🍒"                      },
        { SymbolType::Cherry,      3,    3, "3× Cherry 🍒"                      },
        { SymbolType::Cherry,      2,    2, "any  🍒  🍒"                       },
    };
}

WinResult PayTable::evaluate3Reels(const std::vector<Symbol>& symbols) const {
    if (symbols.size() < 3) return {};
    return evalPayline(symbols, m_table3);
}

WinResult PayTable::evaluate5Reels(const std::vector<std::vector<Symbol>>& grid) const {
    if (grid.size() < 5 || grid[0].size() < 3) return {};
    static const char* rowNames[] = { "Top", "Centre", "Bottom" };
    WinResult combined;
    QStringList descParts;
    for (int row = 0; row < 3; ++row) {
        std::vector<Symbol> line;
        for (const auto& col : grid) line.push_back(col[row]);
        WinResult r = evalPayline(line, m_table5);
        if (!r.isWin) continue;
        PaylineWin pw;
        pw.row         = row;
        pw.winCount    = r.winCount;
        pw.multiplier  = r.multiplier;
        pw.description = r.description;
        combined.paylineWins.push_back(pw);
        combined.isWin      = true;
        combined.multiplier += r.multiplier;
        descParts << QStringLiteral("%1 [%2]").arg(r.description)
                                               .arg(QString::fromLatin1(rowNames[row]));
    }
    if (combined.isWin) combined.description = descParts.join(QStringLiteral("  +  "));
    return combined;
}

WinResult PayTable::evalPayline(const std::vector<Symbol>& line,
                                 const std::vector<Entry>& table) const {
    for (const auto& entry : table) {
        int n = countFromLeft(line, entry.symbol);
        if (n >= entry.count) {
            WinResult r;
            r.isWin       = true;
            r.multiplier  = entry.multiplier;
            r.description = entry.description;
            r.winCount    = n;
            r.paylineRow  = 1;
            return r;
        }
    }
    return {};
}

int PayTable::countFromLeft(const std::vector<Symbol>& line, SymbolType target) const {
    // Count consecutive matching symbols from the RIGHT
    int count = 0;
    for (int i = static_cast<int>(line.size()) - 1; i >= 0; --i) {
        if (line[i].type() == SymbolType::Wild || line[i].type() == target) ++count;
        else break;
    }
    return count;
}
