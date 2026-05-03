#include <QApplication>
#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QKeyEvent>
#include <QShortcut>

// ── Shared credits dialog ─────────────────────────────────────────────────────
int MainWindow::showCreditsDialog(const QString& title, const QString& msg, int defaultVal) {
    QDialog dlg(this);
    dlg.setWindowTitle(title);
    dlg.setFixedSize(360, 200);
    dlg.setModal(true);
    dlg.setStyleSheet(QStringLiteral(
        "QDialog  { background:#12122a; }"
        "QLabel   { color:#FFD700; font-size:15px; font-weight:bold; }"
        "QLabel#sub{ color:#aaa; font-size:11px; font-weight:normal; }"
        "QSpinBox { background:#1e1e3a; color:white; border:2px solid #FFD700;"
        "           padding:6px; font-size:16px; border-radius:6px; }"
        "QPushButton#ok{ background:#FFD700; color:#12122a; font-weight:bold;"
        "                padding:9px 28px; border-radius:8px; font-size:14px; }"
        "QPushButton#ok:hover{ background:#FFA500; }"
        "QPushButton#quit{ background:#333; color:#aaa; padding:8px 20px;"
        "                  border-radius:8px; font-size:12px; }"
        "QPushButton#quit:hover{ background:#555; }"
    ));

    auto* vlay = new QVBoxLayout(&dlg);
    vlay->setSpacing(12);
    vlay->setContentsMargins(30, 22, 30, 22);

    auto* lbl = new QLabel(msg, &dlg);
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setWordWrap(true);
    vlay->addWidget(lbl);

    auto* spin = new QSpinBox(&dlg);
    spin->setRange(10, 10000);
    spin->setValue(defaultVal);
    spin->setSingleStep(50);
    spin->setAlignment(Qt::AlignCenter);
    vlay->addWidget(spin);

    auto* hlay = new QHBoxLayout();
    auto* okBtn = new QPushButton(QStringLiteral("🎰  LET'S PLAY!"), &dlg);
    okBtn->setObjectName(QStringLiteral("ok"));
    hlay->addWidget(okBtn);

    auto* quitBtn = new QPushButton(QStringLiteral("❌  Quit"), &dlg);
    quitBtn->setObjectName(QStringLiteral("quit"));
    hlay->addWidget(quitBtn);
    vlay->addLayout(hlay);

    QObject::connect(okBtn,   &QPushButton::clicked, &dlg, &QDialog::accept);
    QObject::connect(quitBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted)
        return spin->value();
    return -1;   // user wants to quit
}

// ── Constructor ───────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("Slot Machine | v1.2 | OptimTeam"));
    setFixedSize(800, 800);
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);

    m_sound = std::make_unique<SoundEngine>(this);

    m_easyMachine = std::make_unique<SlotMachine>(3, this);
    m_advMachine  = std::make_unique<SlotMachine>(5, this);
    m_advMachine->gameState()->setLevel(GameLevel::Advanced);

    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Mute button — NoFocus so keyboard always goes to screen widgets
    m_muteBtn = new QPushButton(QStringLiteral("🔊 Sound ON  [S]"), this);
    m_muteBtn->setCheckable(true);
    m_muteBtn->setFocusPolicy(Qt::NoFocus);
    m_muteBtn->setStyleSheet(QStringLiteral(
        "QPushButton { background:#1e1e3a; color:#ccc; padding:3px 10px;"
        "  border-radius:4px; border:1px solid #444; font-size:11px; }"
        "QPushButton:checked { background:#3a1a1a; color:#f88; border-color:#933; }"
        "QPushButton:hover   { background:#2a2a5a; }"
    ));
    connect(m_muteBtn, &QPushButton::toggled, this, [this](bool muted) {
        m_sound->setMuted(muted);
        m_muteBtn->setText(muted ? QStringLiteral("🔇 Muted  [S]")
                                 : QStringLiteral("🔊 Sound ON  [S]"));
    });
    statusBar()->addPermanentWidget(m_muteBtn);
    statusBar()->setStyleSheet(QStringLiteral(
        "QStatusBar { background:#0a0a1f; color:#888; font-size:11px; }"
    ));

    m_stack = new QStackedWidget(central);
    layout->addWidget(m_stack);

    m_lowerScreen = new LowerScreen(m_easyMachine.get(), m_sound.get(), m_stack);
    m_upperScreen = new UpperScreen(m_advMachine.get(), m_sound.get(),
                                    m_easyMachine->gameState(), m_stack);

    m_stack->addWidget(m_lowerScreen);
    m_stack->addWidget(m_upperScreen);
    m_stack->setCurrentIndex(0);

    m_bonusCountTimer = new QTimer(this);
    m_bonusCountTimer->setInterval(25);
    connect(m_bonusCountTimer, &QTimer::timeout, this, [this]() {
        int step = qMax(1, m_bonusCountRemaining / 25);
        step = qMin(step, m_bonusCountRemaining);
        m_bonusCountRemaining -= step;
        m_easyMachine->gameState()->setCredits(
            m_easyMachine->gameState()->credits() + step);
        if (m_bonusCountRemaining <= 0) {
            m_bonusCountTimer->stop();
            m_sound->stopCoins();
            m_lowerScreen->refreshDisplay();
        }
    });

    connect(m_easyMachine->gameState(), &GameState::gameOver,
            this, &MainWindow::onGameOver);
    connect(m_lowerScreen, &LowerScreen::levelUpRequested,
            this, &MainWindow::switchToAdvanced);
    connect(m_upperScreen, &UpperScreen::backRequested,
            this, &MainWindow::switchToEasy);

    // ── Global keyboard shortcut for Sound toggle ────────────────────────
    auto* soundShortcut = new QShortcut(QKeySequence(Qt::Key_S), this);
    connect(soundShortcut, &QShortcut::activated, this, [this]() {
        m_muteBtn->toggle();
    });

    // ── START DIALOG — runs BEFORE window is shown ────────────────────────────
    // This works because exec() pumps events even before show()
    int credits = showCreditsDialog(
        QStringLiteral("Slot Machine v1.2 — OptimTeam"),
        QStringLiteral("🎰  Welcome!\n\nHow many credits to start?"),
        100);
    if (credits < 0) {
        // User pressed Quit
        QTimer::singleShot(0, qApp, &QApplication::quit);
        return;
    }
    m_easyMachine->gameState()->setCredits(credits);
}

// ── Keyboard — S for sound (works globally) ──────────────────────────────────
void MainWindow::keyPressEvent(QKeyEvent* e) {
    if (e->isAutoRepeat()) { QMainWindow::keyPressEvent(e); return; }
    if (e->key() == Qt::Key_S) {
        m_muteBtn->toggle();   // fires toggled signal → updates everything
        return;
    }
    QMainWindow::keyPressEvent(e);
}

// ── Game Over ─────────────────────────────────────────────────────────────────
void MainWindow::onGameOver() {
    QTimer::singleShot(800, this, [this]() {
        int credits = showCreditsDialog(
            QStringLiteral("💀  GAME OVER"),
            QStringLiteral("Out of credits!\n\nPlay again?"),
            100);
        if (credits < 0) {
            close();
            return;
        }
        m_easyMachine->gameState()->reset();
        m_advMachine->gameState()->reset();
        m_advMachine->gameState()->setLevel(GameLevel::Advanced);
        m_easyMachine->gameState()->setCredits(credits);
        m_stack->setCurrentIndex(0);
        m_lowerScreen->refreshDisplay();
    });
}

// ── Screen switching ──────────────────────────────────────────────────────────
void MainWindow::switchToAdvanced() {
    m_advMachine->gameState()->setCredits(0);
    m_advMachine->gameState()->triggerBonusRound();
    m_stack->setCurrentIndex(1);
    m_upperScreen->refreshDisplay();
    m_sound->play(SoundEngine::Sound::LevelUp);
}

void MainWindow::switchToEasy() {
    int bonus = m_advMachine->gameState()->credits();
    m_advMachine->gameState()->setCredits(0);
    m_stack->setCurrentIndex(0);
    if (bonus > 0) {
        m_bonusCountRemaining = bonus;
        m_sound->startCoins();
        m_bonusCountTimer->start();
    } else {
        m_lowerScreen->refreshDisplay();
    }
}

void MainWindow::transferCredits(SlotMachine* from, SlotMachine* to) {
    to->gameState()->setCredits(from->gameState()->credits());
}
