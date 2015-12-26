#ifndef LIBENG_STORAGE_H_
#define LIBENG_STORAGE_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_STORAGE

#include <libeng/Log/Log.h>
#include <libeng/Storage/StoreData.h>
#include <libeng/Storage/StoreMedia.h>

namespace eng {

static const unsigned char FOLDER_TYPE_PICTURES = 1;
static const unsigned char FOLDER_TYPE_MOVIES = 2;
#ifndef __ANDROID__
static const unsigned char FOLDER_TYPE_DOCUMENTS = 3;
#else
static const unsigned char FOLDER_TYPE_APPLICATION = 3;
#endif

//////
class Storage {

private:
    Storage();
    virtual ~Storage();

    static Storage* mThis;

public:
    static Storage* getInstance() {
        if (!mThis)
            mThis = new Storage;
        return mThis;
    }
    static void freeInstance() {
        if (mThis) {
            delete mThis;
            mThis = NULL;
        }
    }

private:
    StringMap mFileMap;
    static void Data2File(std::wstring &content, const StoreData* data);

    unsigned char mStatus; // Storage status

public:
    static bool File2Data(StoreData* data, const std::wstring* content);
    // WARNING: Modify 'data' even if failed (return FALSE)!

    static bool loadFile(const char* file);
    static bool saveFile(const char* file, const StoreData* data);

    static bool saveFile(const char* file, const std::wstring& content);
    // Common use: Storage::saveFile("file", mStorage["file"]->c_str());
    // -> After having change 'mStorages["file"]' value (if needed)

    static std::string getFolder(unsigned char type);
    static float getFreeSpace(const std::string &dir);

    inline unsigned char getStatus() const { return mStatus; }
    inline void setStatus(unsigned char result) { mStatus = result; }

    //////
    void addFile(const char* file, const wchar_t* content);
    void rmvFile(const char* file);

#ifdef __ANDROID__
    bool savePicture(const std::string& file, bool alert, short width, short height, unsigned char* rgba);
    bool saveMedia(const std::string& file, const std::string& type, const std::string& title);

#else
    void savePicture(unsigned char picFormat, short width, short height, unsigned char* buffer);

    StoreVideo* mVideo;
    void saveVideo(const char* file, const char* msgDone, const char* msgFailed, double duration);
#endif

    inline bool isEmpty(const char* file) const { // Return TRUE if not exists

        LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - f:%s"), __PRETTY_FUNCTION__, __LINE__, file);
        assert(file);

        StringMap::const_iterator iter = mFileMap.find(file);
        return (iter != mFileMap.end())? ((*iter->second) == LIBENG_FILE_NOT_FOUND):true;
    }

    //
    inline std::wstring* operator[](const char* file) {

        assert(file);
        StringMap::iterator iter = mFileMap.find(file);
        return (iter != mFileMap.end())? iter->second:NULL;
    }
    inline const std::wstring* operator[](const char* file) const {

        assert(file);
        StringMap::const_iterator iter = mFileMap.find(file);
        return (iter != mFileMap.end())? iter->second:NULL;
    }

};

} // namespace

#endif // LIBENG_ENABLE_STORAGE
#endif // __cplusplus
#endif // LIBENG_STORAGE_H_
