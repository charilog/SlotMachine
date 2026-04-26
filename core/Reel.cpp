#include "Reel.h"
#include "RarityConfig.h"
#include <cstdlib>
#include <algorithm>
#include <random>
#include <cmath>

Reel::Reel(QObject* parent, bool includeBonus)
    : QObject(parent), m_includeBonus(includeBonus) {
    m_strip = buildDefaultStrip(includeBonus);
}

void Reel::setStrip(const std::vector<Symbol>& strip) {
    m_strip = strip;
    m_currentIndex = 0;
}

void Reel::appendSymbol(const Symbol& symbol) {
    m_strip.push_back(symbol);
}

void Reel::spin() {
    if (m_strip.empty()) return;
    m_currentIndex = std::rand() % static_cast<int>(m_strip.size());
}

Symbol Reel::currentSymbol() const { return symbolAt(0); }

Symbol Reel::symbolAt(int rowOffset) const {
    if (m_strip.empty()) return Symbol(SymbolType::Cherry);
    int n   = static_cast<int>(m_strip.size());
    int idx = ((m_currentIndex + rowOffset) % n + n) % n;
    return m_strip[idx];
}

int Reel::currentIndex() const { return m_currentIndex; }
int Reel::stripSize()    const { return static_cast<int>(m_strip.size()); }

// ── Default strip ─────────────────────────────────────────────────────────────
// More symbols → lower probability of each → harder to win.
// Rare symbols appear fewer times.
// Strip built from rarity.cfg weights.
// Each symbol gets stops = round(weight / minWeight) clamped to [1..30].
std::vector<Symbol> Reel::buildDefaultStrip(bool includeBonus) {
    using S = SymbolType;
    auto& cfg = RarityConfig::instance();

    // Full symbol list (includes Bonus)
    static const struct { S sym; const char* key; } ALL[] = {
        { S::Cherry,     "Cherry"      },
        { S::Lemon,      "Lemon"       },
        { S::Orange,     "Orange"      },
        { S::Watermelon, "Watermelon"  },
        { S::Grape,      "Grape"       },
        { S::Strawberry, "Strawberry"  },
        { S::Bell,       "Bell"        },
        { S::Bar,        "Bar"         },
        { S::Bar2,       "Bar2"        },
        { S::Bar3,       "Bar3"        },
        { S::Seven,      "Seven"       },
        { S::Wild,       "Wild"        },
        { S::Bonus,      "Bonus"       },   // scatter — Easy only
    };
    static constexpr int ALL_SIZE = (int)(sizeof(ALL)/sizeof(ALL[0]));

    // Find minimum non-zero weight (excluding Bonus from base calculation)
    double minW = 1e9;
    for (int i = 0; i < ALL_SIZE; ++i) {
        if (ALL[i].sym == S::Bonus) continue;
        double v = cfg.rarityValue(QString::fromLatin1(ALL[i].key));
        if (v > 0.0 && v < minW) minW = v;
    }
    if (minW <= 0.0) minW = 1.0;

    std::vector<Symbol> strip;
    strip.reserve(80);

    for (int i = 0; i < ALL_SIZE; ++i) {
        // Skip Bonus for Advanced machine (5 reels)
        if (ALL[i].sym == S::Bonus && !includeBonus) continue;

        double v = cfg.rarityValue(QString::fromLatin1(ALL[i].key));
        if (v <= 0.0) continue;

        int stops = static_cast<int>(std::round(v / minW));
        stops = std::max(1, std::min(stops, 30));
        for (int j = 0; j < stops; ++j)
            strip.push_back(Symbol(ALL[i].sym));
    }

    static std::mt19937 rng(std::random_device{}());
    std::shuffle(strip.begin(), strip.end(), rng);
    return strip;
}
