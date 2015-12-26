#ifndef LIBENG_LEVEL_H_
#define LIBENG_LEVEL_H_
#if defined(__cplusplus)

#include <libeng/Game/Game.h>

namespace eng {

//////
class Level {

protected:
    Textures* mTextures;

    unsigned char mLoadStep; // Used to check which object to create/initialize/start
                             // -> Needed when loading a level (display a progress bar)
public:
    Level(Game* game);
    virtual ~Level();

    //////
    virtual void initialize();
    virtual void pause();

    virtual void wait(float millis) = 0;

protected:
    virtual bool loaded(const Game* game) = 0; // Return TRUE when done (change level using 'Game::setGameLevel')
public:
    bool load(const Game* game);
    virtual bool update(const Game* game) = 0; // Return FALSE when done (change level using 'Game::setGameLevel')
    // RULES: 'mGameLevel' is odd -> Reserved to create, initialize & start objects / restart objects for resume ('load' method)
    //        'mGameLevel' is pair -> Reserved to update & check objects collision ('update' method)

    virtual void renderLoad() const = 0;
    virtual void renderUpdate() const = 0;

};

} // namespace

#endif // __cplusplus
#endif // LIBENG_LEVEL_H_
