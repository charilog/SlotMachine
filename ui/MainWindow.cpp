#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("Slot Machine | v1.0 | OptimTeam"));
    setMinimumSize(680, 680);
    resize(740, 740);

    m_sound = std::make_unique<SoundEngine>(this);

    m_easyMachine = std::make_unique<SlotMachine>(3, this);
    m_advMachine  = std::make_unique<SlotMachine>(5, this);
    m_advMachine->gameState()->setLevel(GameLevel::Advanced);

    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_muteBtn = new QPushButton(QStringLiteral("🔊 Sound ON"), this);
    m_muteBtn->setCheckable(true);
    m_muteBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background:#1e1e3a; color:#ccc; padding:3px 10px;"
        "  border-radius:4px; border:1px solid #444; font-size:11px; }"
        "QPushButton:checked { background:#3a1a1a; color:#f88; border-color:#933; }"
        "QPushButton:hover   { background:#2a2a5a; }"
    ));
    connect(m_muteBtn, &QPushButton::toggled, this, [this](bool muted) {
        m_sound->setMuted(muted);
        m_muteBtn->setText(muted ? QStringLiteral("🔇 Muted")
                                 : QStringLiteral("🔊 Sound ON"));
    });
    statusBar()->addPermanentWidget(m_muteBtn);
    statusBar()->setStyleSheet(QStringLiteral(
        "QStatusBar { background:#0a0a1f; color:#888; font-size:11px; }"
    ));

    m_stack = new QStackedWidget(central);
    layout->addWidget(m_stack);

    m_lowerScreen = new LowerScreen(m_easyMachine.get(), m_sound.get(), m_stack);
    m_upperScreen = new UpperScreen(m_advMachine.get(),  m_sound.get(), m_stack);

    m_stack->addWidget(m_lowerScreen);
    m_stack->addWidget(m_upperScreen);
    m_stack->setCurrentIndex(0);

    connect(m_lowerScreen, &LowerScreen::levelUpRequested,
            this, &MainWindow::switchToAdvanced);
    connect(m_upperScreen, &UpperScreen::backRequested,
            this, &MainWindow::switchToEasy);
}

void MainWindow::switchToAdvanced() {
    transferCredits(m_easyMachine.get(), m_advMachine.get());
    m_stack->setCurrentIndex(1);
    m_upperScreen->refreshDisplay();
    setWindowTitle(QStringLiteral("Slot Machine | v1.0 | OptimTeam"));
    m_sound->play(SoundEngine::Sound::LevelUp);
}

void MainWindow::switchToEasy() {
    transferCredits(m_advMachine.get(), m_easyMachine.get());

    // ── BUG FIX ───────────────────────────────────────────────────────────────
    // Reset level back to Easy and clear spin counter so that:
    //  (a) evaluateResult() uses the correct 3-reel path again
    //  (b) the progress label shows "0/20" instead of "65/20"
    m_easyMachine->gameState()->resetProgress();
    // ─────────────────────────────────────────────────────────────────────────

    m_stack->setCurrentIndex(0);
    m_lowerScreen->refreshDisplay();
    setWindowTitle(QStringLiteral("Slot Machine | v1.0 | OptimTeam"));
    m_sound->play(SoundEngine::Sound::Click);
}

void MainWindow::transferCredits(SlotMachine* from, SlotMachine* to) {
    to->gameState()->setCredits(from->gameState()->credits());
}
