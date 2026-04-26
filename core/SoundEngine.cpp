#include "SoundEngine.h"
#include <QMediaDevices>
#include <QAudioDevice>
#include <cmath>
#include <cstdlib>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SoundEngine::SoundEngine(QObject* parent) : QObject(parent) {
    m_format.setSampleRate(SAMPLE_RATE);
    m_format.setChannelCount(1);
    m_format.setSampleFormat(QAudioFormat::Int16);

    generateAll();

    QAudioDevice dev = QMediaDevices::defaultAudioOutput();
    if (dev.isNull()) return;

    // Push mode: sink returns a device we write to.
    // The sink stays active forever — no stop/start, no new threads.
    m_sink   = new QAudioSink(dev, m_format, this);
    m_sink->setVolume(0.85);
    m_device = m_sink->start();   // ← push mode entry point

    m_timer = new QTimer(this);
    m_timer->setInterval(TICK_MS);
    connect(m_timer, &QTimer::timeout, this, &SoundEngine::onAudioTimer);
    m_timer->start();
}

SoundEngine::~SoundEngine() {
    if (m_timer)  m_timer->stop();
    if (m_sink)   m_sink->stop();
}

// ── Public API ────────────────────────────────────────────────────────────────
void SoundEngine::play(Sound s) {
    if (m_muted) return;
    switch (s) {
        case Sound::Click:    playVoice(V_UI,     m_sndClick);    break;
        case Sound::ReelStop: playVoice(V_STOP,   m_sndReelStop); break;
        case Sound::Win:      playVoice(V_RESULT, m_sndWin);      break;
        case Sound::BigWin:   playVoice(V_RESULT, m_sndBigWin);   break;
        case Sound::Jackpot:  playVoice(V_RESULT, m_sndJackpot);  break;
        case Sound::LevelUp:    playVoice(V_UI,     m_sndLevelUp);    break;
        case Sound::GambleLose:    playVoice(V_RESULT, m_sndGambleLose);    break;
        case Sound::DoubleUpTick:  playVoice(V_DOUBLEUP, m_sndDoubleUpTick); break;
    }
}

void SoundEngine::startSpin() {
    if (!m_muted) playVoice(V_SPIN, m_sndSpinLoop, true);
}

void SoundEngine::stopSpin() {
    stopVoice(V_SPIN);
}

void SoundEngine::startCoins() {
    if (!m_muted) playVoice(V_COINS, m_sndCoinLoop, true);
}

void SoundEngine::stopCoins() {
    m_voices.erase(V_COINS);
}

void SoundEngine::startDoubleUp() {
    if (!m_muted) playVoice(V_DOUBLEUP, m_sndDoubleUpTick, true);
}

void SoundEngine::stopDoubleUp() {
    m_voices.erase(V_DOUBLEUP);
}

bool SoundEngine::isMuted() const { return m_muted; }
void SoundEngine::setMuted(bool m) {
    m_muted = m;
    if (m) m_voices.clear();
}

// ── Voice management (main thread only) ──────────────────────────────────────
void SoundEngine::playVoice(int id, const QByteArray& pcm, bool loop) {
    m_voices[id] = { pcm, 0, loop };
}

void SoundEngine::stopVoice(int id) {
    m_voices.erase(id);
}

// ── Audio timer: mix and push PCM to sink ─────────────────────────────────────
void SoundEngine::onAudioTimer() {
    if (!m_device || !m_sink) return;

    // How many bytes the sink can accept right now
    int free = static_cast<int>(m_sink->bytesFree());
    if (free <= 0) return;

    // Align to sample boundary
    free &= ~1;
    int numSamples = free / 2;

    std::vector<float> acc(numSamples, 0.0f);

    // Mix all active voices
    std::vector<int> expired;
    for (auto& [id, voice] : m_voices) {
        const auto* src = reinterpret_cast<const int16_t*>(voice.data.constData());
        int srcLen = static_cast<int>(voice.data.size()) / 2;

        for (int i = 0; i < numSamples; ++i) {
            if (voice.pos >= srcLen) {
                if (voice.loop) voice.pos = 0;
                else { expired.push_back(id); break; }
            }
            acc[i] += static_cast<float>(src[voice.pos++]) / 32767.0f;
        }
    }
    for (int id : expired) m_voices.erase(id);

    // Convert to int16 with soft clip
    QByteArray buf(free, '\0');
    auto* out = reinterpret_cast<int16_t*>(buf.data());
    for (int i = 0; i < numSamples; ++i) {
        float v = std::tanh(acc[i] * 0.85f);
        out[i] = static_cast<int16_t>(v * 32767.0f);
    }

    m_device->write(buf);
}

// ── Sound generation ──────────────────────────────────────────────────────────
void SoundEngine::generateAll() {
    m_sndClick    = makeTone(880, 0.06, 30.0, 0.45);

    m_sndReelStop = mix({
        makeNoise(0.09, 0.50),
        makeTone(160, 0.12, 22.0, 0.45),
        makeTone(85,  0.10, 18.0, 0.30),
    });

    m_sndSpinLoop = makeSpinLoop(0.35);

    m_sndWin = seq({
        makeTone(261.63, 0.11, 9.0, 0.65),
        makeTone(329.63, 0.11, 9.0, 0.65),
        makeTone(392.00, 0.11, 9.0, 0.65),
        makeTone(523.25, 0.24, 5.0, 0.75),
    });

    m_sndBigWin = seq({
        makeTone(261.63, 0.09, 9.0, 0.60), makeTone(329.63, 0.09, 9.0, 0.60),
        makeTone(392.00, 0.09, 9.0, 0.60), makeTone(523.25, 0.09, 9.0, 0.70),
        makeTone(659.25, 0.09, 9.0, 0.70),
        mix({ makeTone(523.25,0.38,4.0,0.38), makeTone(659.25,0.38,4.0,0.38),
              makeTone(783.99,0.38,4.0,0.38) }),
    });

    m_sndJackpot = seq({
        seq({ makeTone(261.63,0.07,14,0.55), makeTone(329.63,0.07,14,0.55),
              makeTone(392.00,0.07,14,0.55), makeTone(523.25,0.07,14,0.55),
              makeTone(659.25,0.07,14,0.55), makeTone(783.99,0.07,14,0.55),
              makeTone(1046.5,0.07,14,0.55) }),
        seq({ makeTone(1046.5,0.055,22,0.42), makeTone(1318.5,0.055,22,0.42),
              makeTone(1046.5,0.055,22,0.42), makeTone(1318.5,0.055,22,0.42),
              makeTone(1046.5,0.055,22,0.42), makeTone(1318.5,0.055,22,0.42),
              makeTone(1046.5,0.055,22,0.42), makeTone(1318.5,0.055,22,0.42) }),
        mix({ makeTone(523.25,0.55,3.2,0.35), makeTone(659.25,0.55,3.2,0.35),
              makeTone(783.99,0.55,3.2,0.35), makeTone(1046.5,0.55,3.2,0.35) }),
    });

    // Gamble lose — deep descending "bwong"
    m_sndGambleLose = seq({
        mix({ makeTone(220.0, 0.08, 6.0, 0.55), makeNoise(0.08, 0.35) }),
        makeTone(196.0, 0.10, 5.0, 0.50),
        makeTone(165.0, 0.12, 4.5, 0.45),
        makeTone(130.0, 0.18, 3.5, 0.40),
        makeTone( 98.0, 0.28, 3.0, 0.30),
    });

    // Double Up loop — fan/turbine flutter:
    // Rapid amplitude-modulated noise burst at blade-pass frequency
    {
        int n = static_cast<int>(0.55 * SAMPLE_RATE);
        QByteArray data(n * 2, '\0');
        auto* s = reinterpret_cast<int16_t*>(data.data());
        const double bladeHz  = 28.0;   // blade-pass frequency (fan speed feel)
        const double spinHz   = 7.0;    // slower spin wobble
        for (int i = 0; i < n; ++i) {
            double t    = static_cast<double>(i) / SAMPLE_RATE;
            // Noise source
            double noise = (static_cast<double>(std::rand()) / RAND_MAX) * 2.0 - 1.0;
            // Amplitude modulated by blade-pass + wobble
            double blade = 0.5 + 0.5 * std::sin(2.0 * M_PI * bladeHz * t);
            double wobble= 0.7 + 0.3 * std::sin(2.0 * M_PI * spinHz  * t);
            // Soft fade in/out at edges
            double edge  = (i < 512) ? i / 512.0 : (i > n - 512) ? (n - i) / 512.0 : 1.0;
            double v     = 0.38 * noise * blade * wobble * edge;
            s[i] = static_cast<int16_t>(std::clamp(v * 32767.0, -32767.0, 32767.0));
        }
        m_sndDoubleUpTick = data;
    }

    // Coin loop — rapid metallic clinks that loop seamlessly
    {
        // One "clink": bright metallic tap + harmonic decay
        auto clink = [this](double baseFreq, double amp) {
            return mix({
                makeTone(baseFreq,        0.04, 50.0, amp * 0.70),
                makeTone(baseFreq * 2.76, 0.03, 60.0, amp * 0.35),  // inharmonic overtone
                makeTone(baseFreq * 5.40, 0.02, 80.0, amp * 0.20),  // metallic shimmer
                makeNoise(0.025, amp * 0.12),
            });
        };
        // Slightly varying pitches for natural feel
        QByteArray silence(static_cast<int>(0.06 * SAMPLE_RATE) * 2, '\0');
        QByteArray silence2(static_cast<int>(0.04 * SAMPLE_RATE) * 2, '\0');
        m_sndCoinLoop = seq({
            clink(1200.0, 0.55),  silence,
            clink(1350.0, 0.50),  silence2,
            clink(1100.0, 0.58),  silence,
            clink(1280.0, 0.48),  silence2,
        });
    }

    m_sndLevelUp = seq({
        makeTone(130.81,0.06,14,0.5), makeTone(261.63,0.06,14,0.5),
        makeTone(392.00,0.06,14,0.5), makeTone(523.25,0.06,14,0.5),
        makeTone(783.99,0.06,14,0.5), makeTone(1046.5,0.06,14,0.5),
        mix({ makeTone(523.25,0.48,3.5,0.35), makeTone(659.25,0.48,3.5,0.35),
              makeTone(1046.5,0.48,3.5,0.35) }),
    });
}

QByteArray SoundEngine::makeTone(double hz, double sec,
                                  double decay, double amp) const {
    int n = static_cast<int>(sec * SAMPLE_RATE);
    QByteArray d(n * 2, '\0');
    auto* s = reinterpret_cast<int16_t*>(d.data());
    for (int i = 0; i < n; ++i) {
        double t = static_cast<double>(i) / SAMPLE_RATE;
        double v = amp * std::exp(-decay * t) * std::sin(2.0 * M_PI * hz * t);
        s[i] = static_cast<int16_t>(std::clamp(v * 32767.0, -32767.0, 32767.0));
    }
    return d;
}

QByteArray SoundEngine::makeNoise(double sec, double amp) const {
    int n = static_cast<int>(sec * SAMPLE_RATE);
    QByteArray d(n * 2, '\0');
    auto* s = reinterpret_cast<int16_t*>(d.data());
    for (int i = 0; i < n; ++i) {
        double t = static_cast<double>(i) / SAMPLE_RATE;
        double v = amp * std::exp(-18.0 * t)
                 * ((static_cast<double>(std::rand()) / RAND_MAX) * 2.0 - 1.0);
        s[i] = static_cast<int16_t>(std::clamp(v * 32767.0, -32767.0, 32767.0));
    }
    return d;
}

QByteArray SoundEngine::makeWah(double f0, double f1, double sec) const {
    int n = static_cast<int>(sec * SAMPLE_RATE);
    QByteArray d(n * 2, '\0');
    auto* s = reinterpret_cast<int16_t*>(d.data());
    for (int i = 0; i < n; ++i) {
        double t    = static_cast<double>(i) / SAMPLE_RATE;
        double frac = t / sec;
        double v    = 0.60*(1.0-frac*0.65)
                    * std::sin(2.0*M_PI*(f0+(f1-f0)*frac)*t);
        s[i] = static_cast<int16_t>(std::clamp(v * 32767.0, -32767.0, 32767.0));
    }
    return d;
}

QByteArray SoundEngine::makeSpinLoop(double sec) const {
    int n = static_cast<int>(sec * SAMPLE_RATE);
    QByteArray d(n * 2, '\0');
    auto* s = reinterpret_cast<int16_t*>(d.data());
    double phase = 0.0;
    for (int i = 0; i < n; ++i) {
        double t    = static_cast<double>(i) / SAMPLE_RATE;
        double freq = 175.0 + 35.0 * std::sin(2.0 * M_PI * (1.0/sec) * t);
        phase      += 2.0 * M_PI * freq / SAMPLE_RATE;
        double v    = 0.28 * (std::sin(phase) + 0.4 * std::sin(phase * 2.0));
        double fade = (i < 64) ? i/64.0 : (i > n-64) ? (n-i)/64.0 : 1.0;
        s[i] = static_cast<int16_t>(std::clamp(v*fade*32767.0,-32767.0,32767.0));
    }
    return d;
}

QByteArray SoundEngine::mix(const std::vector<QByteArray>& parts) const {
    if (parts.empty()) return {};
    int maxLen = 0;
    for (const auto& p : parts) maxLen = std::max(maxLen, (int)p.size());
    std::vector<double> acc(maxLen/2, 0.0);
    for (const auto& p : parts) {
        const auto* src = reinterpret_cast<const int16_t*>(p.constData());
        for (int i = 0; i < (int)p.size()/2; ++i) acc[i] += src[i]/32767.0;
    }
    double peak = 0.0;
    for (double v : acc) peak = std::max(peak, std::abs(v));
    double scale = (peak > 1.0) ? 0.88/peak : 1.0;
    QByteArray r(maxLen, '\0');
    auto* out = reinterpret_cast<int16_t*>(r.data());
    for (int i = 0; i < (int)acc.size(); ++i)
        out[i] = static_cast<int16_t>(
            std::clamp(acc[i]*scale*32767.0, -32767.0, 32767.0));
    return r;
}

QByteArray SoundEngine::seq(const std::vector<QByteArray>& parts) const {
    QByteArray r;
    for (const auto& p : parts) r.append(p);
    return r;
}
