#pragma once
#include <QObject>

enum class GameLevel { Easy, Advanced };

class GameState : public QObject {
    Q_OBJECT
public:
    static constexpr int STARTING_CREDITS = 100;
    static constexpr int MIN_BET          = 1;
    static constexpr int MAX_BET          = 20;
    static constexpr int FREE_SPINS_AWARD = 15;

    explicit GameState(QObject* parent = nullptr);

    // ── Accessors ─────────────────────────────────────────────────────────────
    int       credits()       const;
    int       bet()           const;
    int       lastWin()       const;
    GameLevel level()         const;
    int       totalSpins()    const;
    int       totalWins()     const;
    int       freeSpinsLeft() const;
    bool      isInFreeSpins() const;
    bool      canSpin()       const;

    // ── Mutators ──────────────────────────────────────────────────────────────
    void setCredits(int credits);
    void setBet(int bet);
    void setLevel(GameLevel level);

    // ── Game actions ──────────────────────────────────────────────────────────
    bool placeBet();
    void addWin(int amount);
    void reset();
    void resetProgress();
    void triggerBonusRound();
    void consumeFreeSpin();
    void checkLevelUp();   // legacy — checks nothing now, kept for compatibility

signals:
    void creditsChanged(int newCredits);
    void betChanged(int newBet);
    void levelChanged(GameLevel newLevel);
    void gameOver();

private:
    int       m_credits    { STARTING_CREDITS };
    int       m_bet        { MIN_BET };
    int       m_lastWin    { 0 };
    GameLevel m_level      { GameLevel::Easy };
    int       m_totalSpins { 0 };
    int       m_totalWins  { 0 };
    int       m_freeSpins  { 0 };
};
