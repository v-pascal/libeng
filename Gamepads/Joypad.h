#ifndef LIBENG_JOYPAD_H_
#define LIBENG_JOYPAD_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_GAMEPAD

#include <libeng/Game/Game.h>
#include <libeng/Log/Log.h>
#include <cstring>

namespace eng {

static const unsigned char MAX_BUTTON_COUNT = 4;

typedef struct {

    TouchArea forward;
    TouchArea backward;
    TouchArea up;
    TouchArea down;

    TouchArea* buttons[MAX_BUTTON_COUNT];
    // -> The first button touch area must be the most used button

} JoyConfig;

//////
class Joypad : public Gamepad {

public:
    typedef enum {

        JPD_NONE = 0,
        JPD_FORWARD,
        JPD_BACKWARD,
        JPD_DOWN,
        JPD_UP

    } Direction;
    typedef struct {

        bool buttonPushed[MAX_BUTTON_COUNT]; // According button index defined in 'mConfig'
        // -> Only one button can be pushed at same time

        Direction direction;

    } Data;

protected:
#ifdef DEBUG
    const unsigned int* mLog;
#endif
    JoyConfig mConfig;
    Data mCurData;

public:
    Joypad();
    virtual ~Joypad();

    // WARNING: Should call 'initialize' method below after 'Game2D::start' method call (portrait as landscape management)
#ifdef DEBUG
    virtual void initialize(const Game* game, const JoyConfig* config, const unsigned int* log) {

        LOGV(LIBENG_LOG_GAMEPAD, 0, LOG_FORMAT(" - g:%p; c:%p; l:%p"), __PRETTY_FUNCTION__, __LINE__, game, config, log);
        assert(config->forward.left < config->forward.right);
        assert(config->forward.top < config->forward.bottom);
        assert(config->backward.left < config->backward.right);
        assert(config->backward.top < config->backward.bottom);
        assert(config->up.left < config->up.right);
        assert(config->up.top < config->up.bottom);
        assert(config->down.left < config->down.right);
        assert(config->down.top < config->down.bottom);

        assert((!config->buttons[0]) || ((config->buttons[0]->left < config->buttons[0]->right) &&
                (config->buttons[0]->top < config->buttons[0]->bottom)));
        assert((!config->buttons[1]) || ((config->buttons[1]->left < config->buttons[1]->right) &&
                (config->buttons[1]->top < config->buttons[1]->bottom)));
        assert((!config->buttons[2]) || ((config->buttons[2]->left < config->buttons[2]->right) &&
                (config->buttons[2]->top < config->buttons[2]->bottom)));
        assert((!config->buttons[3]) || ((config->buttons[3]->left < config->buttons[3]->right) &&
                (config->buttons[3]->top < config->buttons[3]->bottom))); // 3 == (MAX_BUTTON_COUNT - 1)
#else
    virtual void initialize(const Game* game, const JoyConfig* config) {
#endif
        init(game);
        std::memcpy(&mConfig, config, sizeof(JoyConfig));

#ifdef LIBENG_PORT_AS_LAND
        mConfig.forward.top = game->getScreen()->height - mConfig.forward.top;
        mConfig.forward.bottom = game->getScreen()->height - mConfig.forward.bottom;
        mConfig.backward.top = game->getScreen()->height - mConfig.backward.top;
        mConfig.backward.bottom = game->getScreen()->height - mConfig.backward.bottom;
        mConfig.up.top = game->getScreen()->height - mConfig.up.top;
        mConfig.up.bottom = game->getScreen()->height - mConfig.up.bottom;
        mConfig.down.top = game->getScreen()->height - mConfig.down.top;
        mConfig.down.bottom = game->getScreen()->height - mConfig.down.bottom;

        for (short i = 0; i < MAX_BUTTON_COUNT; ++i) {
            if (mConfig.buttons[i]) {

                mConfig.buttons[i]->top = game->getScreen()->height - mConfig.buttons[i]->top;
                mConfig.buttons[i]->bottom = game->getScreen()->height - mConfig.buttons[i]->bottom;
            }
        }

        // WARNING: Loss of performance occured when 'g_ScreenHeight' is substracted (above)
#endif

#ifdef DEBUG
        mLog = log;

        LOGI(LIBENG_LOG_GAMEPAD, 0, LOG_FORMAT(" - forward [l:%d; t:%d; r:%d; b:%d]"), __PRETTY_FUNCTION__, __LINE__,
                mConfig.forward.left, mConfig.forward.top, mConfig.forward.right, mConfig.forward.bottom);
        LOGI(LIBENG_LOG_GAMEPAD, 0, LOG_FORMAT(" - backward [l:%d; t:%d; r:%d; b:%d]"), __PRETTY_FUNCTION__, __LINE__,
                mConfig.backward.left, mConfig.backward.top, mConfig.backward.right, mConfig.backward.bottom);
        LOGI(LIBENG_LOG_GAMEPAD, 0, LOG_FORMAT(" - up [l:%d; t:%d; r:%d; b:%d]"), __PRETTY_FUNCTION__, __LINE__,
                mConfig.up.left, mConfig.up.top, mConfig.up.right, mConfig.up.bottom);
        LOGI(LIBENG_LOG_GAMEPAD, 0, LOG_FORMAT(" - down [l:%d; t:%d; r:%d; b:%d]"), __PRETTY_FUNCTION__, __LINE__,
                mConfig.down.left, mConfig.down.top, mConfig.down.right, mConfig.down.bottom);

        for (short i = 0; i < MAX_BUTTON_COUNT; ++i) {
            if (mConfig.buttons[i]) {
                LOGI(LIBENG_LOG_GAMEPAD, 0, LOG_FORMAT(" - button %d [l:%d; t:%d; r:%d; b:%d]"), __PRETTY_FUNCTION__, __LINE__,
                        i, mConfig.buttons[i]->left, mConfig.buttons[i]->top, mConfig.buttons[i]->right,
                        mConfig.buttons[i]->bottom);
            }
        }
#endif
    }

    //////
    virtual void refresh();
    inline const void* getData() const { return static_cast<const void*>(&mCurData); }

};

} // namespace

// Cast(s)
#define joypadVia(p)    static_cast<const eng::Joypad*>(p)
#define joyDataVia(d)   static_cast<const eng::Joypad::Data*>(d)

#endif // LIBENG_ENABLE_GAMEPAD
#endif // __cplusplus
#endif // LIBENG_JOYPAD_H_
