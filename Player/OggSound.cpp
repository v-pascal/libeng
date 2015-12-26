#include "OggSound.h"

#include <assert.h>
#include <string>
#include <cstring>

#define QBUFFER_SIZE   (16 * 1024)
#define BUFFER_SIZE    (128 * 1024)
#ifdef DEBUG
#define MAX_DATA_LEN   50000 // Maximum length data size for a not queued buffer
#endif

//////
static size_t read_func(void *ptr, size_t size, size_t nmemb, void *datasource) {

    LOGV(LIBENG_LOG_PLAYER + 1, 0, LOG_FORMAT(" - p:%p; s:%zd; n:%zd; d:%p"), __PRETTY_FUNCTION__, __LINE__, ptr, size, nmemb,
            datasource);
    assert(size == 1); // size always is 1

    eng::MemoryStream* stream = (eng::MemoryStream*)datasource;
    size_t length = nmemb;
    if (length > stream->size - stream->offset) {
        length = stream->size - stream->offset;
    }
    if (length > 0) {
        std::memcpy(ptr, stream->buffer + stream->offset, length);
        stream->offset += length;
    }
    return length;
}
static int seek_func(void *datasource, ogg_int64_t offset, int whence) {

    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - d:%p; o:%lld; w:%d"), __PRETTY_FUNCTION__, __LINE__, datasource, offset, whence);
    return -1;
}
static int close_func(void *datasource) {

    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - d:%p"), __PRETTY_FUNCTION__, __LINE__, datasource);
    return 0;
}

char g_decodedQBuff[QBUFFER_SIZE] = {0};
char g_decodedBuff[BUFFER_SIZE] = {0};
ov_callbacks g_ovCallbacks = {
    read_func,
    seek_func,
    close_func,
    NULL,
};

size_t oggDecode(OggVorbis_File* ovf, char* buffer, size_t size) { // will decode ogg file as much as can into 'buffer'

    size_t left = size;
    while (left != 0) {

        long decoded = ov_read(ovf, buffer, left, 0, 2, 1, NULL);
        if (decoded > 0) {
            buffer += decoded;
            left -= decoded;
        }
        else if (decoded == 0) {
            break;
        }
        else {
            switch (decoded) {
                case OV_HOLE: {
                    //LOGW(LOG_FORMAT(" - Indicates there was an interruption in the data"), __PRETTY_FUNCTION__,
                    //     __LINE__);

                    break; // ...occured when repeat option is active (don't care! No effect)
                }
                case OV_EBADLINK: {
                    LOGW(LOG_FORMAT(" - Indicates that an invalid stream section was supplied to libvorbisfile, or the "
                            "requested link is corrupt"), __PRETTY_FUNCTION__, __LINE__);
                    break;
                }
                case OV_EINVAL: {
                    LOGW(LOG_FORMAT(" - Indicates the initial file headers couldn't be read or are corrupt, or that the "
                            "initial open call for 'ovf' failed"), __PRETTY_FUNCTION__, __LINE__);
                    break;
                }
#ifdef DEBUG
                default: {
                    LOGU(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - decoded:%d"), __PRETTY_FUNCTION__, __LINE__, decoded);
                    break;
                }
#endif
            }
        }
    }
    return (size - left);
}

namespace eng {

//////
OggSound::OggSound(unsigned char id, size_t len, unsigned char* data, bool queued) : mResumeFlag(false),
        mRepeat(false), mId(id) {

#ifdef DEBUG
    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - i:%d; l:%zd; d:%p; q:%s"), __PRETTY_FUNCTION__, __LINE__, id, len, data,
            (queued)? "true":"false");
    mLogUpd = 0;
#endif

    mStream = new MemoryStream;
    std::memset(mStream, 0, sizeof(MemoryStream));
    std::memset(&mOvf, 0, sizeof(OggVorbis_File));

    mFormat = mFreq = mSource = 0;

    mStream->buffer = data;
    mStream->size = len;
    //mStream->offset = 0;

    LOGI(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - Creating source"), __PRETTY_FUNCTION__, __LINE__);
    alGenSources(1, &mSource);
    OggSound::checkError(__FILE__, __LINE__);

    mQueued = queued;
    if (!mQueued) {

#ifdef DEBUG
        LOGI(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - Source %u do not support queued buffers"), __PRETTY_FUNCTION__,
             __LINE__, mSource);
        assert(len < MAX_DATA_LEN);
#endif
        LOGI(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - Creating buffer"), __PRETTY_FUNCTION__, __LINE__);
        alGenBuffers(1, &mBuffer);
        OggSound::checkError(__FILE__, __LINE__);

        initialize(true);
    }
    else {

        mQBuffers[0] = 0;

        LOGI(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - Creating %d buffers"), __PRETTY_FUNCTION__, __LINE__, QBUFFER_COUNT);
        alGenBuffers(QBUFFER_COUNT, mQBuffers);
        OggSound::checkError(__FILE__, __LINE__);
    }

    // Set default volume
    setVolume(DEFAULT_VOLUME);
}
OggSound::~OggSound() {

    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    if (mSource) {

        //alSourceStop(mSource);
        alSourcei(mSource, AL_BUFFER, 0);
    }
    if (mQueued) {
        if (mQBuffers[0])
            alDeleteBuffers(QBUFFER_COUNT, mQBuffers);
    }
    else
        alDeleteBuffers(1, &mBuffer);
    if (mSource) alDeleteSources(1, &mSource);
    if (mOvf.datasource) ov_clear(&mOvf);
    if (mStream->buffer) {
        delete [] mStream->buffer;
    }
    delete mStream;
}

void OggSound::setVolume(float volume) {

    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - Volume: %f"), __PRETTY_FUNCTION__, __LINE__, volume);
    mVolume = volume;
    alSourcef(mSource, AL_GAIN, mVolume);
}

bool OggSound::initialize(bool bFirst) {

    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - f:%s"), __PRETTY_FUNCTION__, __LINE__, (bFirst)? "true":"false");
    assert(mStream->buffer);

    if ((!mQueued) && (!bFirst))
        return true; // Only for not queued buffers

    alSourcei(mSource, AL_BUFFER, 0);
    if (!OggSound::checkError(__FILE__, __LINE__)) return false;

    mStream->offset = 0;
    ov_clear(&mOvf);
    if (ov_open_callbacks(mStream, &mOvf, NULL, 0, g_ovCallbacks) != 0) {

        LOGE(LOG_FORMAT(" - Failed to open ogg file"), __PRETTY_FUNCTION__, __LINE__);
        return false;
    }

    int channels = ov_info(&mOvf, 0)->channels;
    if ((channels != 1) && (channels != 2)) {
        LOGE(LOG_FORMAT(" - Unsupported ogg file - %d channels. Only mono or stereo supported"), __PRETTY_FUNCTION__,
                __LINE__, channels);
        return false;
    }
    mFormat = (channels == 1)? AL_FORMAT_MONO16:AL_FORMAT_STEREO16;
    mFreq = ov_info(&mOvf, 0)->rate;
    
    if (mQueued) {

        // Fill initial queued buffers
        LOGI(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(" - Filling initial buffers"), __PRETTY_FUNCTION__, __LINE__);
        for (unsigned char i = 0; i < QBUFFER_COUNT; ++i) { // Queued buffers count
            
            size_t decoded = oggDecode(&mOvf, g_decodedQBuff, QBUFFER_SIZE);
            if (decoded) {
                
                alBufferData(mQBuffers[i], mFormat, g_decodedQBuff, decoded, mFreq);
                if (!OggSound::checkError(__FILE__, __LINE__)) return false;
                
                alSourceQueueBuffers(mSource, 1, &mQBuffers[i]); // &mQBuffers[i] -> buffer ID
                if (!OggSound::checkError(__FILE__, __LINE__)) return false;
            }
        }
    }
    else {

        // Fill buffer (only once)
        size_t decoded = oggDecode(&mOvf, g_decodedBuff, BUFFER_SIZE);

        LOGI(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        assert(decoded);
        alBufferData(mBuffer, mFormat, g_decodedBuff, decoded, mFreq);
        if (!OggSound::checkError(__FILE__, __LINE__)) return false;

        alSourceQueueBuffers(mSource, 1, &mBuffer);
        if (!OggSound::checkError(__FILE__, __LINE__)) return false;
    }
    return true;
}
void OggSound::updateQueuedBuffers() {

#ifdef DEBUG
    if (!mStream->buffer) {

        LOGE(LOG_FORMAT(" - Empty queue buffer"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
    }
#endif
    if (!mQueued)
        return; // Reserved for queued buffers

#ifdef DEBUG
    ++mLogUpd;
    LOGV(LIBENG_LOG_PLAYER, (mLogUpd % 100), LOG_FORMAT(" - o:%zd; s:%zd"),  __PRETTY_FUNCTION__, __LINE__,
            mStream->offset, mStream->size);
#endif
    if (mStream->offset < mStream->size) {

        ALint processed;
        alGetSourcei(mSource, AL_BUFFERS_PROCESSED, &processed);
        if (!OggSound::checkError(__FILE__, __LINE__)) return;

        // Unqueue and refill processed buffers
        while (processed > 0) {

            ALuint bufFree;
            alSourceUnqueueBuffers(mSource, 1, &bufFree); // &bufFree -> Free buffer ID (ready to be used)
            if (!OggSound::checkError(__FILE__, __LINE__)) return;

            size_t decoded = oggDecode(&mOvf, g_decodedQBuff, QBUFFER_SIZE);
            if (decoded) {

                alBufferData(bufFree, mFormat, g_decodedQBuff, decoded, mFreq); // bufFree filled here!
                if (!OggSound::checkError(__FILE__, __LINE__)) return;

                alSourceQueueBuffers(mSource, 1, &bufFree); // &bufFree used here
                if (!OggSound::checkError(__FILE__, __LINE__)) return;
            }
            --processed;
        }
    }
    else if (mRepeat)
        mStream->offset = 0; // Restart decoding from the beginning
}

void OggSound::stop() {

    LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);

    alSourceStop(mSource);
    OggSound::checkError(__FILE__, __LINE__);

    mRepeat = false;
    mResumeFlag = false;
}
ALint OggSound::getStatus() const {

    if (!alIsSource(mSource))
        return 0;

    ALint state = 0;
    alGetSourcei(mSource, AL_SOURCE_STATE, &state);
    if (!OggSound::checkError(__FILE__, __LINE__)) return 0;

    return state;
}

bool OggSound::checkError(const char* file, int line) {

#ifdef DEBUG
    ALenum err = alGetError();
    if (err != AL_NO_ERROR) {

        LOGE(LOG_FORMAT(" - OpenAL error: 0x%X"), file, line, err);
        return false;
    }
#endif
    return true;
}

} // namespace
