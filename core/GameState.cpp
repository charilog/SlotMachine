#include "GameState.h"
#include <algorithm>

GameState::GameState(QObject* parent) : QObject(parent) {}

// ── Accessors ──────────────────────────────────────────────────────────────────
int       GameState::credits()       const { return m_credits;    }
int       GameState::bet()           const { return m_bet;        }
int       GameState::lastWin()       const { return m_lastWin;    }
GameLevel GameState::level()         const { return m_level;      }
int       GameState::totalSpins()    const { return m_totalSpins; }
int       GameState::totalWins()     const { return m_totalWins;  }
int       GameState::freeSpinsLeft() const { return m_freeSpins;  }
bool      GameState::isInFreeSpins() const { return m_freeSpins > 0; }
bool      GameState::canSpin()       const {
    if (m_freeSpins > 0) return true;  // free spins always allowed
    return m_credits >= m_bet;
}

// ── Mutators ───────────────────────────────────────────────────────────────────
void GameState::setCredits(int credits) {
    m_credits = std::max(0, credits);
    emit creditsChanged(m_credits);
}

void GameState::setBet(int bet) {
    int clamped = std::clamp(bet, MIN_BET, MAX_BET);
    if (clamped != m_bet) {
        m_bet = clamped;
        emit betChanged(m_bet);
    }
}

void GameState::setLevel(GameLevel level) {
    if (level != m_level) {
        m_level = level;
        emit levelChanged(m_level);
    }
}

// ── Game actions ───────────────────────────────────────────────────────────────
bool GameState::placeBet() {
    if (m_credits < m_bet) return false;
    m_credits -= m_bet;
    ++m_totalSpins;
    emit creditsChanged(m_credits);
    return true;
}

void GameState::addWin(int amount) {
    if (amount > 0) {
        m_credits += amount;
        m_lastWin  = amount;
        ++m_totalWins;
        emit creditsChanged(m_credits);
    } else {
        m_lastWin = 0;
        if (m_credits == 0)
            emit gameOver();
    }
}

void GameState::reset() {
    m_credits    = STARTING_CREDITS;
    m_bet        = MIN_BET;
    m_lastWin    = 0;
    m_level      = GameLevel::Easy;
    m_totalSpins = 0;
    m_totalWins  = 0;
    m_freeSpins  = 0;
    emit creditsChanged(m_credits);
    emit betChanged(m_bet);
    emit levelChanged(m_level);
}

void GameState::resetProgress() {
    m_totalSpins = 0;
    m_totalWins  = 0;
    m_lastWin    = 0;
    m_freeSpins  = 0;
    if (m_level != GameLevel::Easy) {
        m_level = GameLevel::Easy;
        emit levelChanged(m_level);
    }
}

void GameState::triggerBonusRound() {
    m_freeSpins = FREE_SPINS_AWARD;
    setLevel(GameLevel::Advanced);
}

void GameState::consumeFreeSpin() {
    if (m_freeSpins > 0)
        --m_freeSpins;
    // NOTE: level stays Advanced until MainWindow::switchToEasy() is called
    // This prevents evaluateResult() from using the wrong 3-reel path
}

// Legacy — bonus trigger is now scatter-based, not spin-count-based
void GameState::checkLevelUp() {}
