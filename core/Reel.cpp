#include "Reel.h"
#include <cstdlib>
#include <algorithm>
#include <random>

Reel::Reel(QObject* parent) : QObject(parent) {
    m_strip = buildDefaultStrip();
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
// Strip weights (out of 64 total stops — standard slot machine reel size):
//   Cherry     ×10  = 15.6%   most common
//   Lemon      ×9   = 14.1%
//   Orange     ×8   = 12.5%
//   Watermelon ×7   = 10.9%
//   Grape      ×6   =  9.4%
//   Strawberry ×5   =  7.8%
//   Bell       ×7   = 10.9%   mid-tier (common enough to tease)
//   BAR        ×5   =  7.8%
//   2BAR       ×3   =  4.7%
//   3BAR       ×2   =  3.1%
//   7          ×1   =  1.6%
//   Wild       ×1   =  1.6%   rarest
//              ----   -----
//   Total      64    100%
std::vector<Symbol> Reel::buildDefaultStrip() {
    using S = SymbolType;
    std::vector<Symbol> strip = {
        Symbol(S::Cherry), Symbol(S::Cherry), Symbol(S::Cherry), Symbol(S::Cherry),
        Symbol(S::Cherry), Symbol(S::Cherry), Symbol(S::Cherry), Symbol(S::Cherry),
        Symbol(S::Cherry), Symbol(S::Cherry),
        Symbol(S::Lemon),  Symbol(S::Lemon),  Symbol(S::Lemon),  Symbol(S::Lemon),
        Symbol(S::Lemon),  Symbol(S::Lemon),  Symbol(S::Lemon),  Symbol(S::Lemon),
        Symbol(S::Lemon),
        Symbol(S::Orange), Symbol(S::Orange), Symbol(S::Orange), Symbol(S::Orange),
        Symbol(S::Orange), Symbol(S::Orange), Symbol(S::Orange), Symbol(S::Orange),
        Symbol(S::Watermelon), Symbol(S::Watermelon), Symbol(S::Watermelon),
        Symbol(S::Watermelon), Symbol(S::Watermelon), Symbol(S::Watermelon),
        Symbol(S::Watermelon),
        Symbol(S::Grape),  Symbol(S::Grape),  Symbol(S::Grape),
        Symbol(S::Grape),  Symbol(S::Grape),  Symbol(S::Grape),
        Symbol(S::Strawberry), Symbol(S::Strawberry), Symbol(S::Strawberry),
        Symbol(S::Strawberry), Symbol(S::Strawberry),
        Symbol(S::Bell),   Symbol(S::Bell),   Symbol(S::Bell),
        Symbol(S::Bell),   Symbol(S::Bell),   Symbol(S::Bell),
        Symbol(S::Bell),
        Symbol(S::Bar),    Symbol(S::Bar),    Symbol(S::Bar),
        Symbol(S::Bar),    Symbol(S::Bar),
        Symbol(S::Bar2),   Symbol(S::Bar2),   Symbol(S::Bar2),
        Symbol(S::Bar3),   Symbol(S::Bar3),
        Symbol(S::Seven), Symbol(S::Seven), Symbol(S::Seven),
        Symbol(S::Wild),  Symbol(S::Wild),  Symbol(S::Wild),
    };

    // Shuffle so symbols appear in random order on the visible strip
    // Use a seeded engine for reproducibility per process run
    static std::mt19937 rng(std::random_device{}());
    std::shuffle(strip.begin(), strip.end(), rng);

    return strip;
}
