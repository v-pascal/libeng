#ifndef LIBENG_APPLEOS_H_
#define LIBENG_APPLEOS_H_
#ifndef __ANDROID__

#import <Foundation/Foundation.h>
#include <cstring>

//////
#ifdef LIBENG_ENABLE_SOCIAL
@protocol SocialOS

@required
-(bool)login;
-(void)logout;
-(bool)isLogged;

-(bool)getUserInfo;
-(bool)getUserPicture;

@property(atomic) bool displayError;
@property(nonatomic, strong) NSString* userID; // Unique identifier (ASCII)

@property(nonatomic, strong) NSString* userName; // UTF16
@property(atomic) unsigned char userGender; // See gender enum in 'Social/Network.h'
@property(nonatomic, strong) NSDate* userBirthday;
@property(nonatomic, strong) NSString* userLocation; // UTF16

-(bool)shareLink:(NSArray*)data;
-(bool)shareVideo:(NSArray*)data;

-(void)resume;
-(void)terminate;
-(void)remove; // Release

@end
#endif

//////
@protocol LibengOS

@required
-(unsigned char)loadTexture:(unsigned char)Id;
-(void)loadSound:(unsigned char)Id;

-(bool)readFile:(const char*)file;
-(bool)writeFile:(const char*)file withContent:(const wchar_t*)content length:(size_t)len;

#ifdef LIBENG_ENABLE_CAMERA
-(bool)startCamera:(short)width andHeight:(short)height;
-(bool)stopCamera;
#endif

#ifdef LIBENG_ENABLE_MIC
-(bool)startMic;
-(void)stopMic;

-(void)initMicRecorder:(NSString*)file withFormat:(int)format withSampleRate:(float)rate
                                withNumChannels:(int)channels;
-(BOOL)startMicRecorder;
-(BOOL)stopMicRecorder;
#endif

#ifdef LIBENG_ENABLE_INTERNET
-(unsigned char)isConnected;
-(bool)isOnline;
#endif

#ifdef LIBENG_ENABLE_ADVERTISING
-(void)loadAd;
-(void)displayAd:(unsigned char)Id;
-(void)hideAd:(unsigned char)Id;
-(unsigned char)getAdStatus;
#endif

#ifdef LIBENG_ENABLE_SOCIAL
@property(atomic) unsigned char userGender;
@property(nonatomic, strong) NSDate* userBirthday;
@property(nonatomic, strong) NSMutableString* userLocation;

-(id<SocialOS>)create:(unsigned char)socialID byDisplayingError:(bool)display;
-(void)setAdReqInfo:(id<SocialOS>)fromSocial;
#endif

-(void)alertMessage:(const char*)msg withDuration:(double)duration;

@end

#endif // !__ANDROID__
#endif // LIBENG_APPLEOS_H_
