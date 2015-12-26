#ifndef LIBENG_GAME2D_H_
#define LIBENG_GAME2D_H_
#if defined(__cplusplus)

#include <libeng/Game/Game.h>
#include <libeng/Render/2D/Shader2D.h>
#include <libeng/Intro/Intro.h>

namespace eng {

//////
class Game2D : public Game {

private:
    const ShaderParam2D* mShaderParam2D;

protected:
    Game2D(unsigned char level);
    virtual ~Game2D();

    mutable Intro* mGameIntro;

public:
    inline const ShaderParam2D* getShader2D() const { return mShaderParam2D; }

    inline void initialize(Screen* screen, const ShaderParam* shader, const ShaderParam2D* shader2D) {

        Game::initialize(screen, shader);
        mShaderParam2D = shader2D;
        if (mGameIntro)
            mGameIntro->initialize(this);

        init();
    }

    void updateIntro() const;
    void renderIntro() const;

    virtual void pause();

    //////
    virtual bool start();
    virtual void destroy();

};

} // namespace

// Cast(s)
#define game2DVia(g)    static_cast<const eng::Game2D*>(g)

#endif // __cplusplus
#endif // LIBENG_GAME2D_H_
