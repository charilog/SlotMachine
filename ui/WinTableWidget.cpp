#include "WinTableWidget.h"
#include "../core/RarityConfig.h"
#include <QPainter>
#include <QPainterPath>

WinTableWidget::WinTableWidget(Mode mode, QWidget* parent)
    : QWidget(parent), m_mode(mode)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setMinimumWidth(138);
    setMaximumWidth(155);
}

QSize WinTableWidget::sizeHint() const { return { 145, 500 }; }

void WinTableWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    if (m_mode == Mode::Easy)     drawEasy(p);
    else                           drawAdvanced(p);
}

// ── shared helpers ─────────────────────────────────────────────────────────────
void WinTableWidget::drawTitle(QPainter& p, const QString& txt,
                                int x, int y, int w, int h) const {
    QLinearGradient grad(x, y, x, y + h);
    grad.setColorAt(0, QColor(180, 130, 0));
    grad.setColorAt(1, QColor(120, 80, 0));
    QPainterPath path;
    path.addRoundedRect(x, y, w, h, 6, 6);
    p.fillPath(path, grad);
    p.setPen(QPen(QColor(255, 220, 60), 1.5));
    p.drawRoundedRect(x, y, w, h, 6, 6);

    QFont f(QStringLiteral("Arial"), 9, QFont::Bold);
    p.setFont(f);
    p.setPen(QColor(255, 240, 160));
    p.drawText(QRect(x, y, w, h), Qt::AlignCenter, txt);
}

void WinTableWidget::drawRows(QPainter& p, const QVector<Row>& rows,
                               int x, int y, int w, int rowH) const {
    for (const auto& row : rows) {
        QColor bg  = row.special ? QColor(60, 50, 0, 200) : QColor(18, 18, 40, 200);
        QColor border = row.special ? QColor(200, 160, 0, 120) : QColor(60, 60, 100, 80);

        QPainterPath path;
        path.addRoundedRect(x, y, w, rowH - 2, 4, 4);
        p.fillPath(path, bg);
        p.setPen(QPen(border, 1));
        p.drawRoundedRect(x, y, w, rowH - 2, 4, 4);

        // Symbol column
        QFont symFont(QStringLiteral("Segoe UI Emoji"), 9);
        p.setFont(symFont);
        p.setPen(row.special ? QColor(255, 230, 100) : QColor(210, 210, 210));
        p.drawText(QRect(x + 3, y, w - 40, rowH), Qt::AlignVCenter | Qt::AlignLeft,
                   row.symbols);

        // Multiplier column
        QFont multFont(QStringLiteral("Arial"), 8, QFont::Bold);
        p.setFont(multFont);
        p.setPen(row.special ? QColor(255, 200, 0) : QColor(100, 200, 255));
        p.drawText(QRect(x + w - 38, y, 36, rowH), Qt::AlignVCenter | Qt::AlignRight,
                   row.mult);

        y += rowH;
    }
}

// ── Easy mode (left panel) ─────────────────────────────────────────────────────
void WinTableWidget::drawEasy(QPainter& p) const {
    int W = width(), H = height();

    // Background
    QLinearGradient bg(0, 0, 0, H);
    bg.setColorAt(0, QColor(10, 10, 28));
    bg.setColorAt(1, QColor(5, 5, 18));
    p.fillRect(rect(), bg);

    // Outer border
    p.setPen(QPen(QColor(120, 90, 20), 2));
    p.drawRoundedRect(1, 1, W-2, H-2, 8, 8);

    int pad = 6;
    int y   = pad;
    int w   = W - pad*2;

    drawTitle(p, QStringLiteral("EASY  ·  3 REELS"), pad, y, w, 22);
    y += 26;
    drawTitle(p, QStringLiteral("PAY TABLE"), pad, y, w, 18);
    y += 22;

    const QVector<Row> rows = {
        { QStringLiteral("★ ★ ★"),      QStringLiteral("×1000"), true  },
        { QStringLiteral("7  7  7"),      QStringLiteral("×200"),  true  },
        { QStringLiteral("▋▋▋ ×3"),      QStringLiteral("×100"),  true  },
        { QStringLiteral("▋▋ ×3"),        QStringLiteral("×60"),   true  },
        { QStringLiteral("▋ ×3"),          QStringLiteral("×30"),   false },
        { QStringLiteral("🔔 🔔 🔔"),    QStringLiteral("×20"),   false },
        { QStringLiteral("🍓 🍓 🍓"),    QStringLiteral("×12"),   false },
        { QStringLiteral("🍇 🍇 🍇"),    QStringLiteral("×10"),   false },
        { QStringLiteral("🍉 🍉 🍉"),    QStringLiteral("×8"),    false },
        { QStringLiteral("🍊 🍊 🍊"),    QStringLiteral("×6"),    false },
        { QStringLiteral("🍋 🍋 🍋"),    QStringLiteral("×4"),    false },
        { QStringLiteral("🍒 🍒 🍒"),    QStringLiteral("×3"),    false },
        { QStringLiteral("any 🍒 🍒"),   QStringLiteral("×2"),    false },
    };

    int rowH = qMin(28, (H - y - pad) / rows.size());
    drawRows(p, rows, pad, y, w, rowH);
    y += rows.size() * rowH + 4;

    // Footer note
    QFont noteFont(QStringLiteral("Arial"), 7);
    p.setFont(noteFont);
    p.setPen(QColor(120, 120, 160));
    p.drawText(QRect(pad, y, w, 20), Qt::AlignCenter,
               QStringLiteral("★ Wild matches all"));
}

// ── Symbol Set panel (right panel — both modes) ───────────────────────────────
void WinTableWidget::drawAdvanced(QPainter& p) const {
    int W = width(), H = height();

    // Deep blue-purple background
    QLinearGradient bg(0, 0, 0, H);
    bg.setColorAt(0, QColor(8, 10, 28));
    bg.setColorAt(1, QColor(4, 6, 18));
    p.fillRect(rect(), bg);

    // Purple border
    p.setPen(QPen(QColor(80, 50, 140), 2));
    p.drawRoundedRect(1, 1, W-2, H-2, 8, 8);

    int pad = 6;
    int y   = pad;
    int w   = W - pad*2;

    drawTitle(p, QStringLiteral("SYMBOL  SET"), pad, y, w, 22);
    y += 26;

    // Column headers
    {
        QFont hf(QStringLiteral("Arial"), 7, QFont::Bold);
        p.setFont(hf);
        p.setPen(QColor(180, 150, 255));
        int col1 = w * 55 / 100;
        p.drawText(QRect(pad, y, col1, 14), Qt::AlignCenter, QStringLiteral("Symbol"));
        p.drawText(QRect(pad + col1, y, w - col1, 14), Qt::AlignCenter, QStringLiteral("Rarity"));
        y += 14;
        p.setPen(QPen(QColor(80, 60, 120), 1));
        p.drawLine(pad, y, pad + w, y);
        y += 3;
    }

    struct SymRow {
        QString sym;
        QString name;
        QString rarity;
        QColor  accent;
    };

    const QVector<SymRow> rows = {
        { QStringLiteral("🍒"), QStringLiteral("Cherry"),    RarityConfig::instance().rarityStr(QStringLiteral("Cherry")),     QColor(200, 60, 60)  },
        { QStringLiteral("🍋"), QStringLiteral("Lemon"),     RarityConfig::instance().rarityStr(QStringLiteral("Lemon")),      QColor(200,200, 40)  },
        { QStringLiteral("🍊"), QStringLiteral("Orange"),    RarityConfig::instance().rarityStr(QStringLiteral("Orange")),     QColor(220,120, 20)  },
        { QStringLiteral("🍉"), QStringLiteral("Melon"),     RarityConfig::instance().rarityStr(QStringLiteral("Watermelon")), QColor( 40,180, 80)  },
        { QStringLiteral("🍇"), QStringLiteral("Grape"),     RarityConfig::instance().rarityStr(QStringLiteral("Grape")),      QColor(160, 60,220)  },
        { QStringLiteral("🍓"), QStringLiteral("Berry"),     RarityConfig::instance().rarityStr(QStringLiteral("Strawberry")), QColor(220, 50,100)  },
        { QStringLiteral("🔔"), QStringLiteral("Bell"),      RarityConfig::instance().rarityStr(QStringLiteral("Bell")),       QColor(220,200, 40)  },
        { QStringLiteral("▋"),  QStringLiteral("BAR"),       RarityConfig::instance().rarityStr(QStringLiteral("Bar")),        QColor( 60,200,100)  },
        { QStringLiteral("▋▋"), QStringLiteral("2BAR"),      RarityConfig::instance().rarityStr(QStringLiteral("Bar2")),       QColor( 80,220,130)  },
        { QStringLiteral("▋▋▋"),QStringLiteral("3BAR"),      RarityConfig::instance().rarityStr(QStringLiteral("Bar3")),       QColor(100,240,160)  },
        { QStringLiteral("7"),  QStringLiteral("Seven"),     RarityConfig::instance().rarityStr(QStringLiteral("Seven")),      QColor( 80,140,255)  },
        { QStringLiteral("★"),  QStringLiteral("Wild"),      RarityConfig::instance().rarityStr(QStringLiteral("Wild")),       QColor(255, 80,255)  },
    };

    int rowH = qMax(18, qMin(26, (H - y - pad - 20) / rows.size()));
    int col1  = w * 55 / 100;

    for (const auto& row : rows) {
        // Row background — subtle tint matching accent colour
        QColor bg2(row.accent.red()/6, row.accent.green()/6, row.accent.blue()/6, 180);
        QPainterPath path;
        path.addRoundedRect(pad, y, w, rowH - 2, 3, 3);
        p.fillPath(path, bg2);

        // Left accent bar
        p.fillRect(pad, y, 3, rowH - 2, row.accent);

        // Emoji/symbol
        QFont symF(QStringLiteral("Segoe UI Emoji"), rowH * 42 / 100);
        p.setFont(symF);
        p.setPen(row.accent.lighter(160));
        p.drawText(QRect(pad + 5, y, 22, rowH), Qt::AlignVCenter | Qt::AlignLeft, row.sym);

        // Name
        QFont nameF(QStringLiteral("Arial"), 7, QFont::Bold);
        p.setFont(nameF);
        p.setPen(QColor(210, 210, 230));
        p.drawText(QRect(pad + 27, y, col1 - 27, rowH), Qt::AlignVCenter | Qt::AlignLeft, row.name);

        // Rarity % — right column, colour-coded: green=common, yellow=medium, orange=rare
        QColor rarCol;
        // Colour relative to max value in config (dynamic thresholds)
        double pct = row.rarity.trimmed().chopped(1).toDouble();
        auto& cfg  = RarityConfig::instance();
        double maxV = 0.0;
        for (const auto& k : { "Cherry","Lemon","Orange","Watermelon","Grape",
                                "Strawberry","Bell","Bar","Bar2","Bar3","Seven","Wild" })
            maxV = qMax(maxV, cfg.rarityValue(QString::fromLatin1(k)));
        double rel = (maxV > 0.0) ? pct / maxV : 0.0;
        if      (rel >= 0.50) rarCol = QColor( 80,200, 80);
        else if (rel >= 0.25) rarCol = QColor(220,200, 60);
        else if (rel >= 0.12) rarCol = QColor(240,140, 40);
        else                  rarCol = QColor(255, 80, 80);

        QFont rarF(QStringLiteral("Arial"), 7, QFont::Bold);
        p.setFont(rarF);
        p.setPen(rarCol);
        p.drawText(QRect(pad + col1, y, w - col1, rowH),
                   Qt::AlignVCenter | Qt::AlignCenter, row.rarity);

        y += rowH;
    }

    // Footer
    QFont noteF(QStringLiteral("Arial"), 6);
    p.setFont(noteF);
    p.setPen(QColor(100, 80, 140));
    p.drawText(QRect(pad, y + 3, w, 10), Qt::AlignCenter,
               QStringLiteral("★ Wild matches all"));

}
