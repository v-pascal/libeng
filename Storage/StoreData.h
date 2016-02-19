#ifndef LIBENG_STOREDATA_H_
#define LIBENG_STOREDATA_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_STORAGE

#include <libeng/Log/Log.h>
#include <assert.h>
#include <string>
#include <map>

namespace eng {

static const wchar_t DATA_STRING = L'0';
static const wchar_t DATA_CHAR = L'1';
static const wchar_t DATA_SHORT = L'2';
static const wchar_t DATA_INT = L'3';

template<class mapT, class valT> bool insert(mapT* &map, const char* key, valT* value) {
    if (!map)
        map = new mapT;
    else if (map->find(key) != map->end()) {

        delete value;
        return false;
    }
    map->insert(std::pair<const std::string, valT*>(key, value)); //map[key] = value;
    return true;
}

typedef std::map<const std::string, std::wstring*> StringMap;
typedef std::map<const std::string, unsigned char*> CharMap;
typedef std::map<const std::string, short*> ShortMap;
typedef std::map<const std::string, int*> IntMap;

class Storage;

//////
class StoreData {

    friend class Storage;

private:
    StringMap* mStringMap; // WARNING: String should not contain any '\t' & '\n' characters (as key string)
    CharMap* mCharMap;
    ShortMap* mShortMap;
    IntMap* mIntMap;

    // WARNING: Key string should not contain any '\t' & '\n' characters!

public:
    StoreData();
    virtual ~StoreData();

    //////
    inline bool add(wchar_t type, const char* key, void* value) { // Return FALSE if already exists

        LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - t:%d; k:%s; v:%p"), __PRETTY_FUNCTION__, __LINE__,
                static_cast<int>(type), (key)? key:"NULL", value);
        assert(key);
        assert(value);

#ifdef DEBUG
        // Check if 'key' string contains only ASCII codes (not extended ASCII codes: < 127)
        const unsigned char* walk = reinterpret_cast<const unsigned char*>(key);

        assert((*walk) < 0x7f); // < 127
        while (*walk++)
            assert((*walk) < 0x7f);
#endif
        switch (type) {

            case DATA_STRING:   return insert<StringMap, std::wstring>(mStringMap, key, static_cast<std::wstring*>(value));
            case DATA_CHAR:     return insert<CharMap, unsigned char>(mCharMap, key, static_cast<unsigned char*>(value));
            case DATA_SHORT:    return insert<ShortMap, short>(mShortMap, key, static_cast<short*>(value));
            case DATA_INT:      return insert<IntMap, int>(mIntMap, key, static_cast<int*>(value));
            default: {

                LOGF(LOG_FORMAT(" - Unknown store data type: %d"), __PRETTY_FUNCTION__, __LINE__, static_cast<int>(type));
                assert(NULL);
                break;
            }
        }
        return false;
    }
    // WARNING: Memory is managed here so do not free values!
    // -> Done in 'remove' or '~DataS' methods, or if this method failed

    bool remove(wchar_t type, const char* key); // Free value
    void* get(wchar_t type, const char* key); // Return NULL when key not found
#ifdef DEBUG
    void display();
#endif

    inline bool isEmpty() const { return ((!mStringMap) && (!mCharMap) && (!mShortMap) && (!mIntMap)); }
    void clear();

};

} // namespace

#endif // LIBENG_ENABLE_STORAGE
#endif // __cplusplus
#endif // LIBENG_STOREDATA_H_
