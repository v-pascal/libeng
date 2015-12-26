#ifndef LIBENG_GAMEPAD_H_
#define LIBENG_GAMEPAD_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_GAMEPAD

#include <stddef.h>

namespace eng {

typedef struct {

    short left;
    short top;
    short right;
    short bottom;
    // ...with all data above in touch coordinates (in pixel)
    // -> With [0;0]: Top/left coordinates; [g_ScreenWidth;g_ScreenHeight]: Bottom/right coordinates

} TouchArea;

class Game;

//////
class Gamepad {

protected:
    const Game* mGame;

public:
    Gamepad() : mGame(NULL) { }
    virtual ~Gamepad() { }

    inline void init(const Game* game) { mGame = game; }

    //////
    virtual void refresh() = 0;
    virtual const void* getData() const = 0;

};

} // namespace

#endif // LIBENG_ENABLE_GAMEPAD
#endif // __cplusplus
#endif // LIBENG_GAMEPAD_H_
