#include "GameState.h"
#include <algorithm>

GameState::GameState(QObject* parent)
    : QObject(parent)
{}

// ── Accessors ─────────────────────────────────────────────────────────────────
int        GameState::credits()    const { return m_credits;    }
int        GameState::bet()        const { return m_bet;        }
int        GameState::lastWin()    const { return m_lastWin;    }
GameLevel  GameState::level()      const { return m_level;      }
int        GameState::totalSpins() const { return m_totalSpins; }
int        GameState::totalWins()  const { return m_totalWins;  }

// ── Mutators ──────────────────────────────────────────────────────────────────
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

// ── Game actions ──────────────────────────────────────────────────────────────
bool GameState::placeBet() {
    if (m_credits < m_bet) return false;
    m_credits -= m_bet;
    ++m_totalSpins;
    emit creditsChanged(m_credits);
    return true;
}

void GameState::addWin(int amount) {
    if (amount > 0) {
        m_credits  += amount;
        m_lastWin   = amount;
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
    emit creditsChanged(m_credits);
    emit betChanged(m_bet);
    emit levelChanged(m_level);
}

bool GameState::canSpin() const {
    return m_credits >= m_bet;
}

void GameState::checkLevelUp() {
    if (m_level == GameLevel::Easy && m_totalSpins >= LEVEL_UP_SPINS)
        setLevel(GameLevel::Advanced);
}

void GameState::resetProgress() {
    m_totalSpins = 0;
    m_totalWins  = 0;
    m_lastWin    = 0;
    // Force back to Easy so evaluateResult() uses the 3-reel path
    if (m_level != GameLevel::Easy) {
        m_level = GameLevel::Easy;
        emit levelChanged(m_level);
    }
}
