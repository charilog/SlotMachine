#pragma once
#include <QWidget>

// ─── WinTableWidget ───────────────────────────────────────────────────────────
// Decorative panel shown left / right of the reel area.
// Displays the paytable for either Easy (3-reel) or Advanced (5-reel) mode.
// Pure custom paint — no child widgets, no layout overhead.
class WinTableWidget : public QWidget {
    Q_OBJECT
public:
    enum class Mode { Easy, Advanced };

    explicit WinTableWidget(Mode mode, QWidget* parent = nullptr);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent*) override;

private:
    Mode m_mode;

    struct Row {
        QString symbols;   // e.g. "🍒 🍒 🍒"
        QString mult;      // e.g. "×3"
        bool    special;   // gold highlight for top prizes
    };

    void drawEasy(QPainter& p) const;
    void drawAdvanced(QPainter& p) const;
    void drawRows(QPainter& p, const QVector<Row>& rows,
                  int x, int y, int w, int rowH) const;
    void drawTitle(QPainter& p, const QString& txt,
                   int x, int y, int w, int h) const;
};
