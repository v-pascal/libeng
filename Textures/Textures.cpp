#include "Textures.h"

#ifndef __ANDROID__
#include "OpenGLES/ES2/gl.h"
#endif

#include <libeng/Tools/Tools.h>
#include <libeng/Log/Log.h>
#include <cstring>

#include <libeng/Social/Social.h>

#include <libeng/Social/Networks/Facebook.h>
#include <libeng/Social/Networks/Twitter.h>
#include <libeng/Social/Networks/Google.h>

#define TEX_COLOR_SIZE      16 // Texture color size

namespace eng {

Textures* Textures::mThis = NULL;

inline short getTwoPower(short resolution, short texture) {

    if (resolution < texture)
        return texture;

    return getTwoPower(resolution, texture * 2);
}

//////
Textures::Textures() { LOGV(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }
Textures::~Textures() {

    LOGV(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);

    delTextures();
    rmvTextures();
    // WARNING: Code lines just above are not needed coz only useful when there is a bad texture management. If so these
    //          lines make an OpenGL error coz at that time there is no more OpenGL context (keep lines to see this error).
}

unsigned char Textures::loadTexture(unsigned char id) {

#ifdef __ANDROID__
    LOGV(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(" - i:%d (j:%p; a:%p)"), __PRETTY_FUNCTION__, __LINE__, id, g_JavaVM,
            g_ActivityClass);
    assert(g_JavaVM);
    assert(g_ActivityClass);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_TEXTURE, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return TEXTURE_IDX_INVALID;

    jmethodID mthd = env->GetStaticMethodID(g_ActivityClass, "loadPNG", "(S)S");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'loadPNG' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return TEXTURE_IDX_INVALID;
    }
    return static_cast<unsigned char>(env->CallStaticShortMethod(g_ActivityClass, mthd, static_cast<short>(id)));
    // NOTE: 'char' in Java is coded on 2 bytes (No need to use a 'char' type in Java)

#else
    LOGV(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(" - i:%d (a:%p)"), __PRETTY_FUNCTION__, __LINE__, id, g_AppleOS);
    assert(g_AppleOS);

    return [g_AppleOS loadTexture:id];
#endif
}

unsigned char Textures::addTexture(unsigned char id, short width, short height, unsigned char* data, bool grayscale) {

#ifdef DEBUG
    LOGV(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(" - i:%d; w:%d; h:%d; d:%p; g:%s"), __PRETTY_FUNCTION__, __LINE__, id, width,
            height, data, (grayscale)? "true":"false");
    assert(data);

    if (getIndex(id) != TEXTURE_IDX_INVALID) { // Should not already exists!

        LOGW(LOG_FORMAT(" - Texture ID %d already loaded"), __PRETTY_FUNCTION__, __LINE__, id);
        assert(NULL);
    }
#endif
    Texture* texture = new Texture;

    texture->textureBuffer = data;
    texture->width = width;
    texture->height = height;

    texture->textureId = 0;

    texture->id = id;
    texture->refCount = 0;
    texture->grayscale = grayscale;

    mTextures.push_back(texture);
    return static_cast<unsigned char>(mTextures.size() - 1);
}
#ifdef LIBENG_ENABLE_SOCIAL
unsigned char Textures::addTexPic(Network::ID id) {

    LOGV(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(" - i:%d"), __PRETTY_FUNCTION__, __LINE__, id);
    assert(Social::mThis);
    assert(Social::mThis->_getSession(id));
    assert(Social::mThis->_getSession(id)->getPicWidth());
    assert(Social::mThis->_getSession(id)->getPicHeight());
    assert(Social::mThis->_getSession(id)->getUserPic());

    Social::mThis->getSession(id)->lock();
    if (!Social::mThis->_getSession(id)->getUserPic()) {

        LOGW(LOG_FORMAT(" - Empty picture buffer"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);

        Social::mThis->getSession(id)->unlock();
        return TEXTURE_IDX_INVALID;
    }
    unsigned char textureIdx = TEXTURE_IDX_INVALID;
    switch (id) {

        case Network::FACEBOOK: textureIdx = getIndex(Facebook::TEXTURE_ID); break;
        case Network::TWITTER: textureIdx = getIndex(Twitter::TEXTURE_ID); break;
        case Network::GOOGLE: textureIdx = getIndex(Google::TEXTURE_ID); break;
    }
    if (textureIdx == TEXTURE_IDX_INVALID) {

        LOGE(LOG_FORMAT(" - Default social texture ID %d not loaded"), __PRETTY_FUNCTION__, __LINE__, id);
        assert(NULL);
        return TEXTURE_IDX_INVALID; // Should exists!
    }
    if (mTextures[textureIdx]->refCount) { // Texture has been generated

        delTexture(textureIdx);
        if (mTextures[textureIdx]->refCount) {

            LOGE(LOG_FORMAT(" - Default social texture ID %d should not be managed"), __PRETTY_FUNCTION__, __LINE__, id);
            assert(NULL);

            ++mTextures[textureIdx]->refCount;
            return TEXTURE_IDX_INVALID;
        }
        if (mTextures[textureIdx]->textureBuffer) {

            LOGE(LOG_FORMAT(" - The buffer of default social texture ID %d is not empty"), __PRETTY_FUNCTION__,
                    __LINE__, id);
            assert(NULL);
            return TEXTURE_IDX_INVALID;
        }
    }
    else { // Texture loaded but not generated

        assert(mTextures[textureIdx]->textureBuffer);
        delete [] mTextures[textureIdx]->textureBuffer;
    }
    mTextures[textureIdx]->grayscale = false;

    short width = Social::mThis->_getSession(id)->getPicWidth();
    short height = Social::mThis->_getSession(id)->getPicHeight();

    // Generate 2 power texture according social picture resolution (64 texels)
    mTextures[textureIdx]->width = getTwoPower(width, 128);
    mTextures[textureIdx]->height = getTwoPower(height, 128);

    int texSize = mTextures[textureIdx]->width * mTextures[textureIdx]->width * 4;
    mTextures[textureIdx]->textureBuffer = new unsigned char[texSize];
    std::memset(mTextures[textureIdx]->textureBuffer, 0, texSize);

    genTexture(textureIdx); // The social texture is managed here

    // Bind & Update the picture texture object
    glBindTexture(GL_TEXTURE_2D, mTextures[textureIdx]->textureId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE,
            Social::mThis->_getSession(id)->getUserPic());
    Social::mThis->getSession(id)->unlock();

    return textureIdx;
}
#endif
unsigned char Textures::addTexColor() {

    LOGV(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    assert(getIndex(TEXTURE_ID_COLOR) == TEXTURE_IDX_INVALID);

    Texture* texture = new Texture;

    texture->textureBuffer = new unsigned char[TEX_COLOR_SIZE * TEX_COLOR_SIZE * 2];
    for (short i = 0; i < (TEX_COLOR_SIZE * TEX_COLOR_SIZE * 2); i += 2) {

        texture->textureBuffer[i + 0] = 0xff; // Luminance (white)
        texture->textureBuffer[i + 1] = 0xff; // Alpha
    }
    texture->width = TEX_COLOR_SIZE;
    texture->height = TEX_COLOR_SIZE;

    texture->textureId = 0;

    texture->id = TEXTURE_ID_COLOR;
    texture->refCount = 0;
    texture->grayscale = true;

    mTextures.push_back(texture);
    return static_cast<unsigned char>(mTextures.size() - 1);
}
#ifdef LIBENG_ENABLE_CAMERA
unsigned char Textures::addTexCam(short width, short height) {

    LOGV(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(" - w:%d; h:%d"), __PRETTY_FUNCTION__, __LINE__, width, height);

    unsigned char textureIdx = getIndex(TEXTURE_ID_CAM);
    if (textureIdx == TEXTURE_IDX_INVALID) { // Create

        LOGI(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(" - Create camera texture"), __PRETTY_FUNCTION__, __LINE__);
        Texture* texture = new Texture;

        // Generate 2 power texture according camera resolution (64 texels)
        texture->width = getTwoPower(width, 128);
        texture->height = getTwoPower(height, 128);

        texture->textureId = 0;

        texture->id = TEXTURE_ID_CAM;
        texture->refCount = 0;
        texture->grayscale = false;

        // Create the camera texture buffer
        int camTexSize = texture->width * texture->height * 4;
        texture->textureBuffer = new unsigned char[camTexSize];
        std::memset(texture->textureBuffer, 0, camTexSize);

        mTextures.push_back(texture);
        textureIdx = static_cast<unsigned char>(mTextures.size() - 1);
    }
    else { // Restore

        LOGI(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(" - Restore camera texture (idx:%d)"), __PRETTY_FUNCTION__, __LINE__,
                textureIdx);
        assert(!mTextures[textureIdx]->refCount); // No need to restore to be able to generate it if not deleted
        assert(!mTextures[textureIdx]->textureBuffer);

        assert(mTextures[textureIdx]->width == getTwoPower(width, 128));
        assert(mTextures[textureIdx]->height == getTwoPower(height, 128));
        // WARNING: To be able to change camera preview size you have to remove camera texture (to be created as in above)

        mTextures[textureIdx]->textureId = 0;

        int camTexSize = mTextures[textureIdx]->width * mTextures[textureIdx]->height * 4;
        mTextures[textureIdx]->textureBuffer = new unsigned char[camTexSize];
        std::memset(mTextures[textureIdx]->textureBuffer, 0, camTexSize);
    }
    return textureIdx;
}
#endif

void Textures::rmvTextures(unsigned char count) {

    LOGV(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(" - c:%d"), __PRETTY_FUNCTION__, __LINE__, count);
    assert(count <= static_cast<unsigned char>(mTextures.size()));

    if (!count) // Remove all texture entries (if any)
        count = static_cast<unsigned char>(mTextures.size());

    while (count--) {

        std::vector<Texture*>::reverse_iterator iter = mTextures.rbegin();
        assert(!(*iter)->refCount);

        LOGI(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(" - Removing texture id: %d"), __PRETTY_FUNCTION__, __LINE__,
                (*iter)->id);
        if ((*iter)->textureBuffer) {

            LOGW(LOG_FORMAT(" - Remove texture without having been used (id: %d)"), __PRETTY_FUNCTION__, __LINE__,
                    (*iter)->id);
            delete [] (*iter)->textureBuffer;
        }
        delete (*iter);
        mTextures.pop_back();
    }

    if (!mTextures.size()) // No more entry
        mTextures.clear();
}

void Textures::genTexture(unsigned char index, bool release, bool rgb) {

    LOGV(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(" - i:%d; r:%s; r:%s (s:%u)"), __PRETTY_FUNCTION__, __LINE__, index,
            (release)? "true":"false", (rgb)? "true":"false", static_cast<unsigned int>(mTextures.size()));
    assert(index != TEXTURE_IDX_INVALID);
    assert(index < static_cast<unsigned char>(mTextures.size()));

    if (!mTextures[index]->refCount) {

        // Generate texture object
        glGenTextures(1, &mTextures[index]->textureId);
        LOGI(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(" - Texture %d generated (%u)"), __PRETTY_FUNCTION__, __LINE__, index,
                mTextures[index]->textureId);

        // Bind the texture object
        glBindTexture(GL_TEXTURE_2D, mTextures[index]->textureId);

        // Set the filtering mode
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Load the texture
        assert(mTextures[index]->textureBuffer);
        if (mTextures[index]->grayscale) { // Luminance

            LOGI(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(" - Using luminance color"), __PRETTY_FUNCTION__, __LINE__);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, mTextures[index]->width, mTextures[index]->height, 0,
                    GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, mTextures[index]->textureBuffer);
        }
        else if (!rgb) { // RGBA

            LOGI(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(" - Using RGBA color"), __PRETTY_FUNCTION__, __LINE__);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mTextures[index]->width, mTextures[index]->height, 0, GL_RGBA,
                    GL_UNSIGNED_BYTE, mTextures[index]->textureBuffer);
        }
        else { // RGB

            LOGI(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(" - Using RGB color"), __PRETTY_FUNCTION__, __LINE__);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mTextures[index]->width, mTextures[index]->height, 0, GL_RGB,
                    GL_UNSIGNED_BYTE, mTextures[index]->textureBuffer);
        }
        // Delete buffer (if requested)
        if (release)
            delete [] mTextures[index]->textureBuffer;
        mTextures[index]->textureBuffer = NULL; // No more buffer reference (avoid to be freed at 'rmvTextures' call)
    }

    ++mTextures[index]->refCount;
    LOGI(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(" - Texture ID %d reference count: %d"), __PRETTY_FUNCTION__, __LINE__,
            mTextures[index]->id, mTextures[index]->refCount);
}
#ifdef LIBENG_ENABLE_CAMERA
void Textures::updateTexCam(unsigned char index, short width, short height, const unsigned char* camBuffer) {

#ifdef DEBUG
    if (index == TEXTURE_IDX_INVALID) {
        LOGE(LOG_FORMAT(" - Invalid index (TEXTURE_IDX_INVALID)"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
    }
    if (index >= static_cast<unsigned char>(mTextures.size())) {
        LOGE(LOG_FORMAT(" - Invalid index (>=)"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
    }
    if (getIndex(TEXTURE_ID_CAM) != index) {
        LOGE(LOG_FORMAT(" - Not the camera texture index"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
    }
    if (!camBuffer) {
        LOGE(LOG_FORMAT(" - Missing 'camBuffer' (NULL)"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
    }
#endif
    // Bind the camera texture object
    glBindTexture(GL_TEXTURE_2D, mTextures[index]->textureId);

    // Update camera texture
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, camBuffer);
}
#endif

void Textures::delTexture(unsigned char index) {

    LOGV(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(" - i:%d (s:%d)"), __PRETTY_FUNCTION__, __LINE__, index,
            static_cast<unsigned int>(mTextures.size()));
    assert(index != TEXTURE_IDX_INVALID);
    assert(index < static_cast<unsigned char>(mTextures.size()));

    LOGI(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(" - Texture ID %d reference count: %d"), __PRETTY_FUNCTION__, __LINE__,
            mTextures[index]->id, mTextures[index]->refCount);
    if (mTextures[index]->refCount)
        --mTextures[index]->refCount;

    if (!mTextures[index]->refCount) {

        glDeleteTextures(1, &mTextures[index]->textureId);
        mTextures[index]->textureId = 0;

        LOGI(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(" - Texture ID %d deleted"), __PRETTY_FUNCTION__, __LINE__,
                mTextures[index]->id);
    }
}
void Textures::delTextures() {

    LOGV(LIBENG_LOG_TEXTURE, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);

    // Force to delete all OpenGL texture objects & reset reference counter
    for (unsigned char i = 0; i < static_cast<unsigned char>(mTextures.size()); ++i) {
        if (mTextures[i]->refCount) {

            mTextures[i]->refCount = 0;
            glDeleteTextures(1, &mTextures[i]->textureId);
        }
    }
}

#ifdef DEBUG
bool Textures::chkTextures() const {

    for (unsigned char i = 0; i < static_cast<unsigned char>(mTextures.size()); ++i) {
        if ((mTextures[i]->id != TEXTURE_ID_CAM) && (mTextures[i]->refCount))
            return false; // Bad textures management
    }
    return true; // Good textures management
}
#endif

} // namespace
