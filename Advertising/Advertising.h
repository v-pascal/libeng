#ifndef ADVERTISING_H_
#define ADVERTISING_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_ADVERTISING

#include <time.h>
#include <cstddef>

namespace eng {

static const unsigned char ADV_REFRESH_DELAY = 46; // Between 45 and 60 seconds

//////
class Advertising {

private:
    Advertising();
    virtual ~Advertising();

    static Advertising* mThis;

    time_t mPublished;
    mutable bool mLoading;

public:
    static Advertising* getInstance() {
        if (!mThis)
            mThis = new Advertising;
        return mThis;
    }
    static void freeInstance() {
        if (mThis) {
            delete mThis;
            mThis = NULL;
        }
    }

    static bool display(unsigned char id);
    static bool hide(unsigned char id);

    static unsigned char getStatus();

    enum {

        STATUS_NONE = 0, // Advertising management disabled in Java
        STATUS_READY,
        STATUS_LOADING,
        STATUS_LOADED,
        STATUS_FAILED,
        STATUS_DISPLAYING,
        STATUS_DISPLAYED
    };

    inline bool isLoading() const { return mLoading; }

    //////
    bool load() const;
    unsigned char refresh();

    void resetPubDelay();
    unsigned char getPubDelay() const; // Return publication delay (in seconds)

};

} // namespace

#endif // LIBENG_ENABLE_ADVERTISING
#endif // __cplusplus
#endif // ADVERTISING_H_
