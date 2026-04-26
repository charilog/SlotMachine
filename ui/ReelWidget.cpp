#include "ReelWidget.h"
#include "../core/RarityConfig.h"
#include <QPainter>
#include <QPainterPath>
#include <cstdlib>
#include <cmath>
#include <algorithm>

Symbol ReelWidget::randomSymbol() const {
    using S = SymbolType;

    // Full MAP includes Bonus
    static const struct { S sym; const char* key; } MAP[] = {
        { S::Cherry,"Cherry" },{ S::Lemon,"Lemon" },{ S::Orange,"Orange" },
        { S::Watermelon,"Watermelon" },{ S::Grape,"Grape" },{ S::Strawberry,"Strawberry" },
        { S::Bell,"Bell" },{ S::Bar,"Bar" },{ S::Bar2,"Bar2" },{ S::Bar3,"Bar3" },
        { S::Seven,"Seven" },{ S::Wild,"Wild" },{ S::Bonus,"Bonus" },
    };
    static constexpr int MAP_SIZE = (int)(sizeof(MAP)/sizeof(MAP[0]));

    // Two separate pre-built tables: with Bonus (Easy) and without (Advanced)
    static std::vector<SymbolType> tableWith;
    static std::vector<SymbolType> tableWithout;
    static bool builtWith    = false;
    static bool builtWithout = false;

    if (!builtWith || !builtWithout) {
        auto& cfg = RarityConfig::instance();

        // Find minimum weight (excluding zero and Bonus for base calc)
        double minW = 1e9;
        for (int i = 0; i < MAP_SIZE; ++i) {
            double v = cfg.rarityValue(QString::fromLatin1(MAP[i].key));
            if (v > 0.0 && MAP[i].sym != S::Bonus && v < minW) minW = v;
        }
        if (minW <= 0.0) minW = 1.0;

        if (!builtWith) {
            for (int i = 0; i < MAP_SIZE; ++i) {
                double v = cfg.rarityValue(QString::fromLatin1(MAP[i].key));
                if (v <= 0.0) continue;
                int stops = std::min(static_cast<int>(std::round(v / minW)), 30);
                for (int j = 0; j < stops; ++j) tableWith.push_back(MAP[i].sym);
            }
            builtWith = true;
        }
        if (!builtWithout) {
            for (int i = 0; i < MAP_SIZE; ++i) {
                if (MAP[i].sym == S::Bonus) continue;   // skip Bonus for Advanced
                double v = cfg.rarityValue(QString::fromLatin1(MAP[i].key));
                if (v <= 0.0) continue;
                int stops = std::min(static_cast<int>(std::round(v / minW)), 30);
                for (int j = 0; j < stops; ++j) tableWithout.push_back(MAP[i].sym);
            }
            builtWithout = true;
        }
    }

    const auto& tbl = m_includeBonus ? tableWith : tableWithout;
    if (tbl.empty()) return Symbol(S::Cherry);
    return Symbol(tbl[std::rand() % static_cast<int>(tbl.size())]);
}

ReelWidget::ReelWidget(QWidget* parent) : QWidget(parent) {
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMinimumSize(90, 270);

    // Initialise strip
    for (int i = 0; i < 4; ++i)
        m_strip.push_back(randomSymbol());

    m_animTimer = new QTimer(this);
    m_animTimer->setInterval(16);
    connect(m_animTimer, &QTimer::timeout, this, &ReelWidget::onAnimTick);

    m_pulseTimer = new QTimer(this);
    m_pulseTimer->setInterval(260);
    connect(m_pulseTimer, &QTimer::timeout, this, &ReelWidget::onPulseTick);
}

// ── Public API ────────────────────────────────────────────────────────────────
void ReelWidget::setAdvancedMode(bool advanced) {
    m_advancedMode = advanced;
    m_includeBonus = !advanced;  // Advanced mode has no Bonus scatter
    update();
}

void ReelWidget::startSpin() {
    clearWinHighlight();
    m_state        = State::Spinning;
    m_scrollOffset = 0.f;
    m_currentSpeed = SPIN_SPEED;
    m_plan.clear();
    m_animTimer->start();
}

void ReelWidget::stopSpin(const std::vector<Symbol>& topCentreBottom) {
    if (m_state == State::Idle) return;

    // ── Build pre-planned symbol queue ────────────────────────────────────────
    // Trace: after N advances the strip becomes [dummy, f0, f1, f2] at offset=0.
    // Symbols must be queued reversed: bottom enters first, top enters last.
    //   queue = [rand×N, f2, f1, f0, dummy]
    // After len(queue) advances → visible = [f0, f1, f2] ✓  (see class doc)
    m_plan.clear();
    for (int i = 0; i < N_PLAN_RANDOMS; ++i)
        m_plan.push_back(randomSymbol());

    if (topCentreBottom.size() >= 3) {
        m_plan.push_back(topCentreBottom[2]);   // bottom  (enters 1st)
        m_plan.push_back(topCentreBottom[1]);   // centre  (enters 2nd)
        m_plan.push_back(topCentreBottom[0]);   // top     (enters 3rd)
    }
    m_plan.push_back(randomSymbol());           // dummy "above" slot at snap

    m_state        = State::Stopping;
    m_currentSpeed = SPIN_SPEED;               // start decelerating from full speed
}

bool ReelWidget::isSpinning() const { return m_state != State::Idle; }

// ── Win highlight ─────────────────────────────────────────────────────────────
void ReelWidget::setWinHighlights(const std::vector<int>& rows) {
    m_highlightRows.clear();
    for (int r : rows) m_highlightRows.insert(r);
    m_pulseOn    = true;
    m_pulseCount = 0;
    if (!m_highlightRows.empty()) m_pulseTimer->start();
    update();
}
void ReelWidget::setWinHighlight(int row) { setWinHighlights({ row }); }
void ReelWidget::clearWinHighlight() {
    m_pulseTimer->stop();
    m_highlightRows.clear();
    m_pulseOn = false;
    update();
}

// ── Animation tick ────────────────────────────────────────────────────────────
void ReelWidget::onAnimTick() {
    const int rH = height() / 3;
    if (rH <= 0) return;

    m_scrollOffset += m_currentSpeed;

    while (m_scrollOffset >= static_cast<float>(rH)) {
        m_scrollOffset -= static_cast<float>(rH);

        if (m_state == State::Spinning) {
            // Random symbol enters from top
            m_strip.pop_back();
            m_strip.push_front(randomSymbol());

        } else if (m_state == State::Stopping) {
            // Decelerate speed for this advance
            m_currentSpeed = std::max(MIN_SPEED, m_currentSpeed * DECEL_PER_ADV);

            if (m_plan.empty()) {
                // Safety snap
                m_state = State::Idle;
                m_scrollOffset = 0.f;
                m_animTimer->stop();
                update();
                emit stopped();
                return;
            }

            // Pop next planned symbol into the strip
            m_strip.pop_back();
            m_strip.push_front(m_plan.front());
            m_plan.pop_front();

            if (m_plan.empty()) {
                // ── All planned symbols consumed ──────────────────────────────
                // Strip is now [dummy, f0, f1, f2], scrollOffset ≈ 0 → perfect snap
                m_state        = State::Idle;
                m_scrollOffset = 0.f;
                m_animTimer->stop();
                update();
                emit stopped();
                return;
            }
        }
    }
    update();
}

void ReelWidget::onPulseTick() {
    m_pulseOn = !m_pulseOn;
    if (++m_pulseCount > 23) {
        m_pulseTimer->stop();
        m_pulseOn = true;
    }
    update();
}

// ── Paint ─────────────────────────────────────────────────────────────────────
void ReelWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setClipRect(rect());

    const int W  = width();
    const int rH = height() / 3;

    p.fillRect(rect(), QColor(15, 15, 30));

    // strip[i] rendered at y = (i-1)*rH + scrollOffset
    // i=0 → y = -rH + offset  (entering from above)
    // i=1 → y = 0  + offset   (top row)
    // i=2 → y = rH + offset   (centre row)
    // i=3 → y = 2rH+ offset   (bottom row)
    for (int i = 0; i < 4 && i < (int)m_strip.size(); ++i) {
        int y = (i - 1) * rH + static_cast<int>(m_scrollOffset);
        if (y + rH <= 0 || y >= height()) continue;

        QRect rowRect(2, y + 2, W - 4, rH - 4);
        bool isCentre    = (i == 2);
        // Highlight only in Idle; strip index i → visible row (i-1)
        bool isHighlight = (m_state == State::Idle && m_highlightRows.count(i - 1) > 0);
        drawRow(p, m_strip[i], rowRect, isCentre, isHighlight, m_pulseOn);
    }

    // Payline indicators
    p.setPen(QPen(QColor(255, 200, 0, 160), 2, Qt::DashLine));
    int midY = height() / 2;
    p.drawLine(0, midY, W, midY);   // centre (always)
    if (m_advancedMode) {
        // Top payline
        p.setPen(QPen(QColor(100, 200, 255, 140), 1, Qt::DashLine));
        p.drawLine(0, height() / 6, W, height() / 6);
        // Bottom payline
        p.drawLine(0, height() * 5 / 6, W, height() * 5 / 6);
    }

    // Outer frame
    p.setPen(QPen(QColor(120, 100, 40), 3));
    p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 6, 6);
}

void ReelWidget::drawRow(QPainter& p, const Symbol& sym, const QRect& r,
                          bool centre, bool highlighted, bool pulseOn) const {
    QColor bg = bgForSymbol(sym.type());
    if (centre && !highlighted)      bg = bg.lighter(130);
    if (highlighted && pulseOn)      bg = bg.lighter(200);
    else if (highlighted)            bg = bg.lighter(155);

    QPainterPath path;
    path.addRoundedRect(r, 5, 5);
    p.fillPath(path, bg);

    if (highlighted && pulseOn) {
        for (int g = 4; g >= 1; --g) {
            p.setPen(QPen(QColor(255, 220, 0, 40 * g), g * 2));
            p.drawRoundedRect(r.adjusted(-g, -g, g, g), 6, 6);
        }
        p.setPen(QPen(QColor(255, 220, 0), 3));
        p.drawRoundedRect(r, 5, 5);
    } else if (centre) {
        p.setPen(QPen(QColor(255, 220, 0), 2));
        p.drawRoundedRect(r, 5, 5);
    }

    bool isTextSym = sym.isText();

    if (sym.type() == SymbolType::Bar2) {
        // Two stacked BAR lines
        QFont f(QStringLiteral("Arial"), r.height() * 20 / 100, QFont::Bold);
        p.setFont(f);
        p.setPen(fgForSymbol(sym.type()));
        int lineH = r.height() / 3;
        p.drawText(QRect(r.x(), r.y() + lineH/2,       r.width(), lineH), Qt::AlignCenter, QStringLiteral("BAR"));
        p.drawText(QRect(r.x(), r.y() + lineH/2 + lineH, r.width(), lineH), Qt::AlignCenter, QStringLiteral("BAR"));
    } else if (sym.type() == SymbolType::Bar3) {
        // Three stacked BAR lines
        QFont f(QStringLiteral("Arial"), r.height() * 16 / 100, QFont::Bold);
        p.setFont(f);
        p.setPen(fgForSymbol(sym.type()));
        int lineH = r.height() / 4;
        for (int li = 0; li < 3; ++li)
            p.drawText(QRect(r.x(), r.y() + lineH/2 + li * lineH, r.width(), lineH),
                       Qt::AlignCenter, QStringLiteral("BAR"));
    } else {
        int fontSize = isTextSym ? r.height() * 28 / 100 : r.height() * 42 / 100;
        QFont f(isTextSym ? QStringLiteral("Arial") : QStringLiteral("Segoe UI Emoji"),
                fontSize, QFont::Bold);
        p.setFont(f);
        p.setPen(fgForSymbol(sym.type()));
        p.drawText(r, Qt::AlignCenter, sym.emoji());
    }


}

QColor ReelWidget::bgForSymbol(SymbolType t) const {
    switch (t) {
        case SymbolType::Cherry:     return { 100, 15, 15  };
        case SymbolType::Lemon:      return { 100, 95,  0  };
        case SymbolType::Orange:     return { 120, 55,  0  };
        case SymbolType::Watermelon: return {  10, 80, 30  };
        case SymbolType::Grape:      return {  55,  0, 90  };
        case SymbolType::Strawberry: return { 120,  0, 40  };
        case SymbolType::Bell:       return { 110,100,  0  };
        case SymbolType::Bar:        return {   0, 60, 20  };
        case SymbolType::Bar2:       return {   0, 80, 40  };
        case SymbolType::Bar3:       return {   0,100, 60  };
        case SymbolType::Seven:      return {   0, 20,110  };
        case SymbolType::Wild:       return { 110,  0,110  };
        default:                     return {  40, 40, 40  };
    }
}

QColor ReelWidget::fgForSymbol(SymbolType t) const {
    switch (t) {
        case SymbolType::Cherry:     return { 255,120,120 };
        case SymbolType::Lemon:      return { 255,255, 80 };
        case SymbolType::Orange:     return { 255,165, 50 };
        case SymbolType::Watermelon: return {  80,255,120 };
        case SymbolType::Grape:      return { 200,100,255 };
        case SymbolType::Strawberry: return { 255, 80,120 };
        case SymbolType::Bell:       return { 255,240, 80 };
        case SymbolType::Bar:        return { 100,255,140 };
        case SymbolType::Bar2:       return { 120,255,160 };
        case SymbolType::Bar3:       return { 150,255,190 };
        case SymbolType::Seven:      return {  80,160,255 };
        case SymbolType::Wild:       return { 255, 80,255 };
        default:                     return Qt::white;
    }
}
