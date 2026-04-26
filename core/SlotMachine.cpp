#include "SlotMachine.h"
#include <cstdlib>
#include <ctime>

SlotMachine::SlotMachine(int numReels, QObject* parent)
    : QObject(parent)
    , m_payTable(std::make_unique<PayTable>())
    , m_gameState(std::make_unique<GameState>(this))
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    rebuildReels(numReels);
}

void SlotMachine::rebuildReels(int count) {
    m_reels.clear();
    m_reels.reserve(count);
    // Advanced mode (5 reels) has no Bonus scatter symbols
    bool includeBonus = (count <= 3);
    for (int i = 0; i < count; ++i)
        m_reels.push_back(std::make_unique<Reel>(this, includeBonus));
}

int SlotMachine::numReels() const { return static_cast<int>(m_reels.size()); }

bool SlotMachine::spin() {
    if (m_gameState->isInFreeSpins()) {
        // Free spin bonus round — no credit deduction
        m_gameState->consumeFreeSpin();
    } else {
        // Normal spin — deduct bet
        if (!m_gameState->canSpin())  return false;
        if (!m_gameState->placeBet()) return false;
    }

    emit spinStarted();

    for (auto& reel : m_reels)
        reel->spin();

    m_lastResult = evaluateResult();

    // Check scatter bonus: 3+ Bonus symbols anywhere in the visible grid
    {
        int bonusCount = 0;
        for (int ri = 0; ri < numReels(); ++ri)
            for (int row = -1; row <= 1; ++row)
                if (symbolAt(ri, row).type() == SymbolType::Bonus) ++bonusCount;
        // Only set the flag — screen calls triggerBonusRound() after animation
        m_bonusTriggered = (bonusCount >= 3);
    }

    // Store pending win — credits NOT updated yet.
    // The screen calls collectPendingWin() after the player presses Collect.
    m_pendingWin = m_lastResult.isWin
                   ? m_lastResult.multiplier * m_gameState->bet()
                   : 0;

    emit spinFinished(m_lastResult.isWin, m_pendingWin, m_lastResult.description);
    return true;
}

void SlotMachine::collectPendingWin(int amount) {
    // Called in small increments by the counting animation.
    if (amount > 0) {
        int cur = m_gameState->credits();
        m_gameState->setCredits(cur + amount);
    }
}

WinResult   SlotMachine::lastResult()  const { return m_lastResult;  }
int         SlotMachine::pendingWin()  const { return m_pendingWin;  }
GameState*  SlotMachine::gameState()   const { return m_gameState.get(); }

Symbol SlotMachine::symbolAt(int reelIndex, int rowOffset) const {
    if (reelIndex < 0 || reelIndex >= numReels())
        return Symbol(SymbolType::Cherry);
    return m_reels[reelIndex]->symbolAt(rowOffset);
}

std::vector<Symbol> SlotMachine::reelColumn(int reelIndex) const {
    std::vector<Symbol> col;
    for (int row = -1; row <= 1; ++row)
        col.push_back(symbolAt(reelIndex, row));
    return col;
}

WinResult SlotMachine::evaluateResult() const {
    if (m_gameState->level() == GameLevel::Easy) {
        std::vector<Symbol> line;
        line.reserve(m_reels.size());
        for (const auto& r : m_reels)
            line.push_back(r->currentSymbol());
        return m_payTable->evaluate3Reels(line);
    } else {
        std::vector<std::vector<Symbol>> grid;
        grid.reserve(m_reels.size());
        for (int i = 0; i < numReels(); ++i)
            grid.push_back(reelColumn(i));
        return m_payTable->evaluate5Reels(grid);
    }
}

bool SlotMachine::bonusTriggered() const { return m_bonusTriggered; }
