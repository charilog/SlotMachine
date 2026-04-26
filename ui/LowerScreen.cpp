#include "LowerScreen.h"
#include "WinTableWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <algorithm>

LowerScreen::LowerScreen(SlotMachine* machine, SoundEngine* sound, QWidget* parent)
    : QWidget(parent), m_machine(machine), m_sound(sound)
{
    buildUI();

    connect(m_machine, &SlotMachine::spinFinished,
            this, &LowerScreen::onSpinFinished);
    connect(m_machine->gameState(), &GameState::creditsChanged,
            this, &LowerScreen::onCreditsChanged);
    // levelChanged is handled manually after animation — no auto-connect
}

void LowerScreen::buildUI() {
    setStyleSheet(QStringLiteral("QWidget{background-color:#12122a;color:white;}"));

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(16, 12, 16, 12);
    root->setSpacing(8);

    // Title
    QLabel* title = new QLabel(QStringLiteral("🎰  SLOT MACHINE  — EASY MODE  🎰"), this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(QStringLiteral(
        "font-size:22px;font-weight:bold;color:#FFD700;padding:8px;"
        "background:rgba(255,215,0,0.08);border:1px solid #FFD700;border-radius:6px;"));
    root->addWidget(title);

    // Bonus hint
    m_progressLabel = new QLabel(QStringLiteral("🎯 Land 3 BONUS symbols anywhere to enter Advanced Mode!"), this);
    m_progressLabel->setAlignment(Qt::AlignCenter);
    m_progressLabel->setStyleSheet(QStringLiteral("font-size:11px;color:#FFB347;font-weight:bold;"));
    root->addWidget(m_progressLabel);

    // Reels + Win Tables
    QHBoxLayout* reelRow = new QHBoxLayout();
    reelRow->setSpacing(6);

    auto* leftTable  = new WinTableWidget(WinTableWidget::Mode::Easy, this);
    auto* rightTable = new WinTableWidget(WinTableWidget::Mode::Advanced, this);
    reelRow->addWidget(leftTable);

    QHBoxLayout* reelInner = new QHBoxLayout();
    reelInner->setSpacing(4);
    reelInner->setContentsMargins(0,0,0,0);
    for (int i = 0; i < 3; ++i) {
        auto* rw = new ReelWidget(this);
        rw->setFixedSize(100, 300);
        m_reelWidgets.push_back(rw);
        reelInner->addWidget(rw);
    }
    reelRow->addLayout(reelInner);
    reelRow->addWidget(rightTable);
    root->addLayout(reelRow);

    // Win display (shows pending win amount, counts down during animation)
    m_winLabel = new QLabel(QStringLiteral(""), this);
    m_winLabel->setAlignment(Qt::AlignCenter);
    m_winLabel->setFixedHeight(38);
    m_winLabel->setStyleSheet(QStringLiteral(
        "font-size:22px;font-weight:bold;color:#FFD700;"));
    root->addWidget(m_winLabel);

    // Status
    m_statusLabel = new QLabel(QStringLiteral("Press SPIN to play!"), this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet(QStringLiteral("font-size:13px;color:#ccc;"));
    root->addWidget(m_statusLabel);

    // Credits / Bet row
    QHBoxLayout* infoRow = new QHBoxLayout();
    m_creditsLabel = new QLabel(QStringLiteral("Credits: 100"), this);
    m_creditsLabel->setStyleSheet(QStringLiteral(
        "font-size:16px;font-weight:bold;color:#4CAF50;"));
    infoRow->addWidget(m_creditsLabel);
    infoRow->addStretch();

    auto makeSideBtn = [this](const QString& txt) {
        auto* b = new QPushButton(txt, this);
        b->setFixedSize(36, 36);
        b->setStyleSheet(QStringLiteral(
            "QPushButton{background:#2a2a4a;color:white;font-size:18px;"
            "border-radius:4px;border:1px solid #555;}"
            "QPushButton:hover{background:#3a3a6a;}"
            "QPushButton:pressed{background:#1a1a3a;}"));
        return b;
    };
    m_betMinus = makeSideBtn(QStringLiteral("−"));
    m_betLabel  = new QLabel(QStringLiteral("Bet: 1"), this);
    m_betLabel->setStyleSheet(QStringLiteral(
        "font-size:16px;font-weight:bold;color:#2196F3;padding:0 10px;"));
    m_betPlus = makeSideBtn(QStringLiteral("+"));
    infoRow->addWidget(m_betMinus);
    infoRow->addWidget(m_betLabel);
    infoRow->addWidget(m_betPlus);
    root->addLayout(infoRow);

    // SPIN button
    m_spinBtn = new QPushButton(QStringLiteral("🎰  SPIN!"), this);
    m_spinBtn->setFixedHeight(58);
    m_spinBtn->setStyleSheet(QStringLiteral(
        "QPushButton{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,"
        "  stop:0 #FFD700,stop:1 #FFA500);color:#12122a;font-size:22px;"
        "  font-weight:bold;border-radius:10px;border:2px solid #FF8C00;}"
        "QPushButton:hover{background:#FFD700;}"
        "QPushButton:pressed{background:#FFA500;}"
        "QPushButton:disabled{background:#555;color:#888;border-color:#444;}"));
    root->addWidget(m_spinBtn);

    // COLLECT button (hidden until win)
    m_collectBtn = new QPushButton(QStringLiteral("✅  COLLECT"), this);
    m_collectBtn->setFixedHeight(48);
    m_collectBtn->hide();
    m_collectBtn->setStyleSheet(QStringLiteral(
        "QPushButton{background:#1a4a1a;color:#aaffaa;font-size:17px;"
        "  font-weight:bold;border-radius:8px;border:2px solid #44aa44;}"
        "QPushButton:hover{background:#2a6a2a;}"
        "QPushButton:disabled{background:#333;color:#666;border-color:#444;}"));
    root->addWidget(m_collectBtn);

    // DOUBLE UP button (hidden until win)
    m_doubleUpBtn = new QPushButton(QStringLiteral("🎲  DOUBLE UP?"), this);
    m_doubleUpBtn->setFixedHeight(44);
    m_doubleUpBtn->hide();
    m_doubleUpBtn->setStyleSheet(QStringLiteral(
        "QPushButton{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,"
        "  stop:0 #9400D3,stop:1 #6600AA);color:white;font-size:16px;"
        "  font-weight:bold;border-radius:8px;border:2px solid #CC44FF;}"
        "QPushButton:hover{background:#AA00EE;}"));
    root->addWidget(m_doubleUpBtn);

    // Double-Up overlay
    m_doubleUp = new DoubleUpWidget(m_sound, this);

    // Count-up animation timer
    m_countTimer = new QTimer(this);
    m_countTimer->setInterval(25);

    // Connections
    connect(m_spinBtn,     &QPushButton::clicked, this, &LowerScreen::onSpinClicked);
    connect(m_betPlus,     &QPushButton::clicked, this, &LowerScreen::onBetPlus);
    connect(m_betMinus,    &QPushButton::clicked, this, &LowerScreen::onBetMinus);
    connect(m_collectBtn,  &QPushButton::clicked, this, [this]() {
        startCollectAnimation(m_machine->pendingWin());
    });
    connect(m_doubleUpBtn, &QPushButton::clicked, this, [this]() {
        m_collectBtn->setEnabled(false);
        m_doubleUpBtn->setEnabled(false);
        m_doubleUp->startGamble(m_machine->pendingWin());
    });
    connect(m_countTimer, &QTimer::timeout, this, &LowerScreen::onCountTick);

    connect(m_doubleUp, &DoubleUpWidget::collected, this, [this](int totalAmount) {
        // Player collected after doubling — start count animation with new total
        m_collectBtn->hide();
        m_doubleUpBtn->hide();
        startCollectAnimation(totalAmount);
    });
    connect(m_doubleUp, &DoubleUpWidget::lostAll, this, [this]() {
        // Lost the gamble — clear win display, re-enable spin
        m_winLabel->setText(QStringLiteral(""));
        m_statusLabel->setText(QStringLiteral("No luck — try again!"));
        m_collectBtn->hide();
        m_doubleUpBtn->hide();
        finishRound();
    });

    syncLabels();
}

// ─── Spin ─────────────────────────────────────────────────────────────────────
void LowerScreen::onSpinClicked() {
    if (!m_machine->gameState()->canSpin()) {
        m_statusLabel->setText(QStringLiteral("Not enough credits!"));
        m_sound->play(SoundEngine::Sound::Click);
        return;
    }
    m_sound->play(SoundEngine::Sound::Click);
    for (auto* rw : m_reelWidgets) { rw->startSpin(); rw->clearWinHighlight(); }
    m_sound->startSpin();
    m_spinBtn->setEnabled(false);
    m_collectBtn->hide();
    m_doubleUpBtn->hide();
    m_winLabel->setText(QStringLiteral(""));
    m_statusLabel->setText(QStringLiteral("Spinning…"));

    m_machine->spin();
    stopReelsSequentially();
}

void LowerScreen::stopReelsSequentially() {
    // Start first reel after BASE_DELAY, then chain left→right
    QTimer::singleShot(400, this, [this]() { stopNextReel(0); });
}

void LowerScreen::stopNextReel(int idx) {
    int n = static_cast<int>(m_reelWidgets.size());
    if (idx >= n) {
        // All reels stopped — show result after brief pause
        QTimer::singleShot(200, this, [this]() {
            m_sound->stopSpin();

            // ── Bonus scatter: 3+ symbols anywhere → Advanced Mode ──
            if (m_machine->bonusTriggered()) {
                m_spinBtn->setEnabled(false);
                m_collectBtn->hide();
                m_doubleUpBtn->hide();
                m_winLabel->setText(QStringLiteral("🎯🎯🎯  BONUS ROUND!"));
                m_statusLabel->setText(
                    QStringLiteral("%1 FREE SPINS awarded!")
                    .arg(GameState::FREE_SPINS_AWARD));
                // Trigger AFTER setting UI (avoids immediate levelChanged signal)
                m_machine->gameState()->triggerBonusRound();
                QTimer::singleShot(2000, this, [this]() {
                    emit levelUpRequested();
                });
                return;
            }

            WinResult r   = m_machine->lastResult();
            int pending   = m_machine->pendingWin();
            applyWinHighlights();
            if (r.isWin && pending > 0) {
                m_winLabel->setText(QStringLiteral("WIN:  %1  credits").arg(pending));
                m_statusLabel->setText(r.description);
                m_collectBtn->setEnabled(true);  m_collectBtn->show();
                m_doubleUpBtn->setEnabled(true); m_doubleUpBtn->show();
                if (pending >= 500)     m_sound->play(SoundEngine::Sound::Jackpot);
                else if (pending >= 50) m_sound->play(SoundEngine::Sound::BigWin);
                else                    m_sound->play(SoundEngine::Sound::Win);
            } else {
                m_statusLabel->setText(QStringLiteral("No luck — try again!"));
                finishRound();
            }
        });
        return;
    }

    // Tell this reel to decelerate
    auto col = m_machine->reelColumn(idx);
    m_reelWidgets[idx]->stopSpin(col);
    m_sound->play(SoundEngine::Sound::ReelStop);

    // When it fully stops → move to next reel
    connect(m_reelWidgets[idx], &ReelWidget::stopped,
            this, [this, idx]() {
                disconnect(m_reelWidgets[idx], &ReelWidget::stopped, this, nullptr);
                stopNextReel(idx + 1);
            });
}

// ─── Count-up animation ───────────────────────────────────────────────────────
void LowerScreen::startCollectAnimation(int amount) {
    m_countRemaining = amount;
    m_collectBtn->setEnabled(false);
    m_doubleUpBtn->hide();
    if (m_sound) m_sound->startCoins();
    m_countTimer->start();
}

void LowerScreen::onCountTick() {
    int step = qMax(1, m_countRemaining / 25);
    step = qMin(step, m_countRemaining);
    m_countRemaining -= step;

    // Add step to credits
    m_machine->collectPendingWin(step);

    if (m_countRemaining > 0) {
        m_winLabel->setText(QStringLiteral("WIN:  %1  credits").arg(m_countRemaining));
    } else {
        m_countTimer->stop();
        if (m_sound) m_sound->stopCoins();
        m_winLabel->setText(QStringLiteral(""));
        m_collectBtn->hide();
        finishRound();
    }
}

void LowerScreen::finishRound() {
    m_spinBtn->setEnabled(true);
    m_machine->gameState()->checkLevelUp();
    syncLabels();
}

// ─── Helpers ─────────────────────────────────────────────────────────────────
void LowerScreen::onSpinFinished(bool, int, const QString&) { /* deferred */ }

void LowerScreen::onCreditsChanged(int credits) {
    m_creditsLabel->setText(QStringLiteral("Credits: %1").arg(credits));
}

void LowerScreen::onBetPlus() {
    m_machine->gameState()->setBet(m_machine->gameState()->bet() + 1);
    syncLabels();
}

void LowerScreen::onBetMinus() {
    m_machine->gameState()->setBet(m_machine->gameState()->bet() - 1);
    syncLabels();
}

void LowerScreen::syncLabels() {
    GameState* gs = m_machine->gameState();
    m_creditsLabel->setText(QStringLiteral("Credits: %1").arg(gs->credits()));
    m_betLabel->setText(QStringLiteral("Bet: %1").arg(gs->bet()));
    // Progress label is static hint — no update needed
}

void LowerScreen::applyWinHighlights() {
    for (auto* rw : m_reelWidgets) rw->clearWinHighlight();
    WinResult r = m_machine->lastResult();
    if (!r.isWin || r.winCount == 0) return;
    // Wins count from RIGHT → highlight last winCount reels
    int total = static_cast<int>(m_reelWidgets.size());
    int n = std::min(r.winCount, total);
    for (int i = total - n; i < total; ++i)
        m_reelWidgets[i]->setWinHighlight(1);
}

void LowerScreen::refreshDisplay() {
    syncLabels();
    if (m_doubleUp) m_doubleUp->resize(size());
    // Re-enable spin when returning from Advanced mode
    m_spinBtn->setEnabled(true);
    m_collectBtn->hide();
    m_doubleUpBtn->hide();
    m_winLabel->setText(QStringLiteral(""));
    m_progressLabel->setText(
        QStringLiteral("🎯 Land 3 BONUS symbols anywhere to enter Advanced Mode!"));
}
