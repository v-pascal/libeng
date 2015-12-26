#ifndef LIBENG_OBJECT2D_H_
#define LIBENG_OBJECT2D_H_
#if defined(__cplusplus)

#include <libeng/Graphic/Object/Object.h>
#include <libeng/Textures/Textures.h>
#include <libeng/Render/Shader.h>
#include <libeng/Render/2D/Shader2D.h>

namespace eng {

class Game2D;

//////
class Object2D : public Object {

protected:
    Textures* mTextures;
    unsigned char mTextureIdx;

    inline const ShaderParam2D* getShader2D() const { return mShaderParam2D; }
    void transform() const;

private:
    const ShaderParam* mShaderParam;
    const ShaderParam2D* mShaderParam2D;

    bool mManageTexture; // TRUE: If texture is always used (i.e there is at least one object that use it all along the game) - Use reference count
                         // FALSE: If during a moment in the game no existing object use this texture (do management yourself) - Do not use refrence count
    float mRed;
    float mGreen;
    float mBlue;
    float mAlpha;
    // Color filters: [0;1]
    // -> Same as 'glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE)' call if all varaibles are eqaul to 1 (default)
    // -> Same as 'glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE)' call if all varaibles are eqaul to 0

#if defined(LIBENG_ENABLE_CAMERA) && !defined(__ANDROID__)
    bool mBGRA; // Set the camera color buffer to BGRA (instead of RGBA)
#endif

    bool mStarted; // FALSE: Need to start object (call 'start' method); TRUE: Need to resume object (call 'resume' method)

public:
    Object2D(bool manage = true);
    virtual ~Object2D();

    static float* TransformBuffer;
    void initialize(const Game2D* game);

    inline unsigned char getTextureIdx() const { return mTextureIdx; }

    inline bool isStarted() const { return mStarted; }

    inline void setRed(float red) { mRed = red; }
    inline void setGreen(float green) { mGreen = green; }
    inline void setBlue(float blue) { mBlue = blue; }
    inline void setAlpha(float alpha) { mAlpha = alpha; }
#if defined(LIBENG_ENABLE_CAMERA) && !defined(__ANDROID__)
    inline void setBGRA(bool bgra) { mBGRA = bgra; }

    inline bool getBGRA() const { return mBGRA; }
#endif
    inline float getRed() const { return mRed; }
    inline float getGreen() const { return mGreen; }
    inline float getBlue() const { return mBlue; }
    inline float getAlpha() const { return mAlpha; }

    //
    inline void pause() { mTextureIdx = TEXTURE_IDX_INVALID; };

    void resume(unsigned char texture);
    void resume(unsigned char red, unsigned char green, unsigned char blue);

    //////
    virtual void start(unsigned char texture);
    virtual void start(unsigned char red, unsigned char green, unsigned char blue);

    virtual void render(bool unused) const;
    // WARNING: To be redefined!
    // -> Without dynamic management add 'Object2D::transform' method call B4 (without 'Dynamic2D' class multiple inheritance)
    // -> With dynamic management add 'Dynamic2D::transform' method call B4 (with 'Dynamic2D' class multiple inheritance)

};

} // namespace

#endif // __cplusplus
#endif // LIBENG_OBJECT2D_H_
