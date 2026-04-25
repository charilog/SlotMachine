#pragma once
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <vector>
#include "../core/SlotMachine.h"
#include "../core/SoundEngine.h"
#include "ReelWidget.h"
#include "DoubleUpWidget.h"

class UpperScreen : public QWidget {
    Q_OBJECT
public:
    explicit UpperScreen(SlotMachine* machine, SoundEngine* sound, QWidget* parent = nullptr);
    void refreshDisplay();

signals:
    void backRequested();

private slots:
    void onSpinClicked();
    void onBetPlus();
    void onBetMinus();
    void onSpinFinished(bool isWin, int winAmount, const QString& desc);
    void onCreditsChanged(int credits);
    void onCountTick();

private:
    SlotMachine* m_machine;
    SoundEngine* m_sound;

    std::vector<ReelWidget*> m_reelWidgets;

    QPushButton* m_spinBtn     { nullptr };
    QPushButton* m_betPlus     { nullptr };
    QPushButton* m_betMinus    { nullptr };
    QPushButton* m_backBtn     { nullptr };
    QPushButton* m_collectBtn  { nullptr };
    QPushButton* m_doubleUpBtn { nullptr };

    QLabel* m_creditsLabel { nullptr };
    QLabel* m_betLabel     { nullptr };
    QLabel* m_winLabel     { nullptr };
    QLabel* m_statusLabel  { nullptr };

    QTimer* m_countTimer    { nullptr };
    int     m_countRemaining{ 0 };

    DoubleUpWidget* m_doubleUp { nullptr };

    void buildUI();
    void syncLabels();
    void stopReelsSequentially();
    void stopNextReel(int idx);
    void applyWinHighlights();
    void startCollectAnimation(int amount);
    void finishRound();
};
