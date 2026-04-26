#pragma once
#include <QWidget>
#include <QTimer>
#include "../core/Symbol.h"
#include <deque>
#include <vector>
#include <set>

// ─── ReelWidget ───────────────────────────────────────────────────────────────
// Mechanical reel animation with pre-planned stop sequence:
//
//  Spinning:  continuous top→bottom scroll at SPIN_SPEED
//
//  Stopping:  stopSpin() pre-builds a symbol queue:
//               [rand×N, final[2], final[1], final[0], dummy]
//             Each row-crossing pops the next symbol from the queue.
//             Speed decreases per advance (DECEL_PER_ADV).
//             When queue exhausted → strip = [dummy, f0, f1, f2] at offset=0
//             → emit stopped()   (no visible snap, correct symbols guaranteed)
//
//  Strip layout (deque, always 4):
//    [0] above-visible (entering from top)
//    [1] top    row
//    [2] centre row   (payline)
//    [3] bottom row
class ReelWidget : public QWidget {
    Q_OBJECT
public:
    explicit ReelWidget(QWidget* parent = nullptr);

    void setAdvancedMode(bool advanced);  // show 3 paylines + no Bonus symbols
    void startSpin();
    void stopSpin(const std::vector<Symbol>& topCentreBottom);
    bool isSpinning() const;

    void setWinHighlights(const std::vector<int>& rows);
    void setWinHighlight(int row);
    void clearWinHighlight();

signals:
    void stopped();   // emitted when reel reaches Idle after stopSpin()

protected:
    void paintEvent(QPaintEvent*) override;

private slots:
    void onAnimTick();
    void onPulseTick();

private:
    enum class State { Idle, Spinning, Stopping };

    // ── Animation ─────────────────────────────────────────────────────────────
    State              m_state        { State::Idle };
    std::deque<Symbol> m_strip;            // 4 symbols: above + 3 visible
    float              m_scrollOffset { 0.f };
    float              m_currentSpeed { 0.f };
    std::deque<Symbol> m_plan;             // pre-planned symbols for decel

    // ── Animation constants ───────────────────────────────────────────────────
    static constexpr float SPIN_SPEED     = 55.0f; // px / 16ms tick
    static constexpr float DECEL_PER_ADV  = 0.70f; // speed × factor per row crossed
    static constexpr float MIN_SPEED      =  4.0f; // floor speed during decel
    static constexpr int   N_PLAN_RANDOMS =  6;    // random rows before final symbols

    // ── Win highlight ─────────────────────────────────────────────────────────
    std::set<int> m_highlightRows;
    QTimer*       m_pulseTimer { nullptr };
    bool          m_pulseOn    { false };
    int           m_pulseCount { 0 };

    // ── Timers ────────────────────────────────────────────────────────────────
    bool    m_advancedMode { false };
    bool    m_includeBonus { true };
    QTimer* m_animTimer { nullptr };

    // ── Helpers ───────────────────────────────────────────────────────────────
    void drawRow(QPainter& p, const Symbol& sym, const QRect& r,
                 bool centre, bool highlighted, bool pulseOn) const;
    QColor bgForSymbol(SymbolType t) const;
    QColor fgForSymbol(SymbolType t) const;
    Symbol randomSymbol() const;
};
