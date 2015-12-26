#ifndef LIBENG_OGGSOUND_H_
#define LIBENG_OGGSOUND_H_
#if defined(__cplusplus)

#include <libeng/Log/Log.h>

#ifdef __ANDROID__
#include <AL/al.h>
#include <AL/alc.h>

#else
#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#endif
#include <vorbis/vorbisfile.h>
#include <assert.h>

namespace eng {

static const float DEFAULT_VOLUME = 0.5f; // Half gain
static const unsigned char QBUFFER_COUNT = 4; // Queued buffer count

typedef struct {

	unsigned char* buffer;
    size_t size;
    size_t offset;

} MemoryStream;

//////
class OggSound {

public:
	OggSound(unsigned char id, size_t len, unsigned char* data, bool queued);
	virtual ~OggSound();

	ALuint mSource;

	bool initialize(bool bFirst = false);
	void updateQueuedBuffers();

    void stop(); // alSourceStop

    //
    inline unsigned char getId() const { return mId; }
    ALint getStatus() const;
	inline void setResumeFlag(bool resume) { mResumeFlag = resume; }
	inline bool getResumeFlag() const { return mResumeFlag; }

    void setVolume(float volume);
    inline float getVolume() const { return mVolume; }
    
    inline bool getQueued() const { return mQueued; }

    inline void setRepeat() { mRepeat = true; }
    inline bool getRepeat() const { return mRepeat; }

    // Restore 'MemoryStream->buffer' buffer after having been freed (see 'Player::empty' method)
    inline void restore(unsigned char id, size_t len, unsigned char* data, bool queued) {

        LOGV(LIBENG_LOG_PLAYER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        assert(mId == id);
        assert(mStream->size == len);
        assert(!mStream->buffer);
        assert(mQueued == queued);

        mStream->buffer = data;
    };
    inline void empty() {

        if (mStream->buffer) {

            delete [] mStream->buffer;
            mStream->buffer = NULL;
        }
        //else // Can be already empty
        // -> when pause again or close application, without having reload this sound buffer
    };
    inline bool isEmpty() const { return (mStream->buffer == NULL); }

private:
    unsigned char mId;
#ifdef DEBUG
	unsigned int mLogUpd;
#endif
	ALuint mQBuffers[QBUFFER_COUNT];
	ALuint mBuffer;
	MemoryStream* mStream;
	OggVorbis_File mOvf;

    bool mQueued;
    bool mRepeat;

	ALenum mFormat;
	ALsizei mFreq;

	bool mResumeFlag;
	float mVolume;

	static bool checkError(const char* file, int line);
};

} // namespace

#endif // __cplusplus
#endif // LIBENG_OGGSOUND_H_
