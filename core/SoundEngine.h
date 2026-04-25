#pragma once
#include <QObject>
#include <QTimer>
#include <QAudioFormat>
#include <QAudioSink>
#include <QIODevice>
#include <QByteArray>
#include <unordered_map>
#include <vector>

// ─── SoundEngine ──────────────────────────────────────────────────────────────
// Uses QAudioSink in PUSH mode:
//   m_sink->start() returns a QIODevice we write into.
//   A QTimer fires every TICK_MS and writes exactly bytesFree() bytes of
//   mixed PCM — no threads created, no stop/start cycles, no custom QIODevice.
//
// All voice management is on the main thread → no mutex needed.
class SoundEngine : public QObject {
    Q_OBJECT
public:
    enum class Sound { Click, ReelStop, Win, BigWin, Jackpot, LevelUp, GambleLose };

    explicit SoundEngine(QObject* parent = nullptr);
    ~SoundEngine() override;

    void play(Sound s);
    void startSpin();
    void stopSpin();
    void startCoins();   // coin count-up animation loop
    void stopCoins();

    bool isMuted() const;
    void setMuted(bool muted);

private slots:
    void onAudioTimer();

private:
    static constexpr int SAMPLE_RATE = 44100;
    static constexpr int TICK_MS     = 15;   // write interval

    // Voice IDs
    static constexpr int V_UI     = 0;
    static constexpr int V_STOP   = 1;
    static constexpr int V_RESULT = 2;
    static constexpr int V_SPIN   = 3;
    static constexpr int V_COINS  = 4;   // coin count-up loop

    struct Voice {
        QByteArray data;
        int        pos  { 0 };
        bool       loop { false };
    };

    QAudioFormat  m_format;
    QAudioSink*   m_sink    { nullptr };
    QIODevice*    m_device  { nullptr };   // returned by m_sink->start()
    QTimer*       m_timer   { nullptr };
    bool          m_muted   { false };

    std::unordered_map<int, Voice> m_voices;   // main thread only

    void playVoice(int id, const QByteArray& pcm, bool loop = false);
    void stopVoice(int id);

    // Sound data (generated once in constructor)
    QByteArray m_sndClick, m_sndReelStop, m_sndSpinLoop;
    QByteArray m_sndWin,   m_sndBigWin,  m_sndJackpot;
    QByteArray m_sndLevelUp;
    QByteArray m_sndCoinLoop;
    QByteArray m_sndGambleLose;

    void generateAll();
    QByteArray makeTone(double hz,  double sec, double decay=6.0, double amp=0.7) const;
    QByteArray makeNoise(double sec, double amp=0.4) const;
    QByteArray makeWah(double f0, double f1, double sec) const;
    QByteArray makeSpinLoop(double sec) const;
    QByteArray mix(const std::vector<QByteArray>& v) const;
    QByteArray seq(const std::vector<QByteArray>& v) const;
};
