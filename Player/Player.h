#ifndef LIBENG_PLAYER_H_
#define LIBENG_PLAYER_H_
#if defined(__cplusplus)

#include <libeng/Log/Log.h>
#include <libeng/Player/OggSound.h>
#include <boost/thread.hpp>
#include <vector>
#include <queue>

namespace eng {

static unsigned char ALL_TRACK = 0xff; // All tracks index

// Reserved sound index
static unsigned char SOUND_IDX_INVALID = 0xff; // This & Above causes a maximum sound count to 254 (== 0xFF - 1)

// Reserved sound ID
static const unsigned char SOUND_ID_LOGO = 0;

//////
class Player {

private:
    Player();
    virtual ~Player();

    static Player* mThis;

public:
    static Player* getInstance() {
        if (!mThis)
            mThis = new Player;
        return mThis;
    }
    static void freeInstance() {
        if (mThis) {
            delete mThis;
            mThis = NULL;
        }
    }
    static void loadSound(unsigned char id);

    inline unsigned char getIndex(unsigned char id) const {

        for (unsigned char i = 0; i < static_cast<unsigned char>(mSounds.size()); ++i)
            if (mSounds[i]->getId() == id)
                return i;

        return SOUND_IDX_INVALID;
    }

    unsigned char addSound(unsigned char id, size_t len, unsigned char* data, bool queued);
    bool rmvSound(unsigned char track);
    // WARNING: Removing sound will change track order

    // Free all sound buffers by keeping entries ('MemoryStream->buffer' buffers)
    // -> Pause & Resume operation (called in 'Game::pause' method)
    inline void empty() {

        LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        for (std::vector<OggSound*>::iterator iter = mSounds.begin(); iter != mSounds.end(); ++iter)
            (*iter)->empty();
    };
    inline void clean() {

        LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        for (std::vector<OggSound*>::iterator iter = mSounds.begin(); iter != mSounds.end(); ++iter)
            delete (*iter);
        mSounds.clear();
        // Called after 'pause(ALL_TRACK)' & 'free' methods call to finish removing all sound entries
        // -> Used when deleting a level
    };
    inline bool isRunning() const { return (mThread != NULL); }

    //////
    void play(unsigned char track); // track == index
    // WARNING: Do not replay a sound with a repeat option!

    void resume();
    // WARNING: On Android call it each time B4 playing any track so after having load every sounds. Needed for
    //          lock/unlock operation (e.g into 'LEVEL::update' method)

    inline bool getResumeFlag(unsigned char track) const {

        LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - track:%d"), __PRETTY_FUNCTION__, __LINE__, track);
        assert(track < static_cast<unsigned char>(mSounds.size()));
        return mSounds[track]->getResumeFlag();
    };
    // Method above is needed when a sound track is loaded and a pause operation occured during playback. If this sound
    // track is not loaded again during the resume operation use this method to check its status in order to avoid to
    // restart playing this empty track when resumed (e.g stop it into 'LEVEL::pause' method).

    void pause(unsigned char track);
    // WARNING: With 'ALL_TRACK' it will pause tracks & stop the thread (call 'resume' method to resume tracks playback)

    void stop(unsigned char track);
    // WARNING: Do not stop a sound with a repeat option orelse repeat option will be disabled!

    void free();

    void setVolume(unsigned char track, float volume);
    inline float getVolume(unsigned char track) const {

        LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - track:%d"), __PRETTY_FUNCTION__, __LINE__, track);
        assert(track < static_cast<unsigned char>(mSounds.size()));
        return mSounds[track]->getVolume();
    }
    inline short getStatus(unsigned char track) const {

#ifdef DEBUG
        if (track >= static_cast<unsigned char>(mSounds.size())) {
            LOGE(LOG_FORMAT(" - Wrong track index: %d (s:%d)"), __PRETTY_FUNCTION__, __LINE__, track,
                    static_cast<unsigned char>(mSounds.size()));
            assert(NULL);
        }
#endif
        return mSounds[track]->getStatus();
    }

    void setRepeat(unsigned char track);

private:
    std::vector<OggSound*> mSounds;
#ifdef DEBUG
    unsigned int mLogIdle;
#endif
    typedef enum {

        EVT_PLAY = 0,	// Let's playing sounds (if needed)
        EVT_PAUSE,      // Pause/Resume operation
        EVT_EXIT        // Exit loop

    } PlayerEvent;

    PlayerEvent mEvent;
    boost::mutex mMutex;

    void stopAll();

    bool initOpenAL1x();
    void destroyOpenAL1x();

    static bool checkError(const char* file, int line);

    boost::thread* mThread;
    void playThreadRunning();
    static void startPlayerThread(Player* player);
};

} // namespace

#endif // __cplusplus
#endif // LIBENG_PLAYER_H_
