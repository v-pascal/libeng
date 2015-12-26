#include "StoreData.h"

#ifdef LIBENG_ENABLE_STORAGE
namespace eng {

//////
StoreData::StoreData() : mStringMap(NULL), mCharMap(NULL), mShortMap(NULL), mIntMap(NULL) {
    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
}
StoreData::~StoreData() {

    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    clear();
}

template<class T> inline bool rmv(T* map, const char* key) {
    if (!map)
        return false;

    typename T::iterator iter = map->find(key);
    if (iter == map->end())
        return false;

    delete iter->second;
    map->erase(iter);
    return true;
}
bool StoreData::remove(wchar_t type, const char* key) {

    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - t:%d; k:%s"), __PRETTY_FUNCTION__, __LINE__, static_cast<int>(type),
            (key)? key:"NULL");
    switch (type) {

        case DATA_STRING:   return rmv<StringMap>(mStringMap, key);
        case DATA_CHAR:     return rmv<CharMap>(mCharMap, key);
        case DATA_SHORT:    return rmv<ShortMap>(mShortMap, key);
        case DATA_INT:      return rmv<IntMap>(mIntMap, key);
#ifdef DEBUG
        default: {

            LOGD(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - Unknown store data type: %d"), __PRETTY_FUNCTION__, __LINE__,
                    static_cast<int>(type));
            assert(NULL);
            break;
        }
#endif
    }
    return false;
}

template<class mapT, class valT> inline valT* ret(mapT* map, const char* key) {
    if (!map)
        return NULL;

    typename mapT::iterator iter = map->find(key);
    if (iter != map->end())
        return iter->second;

    return NULL;
}
void* StoreData::get(wchar_t type, const char* key) {

    switch (type) {

        case DATA_STRING:   return static_cast<void*>(ret<StringMap, std::wstring>(mStringMap, key));
        case DATA_CHAR:     return static_cast<void*>(ret<CharMap, unsigned char>(mCharMap, key));
        case DATA_SHORT:    return static_cast<void*>(ret<ShortMap, short>(mShortMap, key));
        case DATA_INT:      return static_cast<void*>(ret<IntMap, int>(mIntMap, key));
#ifdef DEBUG
        default: {

            LOGD(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - Unknown store data type: %d"), __PRETTY_FUNCTION__, __LINE__,
                    static_cast<int>(type));
            assert(NULL);
            break;
        }
#endif
    }
    return NULL;
}
#ifdef DEBUG
#define LOG_FORMAT_STRING   "type:%d Key:'%s' Value:'%s...(%u)'"
#define LOG_FORMAT_CHAR     "type:%d Key:'%s' Value:%d"
#define LOG_FORMAT_SHORT    LOG_FORMAT_CHAR
#define LOG_FORMAT_INT      LOG_FORMAT_CHAR

template<class T> inline void log(T* map, const char* format, wchar_t type) {
    if (!map)
        return;

    for (typename T::const_iterator iter = map->begin(); iter != map->end(); ++iter) {
        LOGI(1, 0, format, static_cast<int>(type), iter->first.c_str(), (*iter->second));
    }
}

void StoreData::display() {

    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    if (isEmpty()) {

        LOGI(1, 0, "-> StoreData object is empty", NULL);
        return;
    }

    //log<StringMap>(mStringMap, LOG_FORMAT_STRING, DATA_STRING);
    if (mStringMap) {

        for (StringMap::const_iterator iter = mStringMap->begin(); iter != mStringMap->end(); ++iter) {
            LOGI(1, 0, LOG_FORMAT_STRING, static_cast<int>(DATA_STRING), iter->first.c_str(), iter->second->c_str(),
                    static_cast<unsigned int>(iter->second->size()));
        }
    }
    log<CharMap>(mCharMap, LOG_FORMAT_CHAR, DATA_CHAR);
    log<ShortMap>(mShortMap, LOG_FORMAT_SHORT, DATA_SHORT);
    log<IntMap>(mIntMap, LOG_FORMAT_INT, DATA_INT);
}
#endif

template<class T> void clean(T* &map) {
    if (!map)
        return;

    for (typename T::iterator iter = map->begin(); iter != map->end(); ++iter)
        delete iter->second;
    map->clear();

    delete map;
    map = NULL;
}
void StoreData::clear() {

    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);

    clean<StringMap>(mStringMap);
    clean<CharMap>(mCharMap);
    clean<ShortMap>(mShortMap);
    clean<IntMap>(mIntMap);
}

} // namespace

#endif // LIBENG_ENABLE_STORAGE
