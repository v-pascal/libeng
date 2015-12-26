#ifndef LIBENG_INTRO_H_
#define LIBENG_INTRO_H_
#if defined(__cplusplus)

#include <libeng/Graphic/Object/2D/Element2D.h>
#include <libeng/Log/Log.h>

#define LIBENG_WAITCONN_DELAY       20 // In update count (each about 16 ms)

namespace eng {

class WaitConn {

private:
    bool mWifi; // TRUE: Wi-Fi; FALSE: Bluetooth

    Static2D mWarning;
    Element2D mConnL; // Wifi (Left part) / Bluetooth
    Element2D mConnR; // Wifi (Right part) / Bluetooth
    Element2D mDog;
#ifdef DEBUG
    const unsigned int* mLog;
#endif
    const Screen* mScreen;

    unsigned char mMainStep;
    unsigned char mStoryStep;
    bool mForward;

    Element2D mPanel; // Black panel used to hide 'Conn' sprite when behind the warning panel

    inline void reset() {

        LOGV(LIBENG_LOG_INTRO, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        mConnL.reset();
        mConnR.reset();
        mDog.reset();

        mConnL.setAlpha(0.f);
        mConnR.setAlpha(0.f);
        mDog.setAlpha(0.f);

        mDog.translate(0.f, mWarning.getBottom() - mDog.getBottom() - 0.02f);

        float halfW = (mConnL.getRight() - mConnL.getLeft()) / 2.f;
        mConnL.translate(-halfW, 0.f);
        mConnR.translate(halfW, 0.f);
    };
    void moveDog();
    void exitConn();
    void hideConn();

public:
    WaitConn(bool wifi);
    virtual ~WaitConn();

    void initialize(const Game2D* game);

    bool mConnected; // Internet connexion
    inline bool isWifiConn() const { return mWifi; }

    //////
    void start(const Screen* screen);

    inline void pause() {

        LOGV(LIBENG_LOG_INTRO, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        mWarning.pause();
        mConnL.pause();
        mConnR.pause();
        mDog.pause();
        mPanel.pause();
    }
    void resume();

    void update();
    void render() const;

};

//////
class Intro {

public:
    typedef enum {

        LANG_EN = 0,    // England
        LANG_FR,        // French
        LANG_DE,        // Deutsch
        LANG_ES,        // Spanish
        LANG_IT,        // Italian
        LANG_PT         // Portuguese

    } IntroLang;

private:
    const Game2D* mGame;

    inline void center(float* verCoords) {

        float halfW = verCoords[4] * 0.5f;
        float halfH = verCoords[1] * 0.5f;

        verCoords[0] -= halfW;
        verCoords[1] -= halfH;
        verCoords[2] -= halfW;
        verCoords[3] -= halfH;
        verCoords[4] -= halfW;
        verCoords[5] -= halfH;
        verCoords[6] -= halfW;
        verCoords[7] -= halfH;
        mLogo.setVertices(verCoords);
    }

    Static2D mLogo; // Logo & Presents

    bool mDone;
    float mRatio;

protected:
    IntroLang mLang;
    unsigned char mStep;

    bool mOnline; // Need to be connected to Internet B4 continuing
    bool mBluetooth; // Need to have Bluetooth activated
    WaitConn* mWaitConn;

public:
    Intro();
    Intro(IntroLang lang);
    Intro(IntroLang lang, bool online);
    Intro(IntroLang lang, bool online, bool bluetooth);
    virtual ~Intro();

    inline void initialize(const Game2D* game) { mGame = game; }
    inline bool isFinished() const { return mDone; }

    void start(bool resume); // start/resume

    //////
    inline void pause() {

        LOGV(LIBENG_LOG_INTRO, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        mLogo.pause();

        if (mWaitConn) mWaitConn->pause();
    }

    virtual void update();
    void render() const;

};

} // namespace

#endif // __cplusplus
#endif // LIBENG_INTRO_H_
