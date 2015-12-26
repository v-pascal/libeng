#include "Player.h"

#include <libeng/Tools/Tools.h>
#include <libeng/Global.h>
#include <assert.h>

static const ALint gs_contextAttribs[] = { ALC_FREQUENCY, 22050, 0 };

namespace eng {

Player* Player::mThis = NULL;

//////
Player::Player() : mThread(NULL), mEvent(EVT_PLAY) {

    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#ifdef DEBUG
    mLogIdle = 0;
#endif

    initOpenAL1x();
    // ...no need to call method above into the player thread (like OpenGL-ES)
}
Player::~Player() {

    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    if (mThread) {

        LOGW(LOG_FORMAT(" - Player thread not stopped"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);

        mMutex.lock();
        mEvent = EVT_EXIT;
        mMutex.unlock();

        if (mThread) mThread->join();
        if (mThread) delete mThread;
    }
    clean();
    destroyOpenAL1x();
}

bool Player::initOpenAL1x() {

    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);

    // Open a device
    ALCdevice* device = alcOpenDevice(NULL);
    if (!device) {

        LOGE(LOG_FORMAT(" - Failed to open OpenAL device"), __PRETTY_FUNCTION__, __LINE__);
        return false;
    }

    // Create a context
    ALCcontext* context = alcCreateContext(device, gs_contextAttribs);
    if (!context) {

        LOGE(LOG_FORMAT(" - Failed to create OpenAL context"), __PRETTY_FUNCTION__, __LINE__);
        return false;
    }

    // Enable context
    if (!alcMakeContextCurrent(context)) {

        LOGE(LOG_FORMAT(" - Context activtation failed"), __PRETTY_FUNCTION__, __LINE__);
        return false;
    }
    return true;
}
void Player::destroyOpenAL1x() {

    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);

    // Get context and device
    ALCcontext* context = alcGetCurrentContext();
    ALCdevice*  device = alcGetContextsDevice(context);

    // Disable context
    alcMakeContextCurrent(NULL);

    // Destroy context
    if (context) alcDestroyContext(context);

    // Close device
    if (device) alcCloseDevice(device);
}

bool Player::checkError(const char* file, int line) {

#ifdef DEBUG
    ALenum err = alGetError();
    if (err != AL_NO_ERROR) {

        LOGE(LOG_FORMAT(" - OpenAL error: 0x%X"), file, line, err);
        return false;
    }
#endif
    return true;
}

void Player::loadSound(unsigned char id) {

#ifdef __ANDROID__
    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - i:%d (j:%p; a:%p)"), __PRETTY_FUNCTION__, __LINE__, id, g_JavaVM,
            g_ActivityClass);
    assert(g_JavaVM);
    assert(g_ActivityClass);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_PLAYER, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return;

    jmethodID mthd = env->GetStaticMethodID(g_ActivityClass, "loadOGG", "(S)V");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'loadOGG' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return;
    }
    env->CallStaticVoidMethod(g_ActivityClass, mthd, static_cast<short>(id));   // NOTE: 'char' in Java is coded on 2 bytes
                                                                                // -> No need to use a 'char' type in Java
#else
    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - i:%d (a:%p)"), __PRETTY_FUNCTION__, __LINE__, id, g_AppleOS);
    assert(g_AppleOS);

    [g_AppleOS loadSound:id];
#endif
}
unsigned char Player::addSound(unsigned char id, size_t len, unsigned char* data, bool queued) {

    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - i:%d; l:%zd; d:%p; q:%s"), __PRETTY_FUNCTION__, __LINE__, id, len,  data,
            (queued)? "true":"false");
    assert(data);

    unsigned char soundIdx = getIndex(id);
    if (soundIdx != SOUND_IDX_INVALID) {

        LOGI(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - Existing sound entry (id:%d)"), __PRETTY_FUNCTION__, __LINE__, id);
        if (!mSounds[soundIdx]->isEmpty()) {

            LOGW(LOG_FORMAT(" - Sound ID %d already loaded"), __PRETTY_FUNCTION__, __LINE__, id);
            assert(NULL);
            delete [] data;
        }
        else
            mSounds[soundIdx]->restore(id, len, data, queued);
        return soundIdx;
    }
    OggSound* sound = new OggSound(id, len, data, queued);

    mMutex.lock();
    mSounds.push_back(sound);
    mMutex.unlock();
    return static_cast<unsigned char>(mSounds.size() - 1);
}
bool Player::rmvSound(unsigned char track) {

    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - track:%d"), __PRETTY_FUNCTION__, __LINE__, track);
    assert((track == ALL_TRACK) || (track < static_cast<unsigned char>(mSounds.size())));

    if ((mSounds[track]->getStatus() != AL_INITIAL) && (mSounds[track]->getStatus() != AL_STOPPED)) {

        LOGE(LOG_FORMAT(" - Unable to remove a sound in %d status"), __PRETTY_FUNCTION__, __LINE__,
                mSounds[track]->getStatus());
        assert(NULL);
        return false;
    }
    mMutex.lock();
    std::vector<OggSound*>::iterator iter = mSounds.begin();
    for (unsigned char i = 0; i < static_cast<unsigned char>(mSounds.size()); ++i, ++iter)
        if (i == track)
            break;

    delete (*iter);

    mSounds.erase(iter);
    mMutex.unlock();
    return true;
}

void Player::play(unsigned char track) {

    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - track:%d"), __PRETTY_FUNCTION__, __LINE__, track);
    assert((track == ALL_TRACK) || (track < static_cast<unsigned char>(mSounds.size())));
    assert(mThread);

    mMutex.lock();
    for (unsigned char i = ((track == ALL_TRACK)? 0:track);
            i < ((track == ALL_TRACK)? static_cast<unsigned char>(mSounds.size()):(track + 1)); ++i) {
        switch (mSounds[i]->getStatus()) {
            case AL_PLAYING: {

                LOGI(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
                assert(!mSounds[i]->getRepeat());

                alSourceStop(mSounds[i]->mSource);
                Player::checkError(__FILE__, __LINE__);
                //break;
            }
            case AL_INITIAL:
            case AL_STOPPED: {

                if (!mSounds[i]->getRepeat())
                    mSounds[i]->initialize();
                break;
            }
            //case AL_PAUSED:
        }

        LOGI(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - Play source %u"), __PRETTY_FUNCTION__, __LINE__, mSounds[i]->mSource);
        alSourcePlay(mSounds[i]->mSource);
        Player::checkError(__FILE__, __LINE__);
    }
    mMutex.unlock();
}
void Player::resume() {

    if (!mThread) { // Check coz 'resume' method can be called several time when several levels exist
                    // Or when need to play sound during loading a level (call this method before playing any track)

        LOGI(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);

        // Play previously paused track(s)
        mMutex.lock();
        for (std::vector<OggSound*>::iterator iter = mSounds.begin(); iter != mSounds.end(); ++iter) {
            if ((*iter)->getResumeFlag()) {

                LOGI(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
                assert((*iter)->getStatus() == AL_PAUSED);
                (*iter)->setResumeFlag(false);

                LOGI(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - Resume source %u"), __PRETTY_FUNCTION__, __LINE__,
                        (*iter)->mSource);
                alSourcePlay((*iter)->mSource);
                Player::checkError(__FILE__, __LINE__);
            }
        }
        mEvent = EVT_PLAY;
        mMutex.unlock(); // Needed ?!?

        mThread = new boost::thread(Player::startPlayerThread, this);
    }
    //else // Already resumed
}
void Player::pause(unsigned char track) {

    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - track:%d"), __PRETTY_FUNCTION__, __LINE__, track);
    assert((track == ALL_TRACK) || (track < static_cast<unsigned char>(mSounds.size())));

    if (track == ALL_TRACK) {

#ifdef DEBUG
        if (!mThread) {

            LOGI(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - Close game without having resume the player (track:%d)"),
                    __PRETTY_FUNCTION__, __LINE__, track);

            // Occured when the application is about to finish during a load operation
            // -> No 'resume' method called yet!
            return;
        }
#else
        if (!mThread)
            return;
#endif
        // Pause on all tracks means to stop player thread
        // -> Waiting for a 'resume' to restart playing paused tracks

        mMutex.lock();
        mEvent = EVT_PAUSE;
        mMutex.unlock();

        LOGI(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - Waiting thread stopped"), __PRETTY_FUNCTION__, __LINE__);
        if (mThread) mThread->join();
        if (mThread) delete mThread;

        mThread = NULL;
    }
    else {

        assert(mThread);
        assert(mSounds[track]->getStatus() == AL_PLAYING);

        mMutex.lock();
        alSourcePause(mSounds[track]->mSource);
        Player::checkError(__FILE__, __LINE__);
        mMutex.unlock();
    }
}
void Player::stop(unsigned char track) {

    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - track:%d"), __PRETTY_FUNCTION__, __LINE__, track);
    assert((track == ALL_TRACK) || (track < static_cast<unsigned char>(mSounds.size())));
    //assert(mThread); // To be able to stop sounds after a pause/resume operation

    mMutex.lock();
    for (unsigned char i = ((track == ALL_TRACK)? 0:track);
            i < ((track == ALL_TRACK)? static_cast<unsigned char>(mSounds.size()):(track + 1)); ++i) {

        LOGI(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - Stop source %u"), __PRETTY_FUNCTION__, __LINE__,
                mSounds[i]->mSource);
        mSounds[i]->stop(); // alSourceStop + Disable repeat option + Disable resume flag
        Player::checkError(__FILE__, __LINE__);
    }
    mMutex.unlock();
}
void Player::free() {

    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#ifdef __ANDROID__
    assert(!mThread);
    // ...stopped in 'Player::pause(ALL_TRACK)' method

    stopAll();

#else
    if (mThread) {

        mMutex.lock();
        mEvent = EVT_EXIT; // 'stopAll' method is called there
        mMutex.unlock();

        mThread->join();
        delete mThread;
        
        mThread = NULL;
    }
    else // When a level is deleted (see '~Level' destructor)
        stopAll();
#endif
}

void Player::setVolume(unsigned char track, float volume) {

    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - track:%d; volume:%f"), __PRETTY_FUNCTION__, __LINE__, track, volume);
    assert((track == ALL_TRACK) || (track < static_cast<unsigned char>(mSounds.size())));
    assert((volume > 0.f) && (volume < 1.f));

    for (unsigned char i = ((track == ALL_TRACK)? 0:track);
            i < ((track == ALL_TRACK)? static_cast<unsigned char>(mSounds.size()):(track + 1)); ++i)
        mSounds[i]->setVolume(volume);
}

void Player::setRepeat(unsigned char track) {

    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - track:%d"), __PRETTY_FUNCTION__, __LINE__, track);
    assert(track < static_cast<unsigned char>(mSounds.size()));
    assert((mSounds[track]->getStatus() == AL_INITIAL) || (mSounds[track]->getStatus() == AL_PAUSED));
    assert(mSounds[track]->getQueued());

    if (!mSounds[track]->getRepeat()) {

        mSounds[track]->setRepeat();
        mSounds[track]->initialize(); // Only called once (even if empty)
    }
    LOGI(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - Done!"), __PRETTY_FUNCTION__, __LINE__);
}

void Player::stopAll() { // Force to stop all sounds (even those in paused)

    mMutex.lock();
    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    for (std::vector<OggSound*>::iterator iter = mSounds.begin(); iter != mSounds.end(); ++iter) {

        alSourceStop((*iter)->mSource);
        Player::checkError(__FILE__, __LINE__);
    }
    mMutex.unlock();
}

void Player::playThreadRunning() {

    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - Start loop"), __PRETTY_FUNCTION__, __LINE__);

    bool bAbort = false;
    while (!bAbort) {

        mMutex.lock();
        switch (mEvent) {

            case EVT_PLAY: { // Let's playing sounds (if needed)

#ifdef DEBUG
                ++mLogIdle;
                LOGI(LIBENG_LOG_PLAYER, (mLogIdle % 100), LOG_FORMAT(" - EVT_PLAY"), __PRETTY_FUNCTION__, __LINE__);
#endif
                // Manage queued buffers on all sounds (if needed)
                for (std::vector<OggSound*>::iterator iter = mSounds.begin(); iter != mSounds.end(); ++iter)
                    if ((*iter)->getStatus() == AL_PLAYING)
                        (*iter)->updateQueuedBuffers();

                mMutex.unlock();
                boost::this_thread::sleep(boost::posix_time::milliseconds(10));
                break;
            }
            case EVT_PAUSE: { // Pause track(s)

                mMutex.unlock();
                LOGI(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - EVT_PAUSE"), __PRETTY_FUNCTION__, __LINE__);

                for (unsigned char i = 0; i < static_cast<unsigned char>(mSounds.size()); ++i) {
                    if (mSounds[i]->getStatus() == AL_PLAYING) {

                        // Set resume operation
                        mSounds[i]->setResumeFlag(true);

                        LOGI(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - Pause source %u"), __PRETTY_FUNCTION__, __LINE__,
                                mSounds[i]->mSource);
                        alSourcePause(mSounds[i]->mSource);
                        Player::checkError(__FILE__, __LINE__);
                    }
                }
                bAbort = true; // Exit
                break;
            }
            case EVT_EXIT: {

                mMutex.unlock();
                LOGI(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - EVT_EXIT"), __PRETTY_FUNCTION__, __LINE__);

                stopAll();

                bAbort = true; // Exit
                break;
            }
        }
    }

    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - Finished"), __PRETTY_FUNCTION__, __LINE__);
}
void Player::startPlayerThread(Player* player) {

    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - p:%p"), __PRETTY_FUNCTION__, __LINE__, player);
    player->playThreadRunning();
}

} // namespace
