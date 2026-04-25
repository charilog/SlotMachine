#pragma once
#include <QWidget>
#include <QTimer>
#include <QPushButton>
#include <QLabel>
#include "../core/SoundEngine.h"

// ─── DoubleUpWidget ───────────────────────────────────────────────────────────
// Gamble / Double-Up overlay.
// After opening: player presses SPACE/STOP → 50/50 red-black result.
//   Win  → amount ×2, offer to gamble again or collect
//   Lose → emit lostAll()
//   Collect (any time after win) → emit collected(totalAmount)
class DoubleUpWidget : public QWidget {
    Q_OBJECT
public:
    explicit DoubleUpWidget(SoundEngine* sound, QWidget* parent = nullptr);

    void startGamble(int amount);

signals:
    void collected(int totalAmount);   // full amount to count into credits
    void lostAll();                    // pending win lost

protected:
    void paintEvent(QPaintEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void showEvent(QShowEvent*) override;
    void resizeEvent(QResizeEvent*) override;

private slots:
    void onSpinTimer();
    void onStopPressed();
    void onCollectPressed();
    void onDoubleAgainPressed();

private:
    enum class State { Spinning, Won, Lost };

    SoundEngine* m_sound    { nullptr };
    int          m_amount   { 0 };      // current (doubles on each win)
    State        m_state    { State::Spinning };
    bool         m_isRed    { true };
    int          m_faceIdx  { 0 };
    int          m_flashFrames { 0 };
    bool         m_flashOn  { false };

    QTimer*      m_spinTimer  { nullptr };
    QTimer*      m_flashTimer { nullptr };

    QLabel*      m_titleLabel    { nullptr };
    QLabel*      m_amountLabel   { nullptr };
    QLabel*      m_resultLabel   { nullptr };
    QPushButton* m_stopBtn       { nullptr };
    QPushButton* m_collectBtn    { nullptr };
    QPushButton* m_doubleAgainBtn{ nullptr };

    QRect  m_cardRect;

    void buildUI();
    void layoutWidgets();
    void showResult(bool won);
    void updateAmountLabel();

    struct CardFace { bool red; QString suit; };
    static const CardFace FACES[];
    static constexpr int  NUM_FACES = 4;
};
