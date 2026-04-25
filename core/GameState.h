#pragma once
#include <QObject>

// ─── GameLevel ────────────────────────────────────────────────────────────────
enum class GameLevel {
    Easy,       // Lower screen – 3 reels, 1 payline
    Advanced    // Upper screen – 5 reels, 3 paylines
};

// ─── GameState ────────────────────────────────────────────────────────────────
// Owns all mutable game data and emits Qt signals on change.
// No UI logic here — screens observe via signals.
class GameState : public QObject {
    Q_OBJECT
public:
    static constexpr int STARTING_CREDITS  = 100;
    static constexpr int MIN_BET           = 1;
    static constexpr int MAX_BET           = 20;
    static constexpr int LEVEL_UP_SPINS    = 20;   // spins on Easy to unlock Advanced

    explicit GameState(QObject* parent = nullptr);

    // ── Accessors ─────────────────────────────────────────────────────────────
    int        credits()     const;
    int        bet()         const;
    int        lastWin()     const;
    GameLevel  level()       const;
    int        totalSpins()  const;
    int        totalWins()   const;

    // ── Mutators ──────────────────────────────────────────────────────────────
    void setCredits(int credits);
    void setBet(int bet);
    void setLevel(GameLevel level);

    // ── Game actions ──────────────────────────────────────────────────────────
    bool placeBet();            // deducts bet from credits; returns false if insufficient
    void addWin(int amount);    // adds winnings and increments totalWins if amount > 0
    void reset();               // full reset to starting state
    void resetProgress();       // reset level+spins only, keep credits/bet

    bool canSpin() const;       // credits >= bet

    // Checks whether enough spins were played to advance level
    void checkLevelUp();

signals:
    void creditsChanged(int newCredits);
    void betChanged(int newBet);
    void levelChanged(GameLevel newLevel);
    void gameOver();

private:
    int        m_credits    { STARTING_CREDITS };
    int        m_bet        { MIN_BET };
    int        m_lastWin    { 0 };
    GameLevel  m_level      { GameLevel::Easy };
    int        m_totalSpins { 0 };
    int        m_totalWins  { 0 };
};
