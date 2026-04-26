#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QTimer>
#include <memory>
#include "../core/SlotMachine.h"
#include "../core/SoundEngine.h"
#include "LowerScreen.h"
#include "UpperScreen.h"

// ─── MainWindow ───────────────────────────────────────────────────────────────
// Hosts a QStackedWidget with two pages:
//   index 0 → LowerScreen  (Easy,     3-reel machine)
//   index 1 → UpperScreen  (Advanced, 5-reel machine)
//
// Credits are transferred between machines on screen switches so the player's
// wallet is shared across both modes.
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void switchToAdvanced();
    void switchToEasy();

private:
    // Two independent machines (different reel counts / pay-tables)
    std::unique_ptr<SlotMachine> m_easyMachine;
    std::unique_ptr<SlotMachine> m_advMachine;
    std::unique_ptr<SoundEngine> m_sound;

    QStackedWidget* m_stack        { nullptr };
    LowerScreen*    m_lowerScreen  { nullptr };
    UpperScreen*    m_upperScreen  { nullptr };
    QPushButton*    m_muteBtn      { nullptr };

    void transferCredits(SlotMachine* from, SlotMachine* to);

    // Bonus count-up animation (runs after returning to Easy)
    QTimer* m_bonusCountTimer    { nullptr };
    int     m_bonusCountRemaining{ 0 };
};
