#ifndef LIBENG_TEXTURES_H_
#define LIBENG_TEXTURES_H_
#if defined(__cplusplus)

#include <libeng/Global.h>

#ifdef __ANDROID__
#include <GLES2/gl2.h>
#else
typedef uint32_t GLuint;
// -> Avoid to include "OpenGLES/ES2/gl.h" in header file
// BUG: 'CVOpenGLESTexture.h' compilation error (from iOS SDK 8.1)

#endif
#include <assert.h>
#include <vector>

#include <libeng/Social/Network.h>

namespace eng {

////// Reserved texture index
static const unsigned char TEXTURE_IDX_INVALID = 0xff; // This causes a maximum texture count to 254 (== 0xff - 1)

////// Reserved texture ID
static const unsigned char TEXTURE_ID_LOGO = 0;
static const unsigned char TEXTURE_ID_FONT = 1;
// Google::TEXTURE_ID = 0xfb
// Twitter::TEXTURE_ID = 0xfc
// Facebook::TEXTURE_ID = 0xfd
static const unsigned char TEXTURE_ID_COLOR = 0xfe;
static const unsigned char TEXTURE_ID_CAM = 0xff;

typedef struct {

    unsigned char* textureBuffer;

    GLuint textureId;
    short width;
    short height;

    short refCount; // Number of objects that use this texture
    unsigned char id;

    // Grayscale color flag (Luminance color: GL_LUMINANCE_ALPHA)
    bool grayscale;

} Texture;

//////
class Textures {

private:
    Textures();
    virtual ~Textures();

    static Textures* mThis;
    std::vector<Texture*> mTextures;

public:
    static Textures* getInstance() {
        if (!mThis)
            mThis = new Textures;
        return mThis;
    }
    static void freeInstance() {
        if (mThis) {
            delete mThis;
            mThis = NULL;
        }
    }
    static unsigned char loadTexture(unsigned char id);

    //
    inline unsigned char getIndex(unsigned char id) const {

        for (unsigned char i = 0; i < static_cast<unsigned char>(mTextures.size()); ++i)
            if (mTextures[i]->id == id)
                return i;

        return TEXTURE_IDX_INVALID;
    }

    //////
    unsigned char addTexture(unsigned char id, short width, short height, unsigned char* data, bool grayscale);
#ifdef LIBENG_ENABLE_SOCIAL
    unsigned char addTexPic(Network::ID id); // Social picture
#endif
    unsigned char addTexColor();
#ifdef LIBENG_ENABLE_CAMERA
    unsigned char addTexCam(short width, short height);
    // WARNING: To be able to change camera preview size you have to remove camera texture (e.g using 'rmvTextures' method)
#endif
    void rmvTextures(unsigned char count = 0);  // Remove texture count from the last to the first entry
                                                // -> Avoid to change texture index order (!count: Remove all textures)

    void genTexture(unsigned char index, bool release = true, bool rgb = false);
    // Load texture into graphic card + Free texture buffer if requested (according 'release' parameter) + update 'refCount'
    // WARNING: When setting the 'release' parameter to FALSE verify that the 'operator[].textureBuffer' field has been previously
    //          stored and will be freed by your own management ('textureBuffer' field is set to NULL after this call)

#ifdef LIBENG_ENABLE_CAMERA
    void updateTexCam(unsigned char index, short width, short height, const unsigned char* camBuffer);
#endif
    void delTexture(unsigned char index); // Remove texture from graphic card according 'refCount' (keep texture entry/indexes)
    void delTextures(); // Remove all texture from graphic card without checking 'refCount' (keep texture entries/indexes)

#ifdef DEBUG
    bool chkTextures() const; // Check correct textures management
#endif
    inline Texture* operator[](unsigned char index) {

        assert(index != TEXTURE_IDX_INVALID);
        assert(index < static_cast<unsigned char>(mTextures.size()));
        return mTextures[index];
    }
    inline const Texture* operator[](unsigned char index) const {

        assert(index != TEXTURE_IDX_INVALID);
        assert(index < static_cast<unsigned char>(mTextures.size()));
        return mTextures[index];
    }

};

} // namespace

#endif // __cplusplus
#endif // LIBENG_TEXTURES_H_
