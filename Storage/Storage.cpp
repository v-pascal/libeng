#include "Storage.h"

#ifdef LIBENG_ENABLE_STORAGE
#include <libeng/Tools/Tools.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#ifdef __ANDROID__
#include <boost/regex.hpp>
#else
#include <regex>
#endif

#define FORMAT_STRING   L"%c\t%s\t%s\n"
#define FORMAT_CHAR     L"%c\t%s\t%d\n"
#define FORMAT_SHORT    FORMAT_CHAR
#define FORMAT_INT      FORMAT_CHAR

namespace eng {

Storage* Storage::mThis = NULL;

template<class T>
void map2str(std::wstring &content, const T* map, const wchar_t* format, const wchar_t type) {
    if (!map)
        return;

    for (typename T::const_iterator iter = map->begin(); iter != map->end(); ++iter)
        content += boost::str(boost::wformat(format) % type % std::wstring(iter->first.begin(),
                iter->first.end()) % (*iter->second));
}

//////
Storage::Storage() : mStatus(STORE_SUCCEEDED) {

    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#ifndef __ANDROID__
    mVideo = nil;
#endif
}
Storage::~Storage() {

    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    for (StringMap::iterator iter = mFileMap.begin(); iter != mFileMap.end(); ++iter)
        delete iter->second;
    mFileMap.clear();

    if (!mStatus) { // == STORE_IN_PROGRESS
        LOGW(LOG_FORMAT(" - Close application without having finish to save video/picture"), __PRETTY_FUNCTION__, __LINE__);
    }
#ifndef __ANDROID__
    else if (mVideo != nil)
        [mVideo release];
#endif
}

void Storage::Data2File(std::wstring &content, const StoreData* data) {

    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - d:%p"), __PRETTY_FUNCTION__, __LINE__, data);

    map2str<StringMap>(content, data->mStringMap, FORMAT_STRING, DATA_STRING);
    //map2str<CharMap>(content, data->mCharMap, FORMAT_CHAR, DATA_CHAR);
    if (data->mCharMap) {

        for (CharMap::const_iterator iter = data->mCharMap->begin(); iter != data->mCharMap->end(); ++iter)
            content += boost::str(boost::wformat(FORMAT_CHAR) % DATA_CHAR % std::wstring(iter->first.begin(),
                    iter->first.end()) % static_cast<int>(*iter->second));
    }
    map2str<ShortMap>(content, data->mShortMap, FORMAT_SHORT, DATA_SHORT);
    map2str<IntMap>(content, data->mIntMap, FORMAT_INT, DATA_INT);
}
bool Storage::File2Data(StoreData* data, const std::wstring* content) {

    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - d:%p; c:%p"), __PRETTY_FUNCTION__, __LINE__, data, content);
    assert(content);
    assert(data);

    if ((content->empty()) || ((*content) == LIBENG_FILE_NOT_FOUND))
        return false;

    try {

        std::wstring::const_iterator start = content->begin();
        std::wstring::const_iterator end = content->end();
        
        data->clear();

#ifdef __ANDROID__
        boost::match_results<std::wstring::const_iterator> what;
        boost::wregex exp(L"^(\\d{1})\t([^\t]*)\t([^\n]*)\n"); // boost::regex::perl
        while (boost::regex_search(start, end, what, exp)) {
#else
        std::match_results<std::wstring::const_iterator> what;
        std::wregex exp(L"^(\\d{1})\t([^\t]*)\t([^\n]*)\n");
        while (std::regex_search(start, end, what, exp)) {
#endif

#ifdef DEBUG
            if (!what[0].matched) {

                LOGW(LOG_FORMAT(" - Data not found"), __PRETTY_FUNCTION__, __LINE__);
                assert(NULL);
            }
#endif
            wchar_t type = static_cast<wchar_t>(*what[1].first);
            const std::string key(what[2].first, what[2].second); // Should only contain ASCII codes (no extended ASCII codes)
            const std::wstring value(what[3].first, what[3].second);

            switch (type) {

            case DATA_STRING:   data->add(type, key.c_str(), static_cast<void*>(new std::wstring(what[3].first, what[3].second))); break;
            case DATA_CHAR:     data->add(type, key.c_str(), static_cast<void*>(new unsigned char(boost::numeric_cast<unsigned char>(boost::lexical_cast<short>(value))))); break;
            case DATA_SHORT:    data->add(type, key.c_str(), static_cast<void*>(new short(boost::lexical_cast<short>(value)))); break;
            case DATA_INT:      data->add(type, key.c_str(), static_cast<void*>(new int(boost::lexical_cast<int>(value)))); break;

                default: {

                    LOGD(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - Unknown store data type: %d"), __PRETTY_FUNCTION__,
                            __LINE__, static_cast<int>(type));
                    assert(NULL);
                    return false;
                }
            }
            start = what[0].second;
        }
        return (start == end);
    }
    catch (const boost::bad_numeric_cast& num) {

        LOGW(LOG_FORMAT(" - Numeric cast error: %s"), __PRETTY_FUNCTION__, __LINE__, num.what());
        assert(NULL);
        return false;
    }
    catch (const boost::bad_lexical_cast& cast) {

        LOGW(LOG_FORMAT(" - Cast error: %s"), __PRETTY_FUNCTION__, __LINE__, cast.what());
        assert(NULL);
        return false;
    }
#ifdef __ANDROID__
    catch (const boost::regex_error& reg) {
#else
    catch (const std::regex_error& reg) {
#endif
        LOGW(LOG_FORMAT(" - Regex error: %s"), __PRETTY_FUNCTION__, __LINE__, reg.what());
        assert(NULL);
        return false;
    }
}

bool Storage::loadFile(const char* file) {

#ifdef __ANDROID__
    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - f:%s (j:%p; a:%p)"), __PRETTY_FUNCTION__, __LINE__, (file)? file:"NULL",
            g_JavaVM, g_ActivityClass);
    assert(g_JavaVM);
    assert(g_ActivityClass);
    assert(file);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_STORAGE, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    jmethodID mthd = env->GetStaticMethodID(g_ActivityClass, "readFILE", "(Ljava/lang/String;)Z");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'readFILE' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    jstring jFile = env->NewStringUTF(file);
    bool res = static_cast<bool>(env->CallStaticBooleanMethod(g_ActivityClass, mthd, jFile));
    env->DeleteLocalRef(jFile);
    return res;

#else
    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - f:%s (a:%p)"), __PRETTY_FUNCTION__, __LINE__, (file)? file:"NULL", g_AppleOS);
    assert(g_AppleOS);
    assert(file);

    return [g_AppleOS readFile:file];
#endif
}
bool Storage::saveFile(const char* file, const StoreData* data) {

    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - f:%s; d:%p"), __PRETTY_FUNCTION__, __LINE__, (file)? file:"NULL", data);
    assert(file);
    assert(data);

    std::wstring strData;
    Data2File(strData, data);
    return saveFile(file, strData.c_str());
}
bool Storage::saveFile(const char* file, const std::wstring& content) {

#ifdef __ANDROID__
    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - f:%s (c:%d; j:%p; a:%p)"), __PRETTY_FUNCTION__, __LINE__, (file)? file:"NULL",
            static_cast<int>(content.size()), g_JavaVM, g_ActivityClass);
    assert(g_JavaVM);
    assert(g_ActivityClass);
    assert(file);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_STORAGE, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    jmethodID mthd = env->GetStaticMethodID(g_ActivityClass, "writeFILE", "(Ljava/lang/String;[BI)Z");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'writeFILE' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }

#ifdef DEBUG
    if (sizeof(jchar) != 2) {
        LOGE(LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
    }
    if (sizeof(wchar_t) != 4) {
        LOGE(LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
    }
#endif
    jbyte* byteContent = wstring2byteArray(content);
    jsize len = static_cast<jsize>(content.size() << 1);

    jbyteArray byteArray = env->NewByteArray(len);
    env->SetByteArrayRegion(byteArray, 0, len, byteContent);
    jstring jFile = env->NewStringUTF(file);
    bool res = static_cast<bool>(env->CallStaticBooleanMethod(g_ActivityClass, mthd, jFile, byteArray, len));

    env->DeleteLocalRef(byteArray);
    env->DeleteLocalRef(jFile);
    delete [] byteContent;
    return res;

#else
    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - f:%s (c:%d; a:%p)"), __PRETTY_FUNCTION__, __LINE__, (file)? file:"NULL",
            static_cast<int>(content.size()), g_AppleOS);
    assert(g_AppleOS);
    assert(file);

    return [g_AppleOS writeFile:file withContent:content.c_str() length:content.size()];
#endif
}

std::string Storage::getFolder(unsigned char type) {

    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - t:%d"), __PRETTY_FUNCTION__, __LINE__, type);
    std::string folder; // Error (empty)

#ifdef __ANDROID__
    LOGI(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - (j:%p; o:%p)"), __PRETTY_FUNCTION__, __LINE__, g_JavaVM, g_ActivityObject);
    assert((type == FOLDER_TYPE_PICTURES) || (type == FOLDER_TYPE_MOVIES) || (type == FOLDER_TYPE_APPLICATION));
    assert(g_JavaVM);
    assert(g_ActivityObject);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_STORAGE, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return folder;

    jclass clss = env->GetObjectClass(g_ActivityObject);
    if (!clss) {

        LOGE(LOG_FORMAT(" - Failed to get activity object class"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return folder;
    }
    jmethodID mthd = env->GetMethodID(clss, "getFolder", "(S)Ljava/lang/String;");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'getFolder' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return folder;
    }

    jstring strFolder = static_cast<jstring>(env->CallObjectMethod(g_ActivityObject, mthd, static_cast<short>(type)));
    if (strFolder) {

        const char* lpFolder = env->GetStringUTFChars(strFolder, 0);
        folder.assign(lpFolder);
        env->ReleaseStringUTFChars(strFolder, lpFolder);

        // Check avoid #JNI1 issue case
        if (folder == " ") folder.clear(); // null
    }
#ifdef DEBUG
    else { // #JNI1 issue found

        LOGF(LOG_FORMAT(" - #JNI1 issue found"), __PRETTY_FUNCTION__, __LINE__);
        //assert(NULL); // Let's #JNI1 crash
        folder.clear();
    }
#endif

#else
    NSSearchPathDirectory dirType;
    switch (type) {

        case FOLDER_TYPE_PICTURES: dirType = NSPicturesDirectory; break;
        case FOLDER_TYPE_MOVIES: dirType = NSMoviesDirectory; break;
        case FOLDER_TYPE_DOCUMENTS: dirType = NSDocumentDirectory; break;
        default: {

            LOGF(LOG_FORMAT(" - Unknown folder type"), __PRETTY_FUNCTION__, __LINE__);
            assert(NULL);
            return folder;
        }
    }
    NSArray* pathList = NSSearchPathForDirectoriesInDomains(dirType, NSUserDomainMask, YES);
    if ([pathList count] == 0) {
        pathList = NSSearchPathForDirectoriesInDomains(dirType, NSLocalDomainMask, YES);
        if ([pathList count] == 0)
            pathList = NSSearchPathForDirectoriesInDomains(dirType, NSAllDomainsMask, YES);
    }
    if ([pathList count] > 0)
        folder.assign([[NSFileManager defaultManager] fileSystemRepresentationWithPath:[pathList objectAtIndex:0]]);
#ifdef DEBUG
    else {

        LOGE(LOG_FORMAT(" - Failed to get '%s' folder"), __PRETTY_FUNCTION__, __LINE__, (type == FOLDER_TYPE_PICTURES)?
             "Pictures":"Movies");
        assert(NULL);
    }
#endif
#endif
    return folder;
}
float Storage::getFreeSpace(const std::string &dir) {

    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - d:%s"), __PRETTY_FUNCTION__, __LINE__, dir.c_str());

#ifdef __ANDROID__
    LOGI(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - (j:%p; a:%p)"), __PRETTY_FUNCTION__, __LINE__, g_JavaVM, g_ActivityClass);
    assert(g_JavaVM);
    assert(g_ActivityClass);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_STORAGE, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return LIBENG_NO_DATA; // Error

    jmethodID mthd = env->GetStaticMethodID(g_ActivityClass, "getFreeSpace", "(Ljava/lang/String;)F");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'getFreeSpace' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return LIBENG_NO_DATA;
    }

    jstring jDirectory = env->NewStringUTF(dir.c_str());
    jfloat jSpace = env->CallStaticFloatMethod(g_ActivityClass, mthd, jDirectory);
    env->DeleteLocalRef(jDirectory);

    LOGI(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - Free space available: %f Bytes"), __PRETTY_FUNCTION__, __LINE__,
         static_cast<float>(jSpace));
    return static_cast<float>(jSpace);

#else
    NSString* filePath = [[NSString alloc] initWithCString:dir.c_str() encoding:NSUTF8StringEncoding];
    NSDictionary* fileInfo = [[NSFileManager defaultManager] attributesOfFileSystemForPath:filePath error:nil];
    if (fileInfo) {

        NSNumber* freeSpace = [fileInfo objectForKey:NSFileSystemFreeSize];
        if (freeSpace) {
            LOGI(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - Free space available: %f Bytes"), __PRETTY_FUNCTION__, __LINE__,
                 [freeSpace floatValue]);
            return [freeSpace floatValue];
        }
    }
    LOGE(LOG_FORMAT(" - Failed to get free space"), __PRETTY_FUNCTION__, __LINE__);
    assert(NULL);
    return LIBENG_NO_DATA;

#endif
}

void Storage::addFile(const char* file, const wchar_t* content) {

    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - f:%s; c:%p"), __PRETTY_FUNCTION__, __LINE__, (file)? file:"NULL", content);
    assert(file);
    assert(content);

    StringMap::const_iterator iter = mFileMap.find(file);
    if (iter != mFileMap.end()) {

        LOGW(LOG_FORMAT(" - File '%s' already loaded"), __PRETTY_FUNCTION__, __LINE__, file);
        assert(NULL);
        return;
    }
    mFileMap[file] = new std::wstring(content);
}
void Storage::rmvFile(const char* file) {

    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - f:%s"), __PRETTY_FUNCTION__, __LINE__, (file)? file:"NULL");
    assert(file);

    StringMap::iterator iter = mFileMap.find(file);
    if (iter != mFileMap.end()) {

        delete iter->second;
        mFileMap.erase(iter);
    }
}

#ifdef __ANDROID__
bool Storage::savePicture(const std::string& file, bool alert, short width, short height, unsigned char* rgba) {

    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - f:%s; a:%s; w:%d; h:%d; r:%p"), __PRETTY_FUNCTION__, __LINE__, file.c_str(),
             (alert)? "true":"false", width, height, rgba);
    assert(g_JavaVM);
    assert(g_ActivityObject);

    if (!mStatus) { // == STORE_IN_PROGRESS

        LOGW(LOG_FORMAT(" - Failed to save picture due to an existing processus"), __PRETTY_FUNCTION__, __LINE__);
        return false;
    }

    JNIEnv* env = getJavaEnv(LIBENG_LOG_STORAGE, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    jclass clss = env->GetObjectClass(g_ActivityObject);
    if (!clss) {

        LOGE(LOG_FORMAT(" - Failed to get activity object class"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    jmethodID mthd = env->GetMethodID(clss, "saveBMP", "(Ljava/lang/String;ZSS[B)Z");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'saveBMP' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    jsize len = (width * height) << 2;
    jbyteArray picArray = env->NewByteArray(len);
    env->SetByteArrayRegion(picArray, 0, len, reinterpret_cast<const jbyte*>(rgba));

    mStatus = STORE_IN_PROGRESS;
    jstring jFile = env->NewStringUTF(file.c_str());
    if (env->CallBooleanMethod(g_ActivityObject, mthd, jFile, alert, width, height, picArray)) {

        env->DeleteLocalRef(picArray);
        env->DeleteLocalRef(jFile);
        return true;
    }
    env->DeleteLocalRef(picArray);
    env->DeleteLocalRef(jFile);

    mStatus = STORE_FAILED;
    return false;
}
bool Storage::saveMedia(const std::string& file, const std::string& type, const std::string& title) {

    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - f:%s; t:%s; t:%s (j:%p; a:%p)"), __PRETTY_FUNCTION__, __LINE__, file.c_str(),
            type.c_str(), title.c_str(), g_JavaVM, g_ActivityObject);
    assert(g_JavaVM);
    assert(g_ActivityObject);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_STORAGE, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    jclass clss = env->GetObjectClass(g_ActivityObject);
    if (!clss) {

        LOGE(LOG_FORMAT(" - Failed to get activity object class"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    jmethodID mthd = env->GetMethodID(clss, "saveMedia", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Landroid/net/Uri;");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'saveMedia' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    jstring jFile = env->NewStringUTF(file.c_str());
    jstring jType = env->NewStringUTF(type.c_str());
    jstring jTitle = env->NewStringUTF(title.c_str());
    return (env->CallObjectMethod(g_ActivityObject, mthd, jFile, jType, jTitle) != NULL);
}
#else
void Storage::savePicture(unsigned char picFormat, short width, short height, unsigned char* buffer) {

    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - p:%d; w:%d; h:%d; r:%p"), __PRETTY_FUNCTION__, __LINE__, picFormat, width, height,
         buffer);
    mStatus = STORE_IN_PROGRESS;
    saveBitmap(picFormat, &mStatus, width, height, buffer);
}
void Storage::saveVideo(const char* file, const char* msgDone, const char* msgFailed, double duration) {

    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - f:%s; d:%p (%s); m:%p (%s); d:%f"), __PRETTY_FUNCTION__, __LINE__, file, msgDone,
         (msgDone)? msgDone:"null", (msgFailed)? msgFailed:"null", static_cast<float>(duration));
    mStatus = STORE_IN_PROGRESS;

    if (mVideo != nil)
        [mVideo release];

    mVideo = [[StoreVideo alloc] init];
    [mVideo save:file withOkMsg:msgDone withBadMsg:msgFailed andDuration:duration error:&mStatus];
}
#endif

} // namespace

#endif // LIBENG_ENABLE_STORAGE
