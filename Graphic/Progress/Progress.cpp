#include "Progress.h"

#ifdef LIBENG_ENABLE_PROGRESS
#include <libeng/Graphic/Text/2D/Text2D.h>
#include <libeng/Game/2D/Game2D.h>
#include <string.h>

#define DEFAULT_LOCATION    20 // Progress bar height & position from the bottom of the screen (in pixel)

namespace eng {

//////
Progress::Progress(bool manage) : mTodo(manage), mDone(manage), mScreen(NULL), mPos(0), mIncWidth(0.f) {

    LOGV(LIBENG_LOG_PROGRESS, 0, LOG_FORMAT(" - m:%s"), __PRETTY_FUNCTION__, __LINE__, (manage)? "true":"false");
    memset(&mInfo, 0, sizeof(Info));

#ifdef DEBUG
    mLog = NULL;
#endif
}
Progress::~Progress() { LOGV(LIBENG_LOG_PROGRESS, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }

void Progress::initialize(const Game2D* game) {

    LOGV(LIBENG_LOG_PROGRESS, 0, LOG_FORMAT(" - g:%x"), __PRETTY_FUNCTION__, __LINE__, game);
#ifdef DEBUG
    mLog = game->getLog();
#endif
    mScreen = game->getScreen();

    mTodo.initialize(game);
    mDone.initialize(game);
}

void Progress::reset() {

    LOGV(LIBENG_LOG_PROGRESS, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);

    // 'mTodo' should be already positionned
    mDone.copyVertices(mTodo);
    mIncWidth = (mTodo.getRight() - mTodo.getLeft()) / static_cast<float>(mInfo.maxPos);

    // Position progression at the first position (mPos == 0)
    // -> Not displayed yet (see 'render' method)
    mDone.set(-mInfo.maxPos * mIncWidth);
}
void Progress::position(short x0, short y0, short x2, short y2) {

    LOGV(LIBENG_LOG_PROGRESS, 0, LOG_FORMAT(" - x0:%d; y0:%d; x2:%d; y2:%d"), __PRETTY_FUNCTION__, __LINE__, x0, y0,
            x2, y2);

    mTodo.setVertices(x0, y0, x2, y2);
    reset(); // ...call after having positionned 'mTodo'
}

void Progress::resume() {

    LOGV(LIBENG_LOG_PROGRESS, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    mTodo.resume(mInfo.todoRed, mInfo.todoGreen, mInfo.todoBlue);
    mDone.resume(mInfo.doneRed, mInfo.doneGreen, mInfo.doneBlue);

    if (mPos) {

        mDone.set(-mPos * mIncWidth);
        mPos = 0;
    }
}

void Progress::start(const Info* info) {

    LOGV(LIBENG_LOG_PROGRESS, 0, LOG_FORMAT(" - i:%p"), __PRETTY_FUNCTION__, __LINE__, info);
    assert(info->maxPos > 0);

    memcpy(&mInfo, info, sizeof(Info));
    resume();

    mTodo.setTexCoords(FULL_TEXCOORD_BUFFER);
    mDone.setTexCoords(FULL_TEXCOORD_BUFFER);

    // Position the progress bar as default (at the bottom of the screen) + reset it
    mTodo.setVertices(DEFAULT_LOCATION, 2 * DEFAULT_LOCATION, mScreen->width - DEFAULT_LOCATION, DEFAULT_LOCATION);
    reset();
}
void Progress::set(unsigned char position) {

    LOGV(LIBENG_LOG_PROGRESS, 0, LOG_FORMAT(" - p:%d"), __PRETTY_FUNCTION__, __LINE__, position);
    assert(position >= 0);
    assert(position <= mInfo.maxPos);

    mDone.set((position - mPos) * mIncWidth);
    mPos = position;
}
void Progress::setMax(unsigned char newMax) {

    LOGV(LIBENG_LOG_PROGRESS, 0, LOG_FORMAT(" - m:%d"), __PRETTY_FUNCTION__, __LINE__, newMax);
    assert(newMax > 0);

    mInfo.maxPos = newMax;

    mPos = 0;
    reset();
}

void Progress::render(bool resetUniform) const {

#ifdef DEBUG
    LOGV(LIBENG_LOG_PROGRESS, (*mLog % 100), LOG_FORMAT(" - r:%s (p:%d)"), __PRETTY_FUNCTION__, __LINE__,
            (resetUniform)? "true":"false", mPos);
#endif
    if (mPos) {

        mTodo.render(resetUniform);
        mDone.render(false);
    }
    else
        mTodo.render(resetUniform);
}

} // namespace

#endif // LIBENG_ENABLE_PROGRESS
