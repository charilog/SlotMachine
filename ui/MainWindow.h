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

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

protected:
    void keyPressEvent(QKeyEvent* e) override;

private slots:
    void switchToAdvanced();
    void switchToEasy();
    void onGameOver();

private:
    int  showCreditsDialog(const QString& title, const QString& msg, int defaultVal);

    std::unique_ptr<SlotMachine> m_easyMachine;
    std::unique_ptr<SlotMachine> m_advMachine;
    std::unique_ptr<SoundEngine> m_sound;

    QStackedWidget* m_stack        { nullptr };
    LowerScreen*    m_lowerScreen  { nullptr };
    UpperScreen*    m_upperScreen  { nullptr };
    QPushButton*    m_muteBtn      { nullptr };

    QTimer* m_bonusCountTimer     { nullptr };
    int     m_bonusCountRemaining { 0 };

    void transferCredits(SlotMachine* from, SlotMachine* to);
};
