#include "Level.h"

#ifndef __ANDROID__
#include "OpenGLES/ES2/gl.h"
#endif
#include <libeng/Log/Log.h>

namespace eng {

//////
Level::Level(Game* game) : mLoadStep(0), mTextures(NULL) {

    LOGV(LIBENG_LOG_LEVEL, 0, LOG_FORMAT(" - g:%p"), __PRETTY_FUNCTION__, __LINE__, game);
    game->resetInputs();
}
Level::~Level() {

#ifdef DEBUG
    LOGV(LIBENG_LOG_LEVEL, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);

    // Check all texture objects has been deleted correctly
    if (!mTextures->chkTextures()) {
        LOGW(LOG_FORMAT(" - Bad textures management"), __PRETTY_FUNCTION__, __LINE__);
    }
    else {
        LOGI(LIBENG_LOG_LEVEL, 0, LOG_FORMAT(" - Good textures management"), __PRETTY_FUNCTION__, __LINE__);
    }
#endif
    mTextures->delTextures();
    mTextures->rmvTextures();

    Player* player = Player::getInstance();
    player->pause(ALL_TRACK);
    player->free();
    player->clean();

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    // -> Clear screen coz when changing level it will need to be loaded and cannot display anything during this
    // time considering the screen black (so clear it)
}

void Level::initialize() {

    LOGV(LIBENG_LOG_LEVEL, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);

    mTextures = Textures::getInstance();
    // Avoid to check existing instance each time it is used
}
bool Level::load(const Game* game) {

    LOGV(LIBENG_LOG_LEVEL, 0, LOG_FORMAT(" - g:%p (l:%d)"), __PRETTY_FUNCTION__, __LINE__, game, mLoadStep);
    ++mLoadStep;

    if (loaded(game)) {

        LOGI(LIBENG_LOG_LEVEL, 0, LOG_FORMAT(" - Loaded"), __PRETTY_FUNCTION__, __LINE__);
        Player::getInstance()->resume();
        return true;
    }
    return false;
}
void Level::pause() {

    LOGV(LIBENG_LOG_LEVEL, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    mLoadStep = 0;
}

} // namespace
