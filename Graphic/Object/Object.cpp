#include "Object.h"
#include <stddef.h>

namespace eng {

//////
Object::Object() : mVertices(NULL), mScreen(NULL) {

    LOGV(LIBENG_LOG_OBJECT, 0, LOG_FORMAT(" - (t:%p)"), __PRETTY_FUNCTION__, __LINE__, FULL_TEXCOORD_BUFFER);

    mTexCoords = const_cast<float*>(FULL_TEXCOORD_BUFFER);
#ifdef DEBUG
    mLog = NULL;
#endif
}
Object::~Object() {

    LOGV(LIBENG_LOG_OBJECT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    if ((mTexCoords) && (mTexCoords != FULL_TEXCOORD_BUFFER)) {

        LOGI(LIBENG_LOG_OBJECT, 0, LOG_FORMAT(" - Delete texture coords buffer: %p"), __PRETTY_FUNCTION__, __LINE__,
                mTexCoords);
        delete [] mTexCoords;
    }
}

bool Object::isFullCoordsBuffer() const { return (mTexCoords == FULL_TEXCOORD_BUFFER); }

} // namespace
