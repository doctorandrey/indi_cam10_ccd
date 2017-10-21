#ifndef __LIBCAM10_H__
#define __LIBCAM10_H__

//#define TESTING

#define CAM10A_BAUDRATE     20
#define CAM10B_BAUDRATE     5
#define CAM10_READCHUNK     16384
#define CAM10_WRITECHUNK    256
#define CAM10_LATENCYA      2
#define CAM10_LATENCYB      2
#define CAM10_TIMERA        250 // USB Read Timeout
#define CAM10_TIMERB        100 // USB Write Timeout
#define BAUDRATE_MULT       10000
#define FT_OUT_BUFF         0x10000

#ifdef __cplusplus
extern "C" {
#endif

unsigned int    Pattern;
unsigned int    CAM10_Gain;
int             CAM10_Offset;
bool            CAM10_AutoOffset;

bool cameraConnect(void);
bool cameraDisconnect(void); 
bool cameraIsConnected(void);
bool cameraStartExposure(int startY, int numY, double duration, int gain, uint16_t offset, bool autoOffset, int sblevel, bool overscan);
int  cameraGetCameraState(void);
bool cameraGetImageReady(void);
bool cameraSetGain (int val);
bool cameraSetOffset (uint16_t val, bool aut);
int  cameraGetError(void);
uint8_t * cameraGetImage();
bool cameraSetBaudrateA(int val);
bool cameraSetLibftdiTimers(int latA,int latB,int timerA,int timerB);

#ifdef __cplusplus
}
#endif

#endif
