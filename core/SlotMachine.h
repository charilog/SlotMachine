#pragma once
#include "Reel.h"
#include "PayTable.h"
#include "GameState.h"
#include <QObject>
#include <vector>
#include <memory>

class SlotMachine : public QObject {
    Q_OBJECT
public:
    explicit SlotMachine(int numReels, QObject* parent = nullptr);

    void rebuildReels(int count);
    int  numReels() const;

    // Spins reels + evaluates result.
    // Credits are NOT updated here — call collectPendingWin() when the player collects.
    bool spin();

    // Transfers the pending win into credits (call after collect animation).
    void collectPendingWin(int amount);

    Symbol      symbolAt(int reelIndex, int rowOffset = 0) const;
    WinResult   lastResult()  const;
    int         pendingWin()  const;
    bool        bonusTriggered() const;  // true if last spin hit 3 scatters   // win amount waiting to be collected
    GameState*  gameState()   const;
    std::vector<Symbol> reelColumn(int reelIndex) const;

signals:
    void spinStarted();
    void spinFinished(bool isWin, int winAmount, const QString& description);

private:
    std::vector<std::unique_ptr<Reel>> m_reels;
    std::unique_ptr<PayTable>          m_payTable;
    std::unique_ptr<GameState>         m_gameState;
    WinResult                          m_lastResult;
    int                                m_pendingWin { 0 };
    bool                               m_bonusTriggered { false };

    WinResult evaluateResult() const;
};
