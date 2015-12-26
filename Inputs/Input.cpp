#include <libeng/Inputs/Input.h>

namespace eng {

Inputs* Inputs::mThis = NULL;

//////
Inputs::Inputs() {

#ifdef DEBUG
    LOGV(LIBENG_LOG_INPUT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);

    mLogAdd = 0;
    mLogGet = 0;
    mLogPurge = 0;
#endif
    for (short i = 0; i < MAX_INPUT_COUNT; ++i)
        mInputs[i] = NULL;
}
Inputs::~Inputs() {

    LOGV(LIBENG_LOG_INPUT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    free();
}

void Inputs::free() {

    LOGV(LIBENG_LOG_INPUT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    for (short i = 0; i < MAX_INPUT_COUNT; ++i) {
        if (mInputs[i]) {
            delete mInputs[i];
            mInputs[i] = NULL;
        }
    }
}

} // namespace
