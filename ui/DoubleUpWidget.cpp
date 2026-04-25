#include "DoubleUpWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QKeyEvent>
#include <cstdlib>

const DoubleUpWidget::CardFace DoubleUpWidget::FACES[] = {
    { true,  QStringLiteral("♥") },
    { false, QStringLiteral("♠") },
    { true,  QStringLiteral("♦") },
    { false, QStringLiteral("♣") },
};

DoubleUpWidget::DoubleUpWidget(SoundEngine* sound, QWidget* parent)
    : QWidget(parent), m_sound(sound)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::StrongFocus);

    m_spinTimer = new QTimer(this);
    m_spinTimer->setInterval(350);
    connect(m_spinTimer, &QTimer::timeout, this, &DoubleUpWidget::onSpinTimer);

    m_flashTimer = new QTimer(this);
    m_flashTimer->setInterval(120);
    connect(m_flashTimer, &QTimer::timeout, this, [this]() {
        m_flashOn = !m_flashOn;
        if (++m_flashFrames > 12) m_flashTimer->stop();
        update();
    });

    buildUI();
    hide();
}

void DoubleUpWidget::buildUI() {
    m_titleLabel = new QLabel(QStringLiteral("🎲  DOUBLE UP  🎲"), this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet(QStringLiteral(
        "font-size:22px; font-weight:bold; color:#FFD700; background:transparent;"));

    m_amountLabel = new QLabel(QStringLiteral(""), this);
    m_amountLabel->setAlignment(Qt::AlignCenter);
    m_amountLabel->setStyleSheet(QStringLiteral(
        "font-size:15px; color:#aaffaa; background:transparent; font-weight:bold;"));

    m_resultLabel = new QLabel(QStringLiteral(""), this);
    m_resultLabel->setAlignment(Qt::AlignCenter);
    m_resultLabel->setStyleSheet(QStringLiteral(
        "font-size:19px; font-weight:bold; background:transparent;"));

    auto makeBtn = [this](const QString& txt, const QString& style) {
        auto* b = new QPushButton(txt, this);
        b->setStyleSheet(style);
        b->setFixedHeight(48);
        b->hide();
        return b;
    };

    m_stopBtn = makeBtn(QStringLiteral("STOP  (SPACE)"),
        QStringLiteral("QPushButton{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,"
            "stop:0 #FFD700,stop:1 #FFA000);color:#111;font-size:17px;font-weight:bold;"
            "border-radius:8px;border:2px solid #FF8C00;}"
            "QPushButton:hover{background:#FFD700;}"));

    m_collectBtn = makeBtn(QStringLiteral("✅  COLLECT"),
        QStringLiteral("QPushButton{background:#1a4a1a;color:#aaffaa;font-size:15px;"
            "font-weight:bold;border-radius:8px;border:2px solid #44aa44;}"
            "QPushButton:hover{background:#2a6a2a;}"));

    m_doubleAgainBtn = makeBtn(QStringLiteral("🎲  DOUBLE AGAIN!"),
        QStringLiteral("QPushButton{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,"
            "stop:0 #FF4500,stop:1 #CC2200);color:white;font-size:15px;"
            "font-weight:bold;border-radius:8px;border:2px solid #FF6347;}"
            "QPushButton:hover{background:#FF4500;}"));

    connect(m_stopBtn,        &QPushButton::clicked, this, &DoubleUpWidget::onStopPressed);
    connect(m_collectBtn,     &QPushButton::clicked, this, &DoubleUpWidget::onCollectPressed);
    connect(m_doubleAgainBtn, &QPushButton::clicked, this, &DoubleUpWidget::onDoubleAgainPressed);
}

void DoubleUpWidget::startGamble(int amount) {
    m_amount     = amount;
    m_state      = State::Spinning;
    m_faceIdx    = 0;
    m_flashOn    = false;
    m_flashFrames= 0;

    m_resultLabel->setText(QStringLiteral(""));
    m_stopBtn->show();
    m_collectBtn->hide();
    m_doubleAgainBtn->hide();
    updateAmountLabel();
    layoutWidgets();

    show();
    raise();
    setFocus();
    m_spinTimer->start();
    if (m_sound) m_sound->startSpin();
}

void DoubleUpWidget::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Space) {
        if (m_state == State::Spinning) onStopPressed();
        else if (m_state == State::Won)  onDoubleAgainPressed();
    }
    QWidget::keyPressEvent(e);
}

void DoubleUpWidget::showEvent(QShowEvent* e) {
    if (parentWidget()) resize(parentWidget()->size());
    QWidget::showEvent(e);
}

void DoubleUpWidget::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    layoutWidgets();
}

void DoubleUpWidget::layoutWidgets() {
    int W = width(), H = height();
    if (W == 0 || H == 0) return;

    int cardSize = qMin(W, H) * 42 / 100;
    int cx = (W - cardSize) / 2;
    int cardTop = H / 2 - cardSize / 2 - 20;
    m_cardRect = QRect(cx, cardTop, cardSize, cardSize);

    int yBelow = m_cardRect.bottom() + 14;
    int bW = W * 7 / 10;
    int bX = (W - bW) / 2;

    m_titleLabel ->setGeometry(0, 18, W, 36);
    m_amountLabel->setGeometry(0, 58, W, 28);
    m_resultLabel->setGeometry(0, yBelow, W, 36);

    int btnY = yBelow + 42;
    m_stopBtn       ->setGeometry(bX, btnY,      bW, 48);
    m_collectBtn    ->setGeometry(bX, btnY,      bW, 48);
    btnY += 56;
    m_doubleAgainBtn->setGeometry(bX, btnY, bW, 48);
}

void DoubleUpWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor(0, 0, 0, 210));

    QRect panel = m_cardRect.adjusted(-24, -24, 24, 24);
    p.fillRect(panel, QColor(20, 20, 50, 230));
    p.setPen(QPen(QColor(80, 60, 20), 2));
    p.drawRoundedRect(panel, 12, 12);

    if (!m_cardRect.isValid()) return;

    const CardFace& face = FACES[m_faceIdx % NUM_FACES];
    QColor bg   = face.red ? QColor(180, 10, 10) : QColor(15, 15, 15);
    QColor glow = face.red ? QColor(255, 80, 80)  : QColor(80, 80, 255);

    if (m_state == State::Won) {
        bg   = m_flashOn ? QColor(0, 160, 0) : QColor(0, 100, 0);
        glow = QColor(0, 255, 0);
    } else if (m_state == State::Lost) {
        bg   = m_flashOn ? QColor(160, 0, 0) : QColor(80, 0, 0);
        glow = QColor(255, 0, 0);
    }

    for (int g = 4; g >= 1; --g) {
        p.setPen(QPen(QColor(glow.red(), glow.green(), glow.blue(), 40*g), g*2));
        p.drawRoundedRect(m_cardRect.adjusted(-g*3,-g*3,g*3,g*3), 16, 16);
    }
    QPainterPath cp; cp.addRoundedRect(m_cardRect, 14, 14);
    p.fillPath(cp, bg);
    p.setPen(QPen(glow, 3));
    p.drawRoundedRect(m_cardRect, 14, 14);

    QFont f(QStringLiteral("Segoe UI Emoji"), m_cardRect.height() * 45 / 100, QFont::Bold);
    p.setFont(f);
    QColor symColor = face.red ? QColor(255,200,200) : QColor(180,180,255);
    if (m_state == State::Won)  symColor = QColor(100,255,100);
    if (m_state == State::Lost) symColor = QColor(255,100,100);
    p.setPen(symColor);
    p.drawText(m_cardRect, Qt::AlignCenter, face.suit);

    QFont small(QStringLiteral("Arial"), 14, QFont::Bold);
    p.setFont(small);
    p.setPen(symColor);
    p.drawText(m_cardRect.adjusted(8,4,0,0),   Qt::AlignTop|Qt::AlignLeft,    face.suit);
    p.drawText(m_cardRect.adjusted(0,0,-8,-4),  Qt::AlignBottom|Qt::AlignRight, face.suit);

    QString label = face.red ? QStringLiteral("RED") : QStringLiteral("BLACK");
    QFont lf(QStringLiteral("Arial"), 11, QFont::Bold);
    p.setFont(lf);
    p.setPen(symColor);
    p.drawText(m_cardRect.adjusted(0, m_cardRect.height()*3/4, 0, 0),
               Qt::AlignCenter, label);
}

void DoubleUpWidget::onSpinTimer() {
    m_faceIdx = (m_faceIdx + 1) % NUM_FACES;
    update();
}

void DoubleUpWidget::onStopPressed() {
    if (m_state != State::Spinning) return;
    m_spinTimer->stop();
    if (m_sound) { m_sound->stopSpin(); m_sound->play(SoundEngine::Sound::ReelStop); }
    showResult(std::rand() % 2 == 0);
}

void DoubleUpWidget::showResult(bool won) {
    m_state = won ? State::Won : State::Lost;
    m_faceIdx = won ? (std::rand()%2==0 ? 0 : 2) : (std::rand()%2==0 ? 1 : 3);

    m_flashFrames = 0;
    m_flashOn = true;
    m_flashTimer->start();

    if (won) {
        m_amount *= 2;
        m_resultLabel->setStyleSheet(QStringLiteral(
            "font-size:19px;font-weight:bold;color:#00ff88;background:transparent;"));
        m_resultLabel->setText(QStringLiteral("🎉 WIN! → %1 credits").arg(m_amount));
        m_stopBtn->hide();
        m_collectBtn->show();
        m_doubleAgainBtn->show();
        if (m_sound) {
            if (m_amount >= 200) m_sound->play(SoundEngine::Sound::BigWin);
            else                 m_sound->play(SoundEngine::Sound::Win);
        }
    } else {
        m_resultLabel->setStyleSheet(QStringLiteral(
            "font-size:19px;font-weight:bold;color:#ff4444;background:transparent;"));
        m_resultLabel->setText(QStringLiteral("💀 LOST!"));
        m_stopBtn->hide();
        m_doubleAgainBtn->hide();
        m_collectBtn->hide();
        if (m_sound) m_sound->play(SoundEngine::Sound::GambleLose);
        QTimer::singleShot(2200, this, [this]() {
            hide();
            emit lostAll();
        });
    }
    updateAmountLabel();
    update();
}

void DoubleUpWidget::onCollectPressed() {
    m_spinTimer->stop();
    if (m_sound) m_sound->stopSpin();
    hide();
    emit collected(m_amount);   // emit FULL amount for counting animation
}

void DoubleUpWidget::onDoubleAgainPressed() {
    if (m_state != State::Won) return;
    m_state = State::Spinning;
    m_flashTimer->stop();
    m_flashOn = false;
    m_resultLabel->setText(QStringLiteral(""));
    m_stopBtn->show();
    m_collectBtn->hide();
    m_doubleAgainBtn->hide();
    updateAmountLabel();
    update();
    m_spinTimer->start();
    if (m_sound) m_sound->startSpin();
    setFocus();
}

void DoubleUpWidget::updateAmountLabel() {
    if (m_state == State::Spinning)
        m_amountLabel->setText(QStringLiteral("Gamble: %1  →  Win: %2")
            .arg(m_amount).arg(m_amount * 2));
    else if (m_state == State::Won)
        m_amountLabel->setText(QStringLiteral("Won: %1 credits").arg(m_amount));
    else
        m_amountLabel->setText(QStringLiteral(""));
}
