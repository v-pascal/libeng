#ifndef RENDER2D_H_
#define RENDER2D_H_

#include <libeng/Render/Render.h>
#include <libeng/Render/Shader.h>
#include <libeng/Render/2D/Shader2D.h>
#include <libeng/Game/2D/Game2D.h>

//////
class Render2D : public Render {

private:
    Render2D();
    virtual ~Render2D();

public:
    static Render2D* getInstance() {
        if (!Render::mThis)
            Render::mThis = new Render2D;
        return static_cast<Render2D*>(Render::mThis);
    }
    static void freeInstance() {
        if (mThis) {
            delete mThis;
            mThis = NULL;
        }
    }

	void initialize(eng::Game* game, float xDpi, float yDpi) {

	    Render::initialize(game, xDpi, yDpi); // Assign 'mGame', 'xDpi' & 'yDpi'
	    static_cast<eng::Game2D*>(mGame)->initialize(&mScreen, &mShaderParam, &mShaderParam2D);
	}

    //////
    bool init();
    void begin();

private:
    eng::ShaderParam mShaderParam;
    eng::ShaderParam2D mShaderParam2D;

};

#endif // RENDER2D_H_
