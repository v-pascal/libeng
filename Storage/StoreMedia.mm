//
//  StoreMedia.mm
//  libeng
//
//  Created by Pascal Viguié on 02/10/2014.
//  Copyright (c) 2014 Pascal Viguié. All rights reserved.
//

#import "StoreMedia.h"
#include <libeng/Global.h>
#include <libeng/Tools/Tools.h>
#include <libeng/Log/Log.h>

#import <CoreGraphics/CGColorSpace.h>
#import <CoreGraphics/CGImage.h>
#import <CoreFoundation/CFData.h>

#import <UIKit/UIImage.h>
#import <UIKit/UIKit.h>


void saveBitmap(unsigned char format, unsigned char* status, short width, short height, const UInt8* buffer) {

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{

        UInt8* rgbBuffer;
        switch (format) {
            case PIC_FORMAT_BGR:
            case PIC_FORMAT_BGRA: {

                *status = STORE_FAILED;
                NSLog(@"FATAL: Picture format not implemented yet (line:%d)", __LINE__);
                assert(NULL);
                return;
            }
            case PIC_FORMAT_RGB: rgbBuffer = const_cast<UInt8*>(buffer); break;
            case PIC_FORMAT_RGBA: {

                rgbBuffer = new UInt8[width * height * 3];

                unsigned int len = width * height * 4;
                for (unsigned int i = 0, j = 0; i < len; i += 4, j+= 3) {

                    rgbBuffer[j + 0] = buffer[i + 0];
                    rgbBuffer[j + 1] = buffer[i + 1];
                    rgbBuffer[j + 2] = buffer[i + 2];
                }
                break;
            }
        }
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        CFDataRef rgbData = CFDataCreate(NULL, rgbBuffer, width * height * 3);
        CGDataProviderRef provider = CGDataProviderCreateWithCFData(rgbData);
        CGImageRef rgbImageRef = CGImageCreate(width, height, 8, 24, width * 3, colorSpace,
                                        kCGBitmapByteOrderDefault, provider, NULL, true, kCGRenderingIntentDefault);
        CFRelease(rgbData);
        CGDataProviderRelease(provider);
        CGColorSpaceRelease(colorSpace);

        UIImage* picImage = [[UIImage alloc] initWithCGImage:rgbImageRef];
        UIImageWriteToSavedPhotosAlbum(picImage, nil, nil, nil);

        [picImage release];
        CGImageRelease(rgbImageRef);
        switch (format) {
            //case PIC_FORMAT_RGB: break;
            case PIC_FORMAT_RGBA: {

                delete [] rgbBuffer;
                break;
            }
            //case PIC_FORMAT_BGR:
            //case PIC_FORMAT_BGRA:
        }
        *status = STORE_SUCCEEDED;
    });
}

//////
@implementation StoreVideo

-(void)save:(const char*)file withOkMsg:(const char*)done withBadMsg:(const char*)failed andDuration:(double)delay
      error:(unsigned char*)err {

    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - f:%s; d:%x (%s); m:%x (%s); d:%f; e:%x"), __PRETTY_FUNCTION__, __LINE__, file, done,
         (done)? done:"null", (failed)? failed:"null", static_cast<float>(delay), err);
    assert(file);

    status = err;
    duration = delay;
    msgOk = (done)? [[NSString alloc] initWithCString:done encoding:NSUTF8StringEncoding]:nil;
    msgBad = (failed)? [[NSString alloc] initWithCString:failed encoding:NSUTF8StringEncoding]:nil;
    path = [[NSString alloc] initWithCString:file encoding:NSUTF8StringEncoding];
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{

        //if (UIVideoAtPathIsCompatibleWithSavedPhotosAlbum(path))
        UISaveVideoAtPathToSavedPhotosAlbum(path, self, @selector(video:didFinishSavingWithError:contextInfo:), nil);

        //else {

        //    LOGW(LOG_FORMAT(" - Video '%s' file format not supported"), __PRETTY_FUNCTION__, __LINE__, [path UTF8String]);
        //    if (msgBad != nil)
        //        eng::alertMessage(LIBENG_LOG_STORAGE, duration, [msgBad UTF8String]);
        //    *status = STORE_WRONG_FORMAT;
        //}
    });
}
-(void)video:(NSString*)videoPath didFinishSavingWithError:(NSError*)error contextInfo:(void*)contextInfo {

    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - v:%s; e:%x"), __PRETTY_FUNCTION__, __LINE__, [videoPath UTF8String], error);
    if (error) {

        LOGW(LOG_FORMAT(" - Failed to save '%s' video (err:%s)"), __PRETTY_FUNCTION__, __LINE__, [videoPath UTF8String],
             [[error description] UTF8String]);
        //assert(NULL);
        *status = STORE_FAILED;
        if (msgBad != nil)
            eng::alertMessage(LIBENG_LOG_STORAGE, duration, [msgBad UTF8String]);
        return;
    }
    *status = STORE_SUCCEEDED;
    if (msgOk != nil)
        eng::alertMessage(LIBENG_LOG_STORAGE, duration, [msgOk UTF8String]);
}

-(void)dealloc {

    if (msgOk != nil)
        [msgOk release];
    if (msgBad != nil)
        [msgBad release];
    if (path != nil)
        [path release];

    [super dealloc];
}
@end
