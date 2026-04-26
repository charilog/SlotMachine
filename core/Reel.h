#pragma once
#include "Symbol.h"
#include <QObject>
#include <vector>

// ─── Reel ────────────────────────────────────────────────────────────────────
// Represents one physical reel strip.
// Symbols are stored in a circular strip; spin() randomises the stop position.
// symbolAt(offset) lets the UI read adjacent rows (e.g. -1 / 0 / +1).
class Reel : public QObject {
    Q_OBJECT
public:
    explicit Reel(QObject* parent = nullptr, bool includeBonus = true);

    // Build the strip (call before first spin, or use default strip)
    void        setStrip(const std::vector<Symbol>& strip);
    void        appendSymbol(const Symbol& symbol);

    // Core mechanics
    void        spin();                         // randomise stop position
    Symbol      currentSymbol() const;          // centre / payline row
    Symbol      symbolAt(int rowOffset) const;  // -1 = top, 0 = centre, +1 = bottom

    int         currentIndex()  const;
    int         stripSize()     const;

private:
    std::vector<Symbol> m_strip;
    int                 m_currentIndex { 0 };

    static std::vector<Symbol> buildDefaultStrip(bool includeBonus);
    bool m_includeBonus { true };
};
