#ifndef LIBENG_PROGRESS_H_
#define LIBENG_PROGRESS_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_PROGRESS

#include <libeng/Graphic/Object/2D/Static2D.h>

namespace eng {

class IncBar : public Static2D {

public:
    IncBar(bool manage) : Static2D(manage) {
        LOGV(LIBENG_LOG_PROGRESS, 0, LOG_FORMAT(" - m:%s"), __PRETTY_FUNCTION__, __LINE__, (manage)? "true":"false");
    }
    virtual ~IncBar() { LOGV(LIBENG_LOG_PROGRESS, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }

    //////
    inline void set(float lag) {

        LOGV(LIBENG_LOG_PROGRESS, 0, LOG_FORMAT(" - l:%f"), __PRETTY_FUNCTION__, __LINE__, lag);
        mVertices[4] += lag;
        mVertices[6] += lag;
    };
};

//////
class Progress {

public:
    typedef struct {

        unsigned char maxPos;

        unsigned char todoRed;
        unsigned char todoGreen;
        unsigned char todoBlue;

        unsigned char doneRed;
        unsigned char doneGreen;
        unsigned char doneBlue;

    } Info;

private:
    Static2D mTodo;
    IncBar mDone;
#ifdef DEBUG
    const unsigned int* mLog;
#endif
    const Screen* mScreen;

    unsigned char mPos;
    Info mInfo;

    float mIncWidth; // Width to increase according progress bar width (in unit)
    void reset();

public:
    Progress(bool manage = true);
    virtual ~Progress();

    void initialize(const Game2D* game);
    void position(short x0, short y0, short x2, short y2); // Can be called only after 'start' method call

    inline void pause() {

        LOGV(LIBENG_LOG_PROGRESS, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        mTodo.pause();
        mDone.pause();
    };
    void resume();

    //////
    void start(const Info* info);
    void set(unsigned char position);

    void setMax(unsigned char newMax);
    // WARNING:
    // * This method will reset the progess bar (change max position + calling 'set(0)' method)
    // * To be called after having positionned it (call 'position' method B4)

    void render(bool resetUniform) const;

};

} // namespace

#endif // LIBENG_ENABLE_PROGRESS
#endif // __cplusplus
#endif // LIBENG_PROGRESS_H_
