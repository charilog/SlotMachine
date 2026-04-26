#include "UpperScreen.h"
#include "WinTableWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <algorithm>

UpperScreen::UpperScreen(SlotMachine* machine, SoundEngine* sound,
                             GameState* mainCredits, QWidget* parent)
    : QWidget(parent), m_machine(machine), m_sound(sound), m_mainCredits(mainCredits)
{
    buildUI();
    connect(m_machine, &SlotMachine::spinFinished,
            this, &UpperScreen::onSpinFinished);
    connect(m_machine->gameState(), &GameState::creditsChanged,
            this, &UpperScreen::onCreditsChanged);
    if (m_mainCredits)
        connect(m_mainCredits, &GameState::creditsChanged,
                this, [this](int) { syncLabels(); });
    // When free spins run out, consumeFreeSpin() sets level back to Easy
    // Use that signal to auto-return after animation completes
    connect(m_machine->gameState(), &GameState::levelChanged,
            this, [this](GameLevel lvl) {
                if (lvl == GameLevel::Easy && isVisible()) {
                    // Delay so animation can finish first
                    QTimer::singleShot(2400, this, [this]() {
                        if (!m_machine->gameState()->isInFreeSpins())
                            emit backRequested();
                    });
                }
            });
}

void UpperScreen::buildUI() {
    setStyleSheet(QStringLiteral("QWidget{background-color:#0a0a1f;color:white;}"));
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(14, 10, 14, 10);
    root->setSpacing(7);

    // Top bar
    QHBoxLayout* topBar = new QHBoxLayout();
    m_backBtn = new QPushButton(QStringLiteral("← Easy Mode"), this);
    m_backBtn->setStyleSheet(QStringLiteral(
        "QPushButton{background:#1e1e3a;color:#aaa;padding:6px 12px;"
        "border-radius:4px;border:1px solid #444;font-size:11px;}"
        "QPushButton:hover{background:#2a2a4a;}"));
    topBar->addWidget(m_backBtn);
    QLabel* title = new QLabel(QStringLiteral("🎰  UPPER DECK  —  ADVANCED MODE  🎰"), this);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(QStringLiteral(
        "font-size:19px;font-weight:bold;color:#FF4500;padding:6px;"
        "background:rgba(255,69,0,0.08);border:1px solid #FF4500;border-radius:6px;"));
    topBar->addWidget(title, 1);
    QLabel* badge = new QLabel(QStringLiteral("5 REELS · 3 PAYLINES"), this);
    badge->setStyleSheet(QStringLiteral(
        "font-size:10px;color:#FF6347;border:1px solid #FF4500;"
        "border-radius:4px;padding:4px 8px;background:rgba(255,69,0,0.1);"));
    topBar->addWidget(badge);
    root->addLayout(topBar);

    QLabel* plbl = new QLabel(QStringLiteral("▶ Paylines: TOP  |  CENTRE  |  BOTTOM ◀"), this);
    plbl->setAlignment(Qt::AlignCenter);
    plbl->setStyleSheet(QStringLiteral(
        "font-size:11px;color:#FF8C69;background:rgba(255,69,0,0.08);padding:3px;border-radius:4px;"));
    root->addWidget(plbl);

    // 5 Reels + Win Tables
    QHBoxLayout* reelRow = new QHBoxLayout();
    reelRow->setSpacing(6);

    auto* leftTable  = new WinTableWidget(WinTableWidget::Mode::Advanced, this);
    auto* rightTable = new WinTableWidget(WinTableWidget::Mode::Advanced, this);
    reelRow->addWidget(leftTable);

    QHBoxLayout* reelInner = new QHBoxLayout();
    reelInner->setSpacing(3);
    reelInner->setContentsMargins(0,0,0,0);
    for (int i = 0; i < 5; ++i) {
        auto* rw = new ReelWidget(this);
        rw->setFixedSize(92, 276);
        rw->setAdvancedMode(true);   // show 3 paylines
        m_reelWidgets.push_back(rw);
        reelInner->addWidget(rw);
    }
    reelRow->addLayout(reelInner);
    reelRow->addWidget(rightTable);
    root->addLayout(reelRow);

    // Win display
    m_winLabel = new QLabel(QStringLiteral(""), this);
    m_winLabel->setAlignment(Qt::AlignCenter);
    m_winLabel->setFixedHeight(36);
    m_winLabel->setStyleSheet(QStringLiteral(
        "font-size:21px;font-weight:bold;color:#FF4500;"));
    root->addWidget(m_winLabel);

    m_statusLabel = new QLabel(QStringLiteral("5 reels, 3 paylines — bigger wins await!"), this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet(QStringLiteral("font-size:12px;color:#ccc;"));
    root->addWidget(m_statusLabel);

    // Credits row — MAIN (left) | BET (centre) | BONUS (right)
    QHBoxLayout* infoRow = new QHBoxLayout();

    // Main credits (lower screen)
    QVBoxLayout* mainCol = new QVBoxLayout();
    QLabel* mainLbl = new QLabel(QStringLiteral("MAIN"), this);
    mainLbl->setStyleSheet(QStringLiteral("font-size:9px;color:#aaa;font-weight:bold;"));
    m_mainCreditsLabel = new QLabel(QStringLiteral("0"), this);
    m_mainCreditsLabel->setStyleSheet(QStringLiteral(
        "font-size:16px;font-weight:bold;color:#4CAF50;"
        "background:#0a1a0a;border:2px solid #2a6a2a;border-radius:6px;padding:2px 8px;"));
    mainCol->addWidget(mainLbl, 0, Qt::AlignCenter);
    mainCol->addWidget(m_mainCreditsLabel);
    infoRow->addLayout(mainCol);

    infoRow->addStretch();

    // Bonus credits (this screen)
    QVBoxLayout* bonusCol = new QVBoxLayout();
    QLabel* bonusLbl = new QLabel(QStringLiteral("BONUS"), this);
    bonusLbl->setStyleSheet(QStringLiteral("font-size:9px;color:#FFD700;font-weight:bold;"));
    m_creditsLabel = new QLabel(QStringLiteral("0"), this);
    m_creditsLabel->setStyleSheet(QStringLiteral(
        "font-size:16px;font-weight:bold;color:#FFD700;"
        "background:#1a1a00;border:2px solid #806000;border-radius:6px;padding:2px 8px;"));
    bonusCol->addWidget(bonusLbl, 0, Qt::AlignCenter);
    bonusCol->addWidget(m_creditsLabel);
    infoRow->addLayout(bonusCol);

    infoRow->addStretch();
    auto makeSideBtn = [this](const QString& t) {
        auto* b = new QPushButton(t, this);
        b->setFixedSize(34, 34);
        b->setStyleSheet(QStringLiteral(
            "QPushButton{background:#1e1e3a;color:white;font-size:17px;"
            "border-radius:4px;border:1px solid #555;}"
            "QPushButton:hover{background:#2a2a5a;}"
            "QPushButton:pressed{background:#0f0f2a;}"));
        return b;
    };
    m_betMinus = makeSideBtn(QStringLiteral("−"));
    m_betLabel = new QLabel(QStringLiteral("Bet: 1"), this);
    m_betLabel->setStyleSheet(QStringLiteral(
        "font-size:15px;font-weight:bold;color:#2196F3;padding:0 8px;"));
    m_betPlus = makeSideBtn(QStringLiteral("+"));
    infoRow->addWidget(m_betMinus);
    infoRow->addWidget(m_betLabel);
    infoRow->addWidget(m_betPlus);
    root->addLayout(infoRow);

    // SPIN
    m_spinBtn = new QPushButton(QStringLiteral("🎰  SPIN!"), this);
    m_spinBtn->setFixedHeight(54);
    m_spinBtn->setStyleSheet(QStringLiteral(
        "QPushButton{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,"
        "  stop:0 #FF4500,stop:1 #CC2200);color:white;font-size:20px;"
        "  font-weight:bold;border-radius:9px;border:2px solid #FF6347;}"
        "QPushButton:hover{background:#FF4500;}"
        "QPushButton:pressed{background:#CC2200;}"
        "QPushButton:disabled{background:#555;color:#888;border-color:#444;}"));
    root->addWidget(m_spinBtn);

    // COLLECT
    m_collectBtn = new QPushButton(QStringLiteral("✅  COLLECT"), this);
    m_collectBtn->setFixedHeight(46);
    m_collectBtn->hide();
    m_collectBtn->setStyleSheet(QStringLiteral(
        "QPushButton{background:#1a4a1a;color:#aaffaa;font-size:16px;"
        "  font-weight:bold;border-radius:8px;border:2px solid #44aa44;}"
        "QPushButton:hover{background:#2a6a2a;}"
        "QPushButton:disabled{background:#333;color:#666;border-color:#444;}"));
    root->addWidget(m_collectBtn);

    // DOUBLE UP
    m_doubleUpBtn = new QPushButton(QStringLiteral("🎲  DOUBLE UP?"), this);
    m_doubleUpBtn->setFixedHeight(42);
    m_doubleUpBtn->hide();
    m_doubleUpBtn->setStyleSheet(QStringLiteral(
        "QPushButton{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,"
        "  stop:0 #9400D3,stop:1 #6600AA);color:white;font-size:15px;"
        "  font-weight:bold;border-radius:8px;border:2px solid #CC44FF;}"
        "QPushButton:hover{background:#AA00EE;}"));
    root->addWidget(m_doubleUpBtn);

    m_doubleUp   = new DoubleUpWidget(m_sound, this);
    m_countTimer = new QTimer(this);
    m_countTimer->setInterval(25);

    connect(m_spinBtn,     &QPushButton::clicked, this, &UpperScreen::onSpinClicked);
    connect(m_betPlus,     &QPushButton::clicked, this, &UpperScreen::onBetPlus);
    connect(m_betMinus,    &QPushButton::clicked, this, &UpperScreen::onBetMinus);
    connect(m_backBtn,     &QPushButton::clicked, this, &UpperScreen::backRequested);
    connect(m_collectBtn,  &QPushButton::clicked, this, [this]() {
        startCollectAnimation(m_machine->pendingWin());
    });
    connect(m_doubleUpBtn, &QPushButton::clicked, this, [this]() {
        m_collectBtn->setEnabled(false);
        m_doubleUpBtn->setEnabled(false);
        m_doubleUp->startGamble(m_machine->pendingWin());
    });
    connect(m_countTimer, &QTimer::timeout, this, &UpperScreen::onCountTick);

    connect(m_doubleUp, &DoubleUpWidget::collected, this, [this](int total) {
        m_collectBtn->hide();
        m_doubleUpBtn->hide();
        startCollectAnimation(total);
    });
    connect(m_doubleUp, &DoubleUpWidget::lostAll, this, [this]() {
        m_winLabel->setText(QStringLiteral(""));
        m_statusLabel->setText(QStringLiteral("No win — try again!"));
        m_collectBtn->hide();
        m_doubleUpBtn->hide();
        finishRound();
    });

    syncLabels();
}

void UpperScreen::onSpinClicked() {
    // canSpin() returns true for free spins even with 0 credits
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

void UpperScreen::stopReelsSequentially() {
    QTimer::singleShot(400, this, [this]() { stopNextReel(0); });
}

void UpperScreen::stopNextReel(int idx) {
    int n = static_cast<int>(m_reelWidgets.size());
    if (idx >= n) {
        QTimer::singleShot(200, this, [this]() {
            m_sound->stopSpin();
            WinResult r  = m_machine->lastResult();
            int pending  = m_machine->pendingWin();
            applyWinHighlights();
            if (r.isWin && pending > 0) {
                int nl = static_cast<int>(r.paylineWins.size());
                m_winLabel->setText(nl > 1
                    ? QStringLiteral("🏆🏆 %1 LINES  WIN:  %2  credits").arg(nl).arg(pending)
                    : QStringLiteral("WIN:  %1  credits").arg(pending));
                m_statusLabel->setText(r.description);
                m_collectBtn->setEnabled(true);  m_collectBtn->show();
                m_doubleUpBtn->setEnabled(true); m_doubleUpBtn->show();
                if (pending >= 2000)     m_sound->play(SoundEngine::Sound::Jackpot);
                else if (pending >= 200) m_sound->play(SoundEngine::Sound::BigWin);
                else                     m_sound->play(SoundEngine::Sound::Win);
            } else {
                m_statusLabel->setText(QStringLiteral("No win — try again!"));
                finishRound();
            }
        });
        return;
    }

    auto col = m_machine->reelColumn(idx);
    m_reelWidgets[idx]->stopSpin(col);

    connect(m_reelWidgets[idx], &ReelWidget::stopped,
            this, [this, idx]() {
                disconnect(m_reelWidgets[idx], &ReelWidget::stopped, this, nullptr);
                // Sound plays exactly when reel visually locks
                m_sound->play(SoundEngine::Sound::ReelStop);
                stopNextReel(idx + 1);
            });
}

void UpperScreen::startCollectAnimation(int amount) {
    m_countRemaining = amount;
    m_collectBtn->setEnabled(false);
    m_doubleUpBtn->hide();
    if (m_sound) m_sound->startCoins();
    m_countTimer->start();
}

void UpperScreen::onCountTick() {
    int step = qMax(1, m_countRemaining / 25);
    step = qMin(step, m_countRemaining);
    m_countRemaining -= step;
    m_machine->collectPendingWin(step);

    if (m_countRemaining > 0) {
        m_winLabel->setText(QStringLiteral("WIN:  %1  credits").arg(m_countRemaining));
    } else {
        m_countTimer->stop();
        if (m_sound) m_sound->stopCoins();
        m_winLabel->setText(QStringLiteral(""));
        m_collectBtn->hide();
        finishRound();  // will check isInFreeSpins inside
    }
}

void UpperScreen::finishRound() {
    syncLabels();
    if (m_machine->gameState()->isInFreeSpins()) {
        m_spinBtn->setEnabled(true);   // more free spins remain
    } else {
        // levelChanged signal already fired → timer running
        // Show message and lock spin button
        int bonus = m_machine->gameState()->credits();
        m_winLabel->setText(
            QStringLiteral("🏆 BONUS WON: %1 credits!").arg(bonus));
        m_statusLabel->setText(QStringLiteral("Returning to main game…"));
        m_spinBtn->setEnabled(false);
    }
}

void UpperScreen::onSpinFinished(bool, int, const QString&) { /* deferred */ }

void UpperScreen::onCreditsChanged(int c) {
    m_creditsLabel->setText(QStringLiteral("%1").arg(c));
}

void UpperScreen::onBetPlus() {
    m_machine->gameState()->setBet(m_machine->gameState()->bet() + 1);
    syncLabels();
}

void UpperScreen::onBetMinus() {
    m_machine->gameState()->setBet(m_machine->gameState()->bet() - 1);
    syncLabels();
}

void UpperScreen::syncLabels() {
    GameState* gs = m_machine->gameState();
    // Bonus credits (accumulate during free spins)
    m_creditsLabel->setText(QStringLiteral("%1").arg(gs->credits()));
    // Main credits (lower screen — read-only display)
    if (m_mainCreditsLabel && m_mainCredits)
        m_mainCreditsLabel->setText(QStringLiteral("%1").arg(m_mainCredits->credits()));
    m_betLabel->setText(QStringLiteral("Bet: %1").arg(gs->bet()));
    int fs = gs->freeSpinsLeft();
    if (m_statusLabel && fs > 0)
        m_statusLabel->setText(QStringLiteral("🎯 FREE SPINS: %1 remaining").arg(fs));
}

void UpperScreen::applyWinHighlights() {
    for (auto* rw : m_reelWidgets) rw->clearWinHighlight();
    WinResult r = m_machine->lastResult();
    if (!r.isWin || r.paylineWins.empty()) return;
    for (int ri = 0; ri < (int)m_reelWidgets.size(); ++ri) {
        std::vector<int> rows;
        for (const auto& pw : r.paylineWins)
            if (ri < pw.winCount) rows.push_back(pw.row);
        if (!rows.empty())
            m_reelWidgets[ri]->setWinHighlights(rows);
    }
}

void UpperScreen::refreshDisplay() {
    syncLabels();
    if (m_doubleUp) m_doubleUp->resize(size());
}
