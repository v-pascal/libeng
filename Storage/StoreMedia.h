#ifndef LIBENG_STOREMEDIA_H_
#define LIBENG_STOREMEDIA_H_

enum { // Storage status

    STORE_IN_PROGRESS = 0,

    STORE_SUCCEEDED,
    STORE_FAILED,
    STORE_ALREADY_EXISTS,
    STORE_CREATE_FOLDER,
    STORE_WRONG_FORMAT // Video
};

#ifndef __ANDROID__
#import <Foundation/Foundation.h>

enum { // Picture format

    PIC_FORMAT_RGB = 0,
    PIC_FORMAT_RGBA,
    PIC_FORMAT_BGR,
    PIC_FORMAT_BGRA
};

void saveBitmap(unsigned char format, unsigned char* status, short width, short height, const UInt8* buffer);

//////
@interface StoreVideo : NSObject {

    unsigned char* status;
    NSString* path;

    double duration;
    NSString* msgOk;
    NSString* msgBad;
}
-(void)save:(const char*)file withOkMsg:(const char*)done withBadMsg:(const char*)failed andDuration:(double)delay
      error:(unsigned char*)err;
-(void)video:(NSString*)videoPath didFinishSavingWithError:(NSError*)error contextInfo:(void*)contextInfo;

@end

#endif
#endif // LIBENG_STOREMEDIA_H_
