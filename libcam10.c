#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>
#include "libcam10.h"
#include "config.h"
#include <unistd.h>
#include <ftdi.h>

const int CameraWidth   = 1280;
const int CameraHeight  = 1024;

const uint8_t portfirst     = 0xF9; // 1111 1011
const uint8_t portfirst2    = 0xF9; // 1111 1001

/*camera state consts*/
const int cameraIdle = 0;
const int cameraWaiting = 1;
const int cameraExposing = 2;
const int cameraReading = 3;
const int cameraDownload = 4;
const int cameraError = 5;

struct ftdi_context *CAM10A, *CAM10B;

int sdx, sy0, sdy;
int kbyte;
int kolbyte;
int bufa;
int adress;
int zad;
uint16_t alevel, blevel;
int cameraState;
bool isConnected  = false;
bool imageReady = false;
bool frameReady = false;
bool errorReadFlag;
bool errorWriteFlag;

static uint8_t bufim[1280*1024];
static uint8_t bufim2[1280*1024];

static uint8_t FT_Out_Buffer[FT_OUT_BUFF];

void starti (void);
void stopi (void);
void writes (uint8_t adr, uint16_t val);
void writep (void);
void bytei (uint8_t val);
void resetchip (void);
uint16_t reads(uint8_t adr);
bool cameraSetBlevel (int val);
bool cameraSetBaudrate (struct ftdi_context *ftdi, int val );
bool readframe (int x0, int dx, int y0, int dy, bool komp);
void *posExecute ( void *arg );
int ftdi_read_data_modified ( struct ftdi_context *ftdi, unsigned char *buf,  int size );

/*===========================================================================*/

bool cameraConnect()
{
    int ret;
    bool FT_OP_flag = true;

    CAM10A = ftdi_new();
    CAM10B = ftdi_new();

    if ( ftdi_set_interface ( CAM10A, INTERFACE_A ) < 0 )
    {
        fprintf ( stderr,"libftdi error set interface A\n" );
        FT_OP_flag = false;
    }
    if ( ftdi_set_interface ( CAM10B, INTERFACE_B ) < 0 )
    {
        fprintf ( stderr,"libftdi error set interface B\n" );
        FT_OP_flag = false;
    }

    if ((ret =  ftdi_usb_open ( CAM10A, 0x0403, 0x6010 )) < 0 )
    {
        //fprintf ( stderr,"libftdi error open interface A\n" );
        fprintf(stderr, "unable to open ftdi device: %d (%s)\n", ret, ftdi_get_error_string(CAM10A));
        FT_OP_flag = false;
        return EXIT_FAILURE;
    }
    if ((ret = ftdi_usb_open ( CAM10B, 0x0403, 0x6010 )) < 0 )
    {
        //fprintf ( stderr,"libftdi error open interface B\n" );
        fprintf(stderr, "unable to open ftdi device: %d (%s)\n", ret, ftdi_get_error_string(CAM10B));
        FT_OP_flag = false;
        return EXIT_FAILURE;
    }

    if ((ret = ftdi_usb_reset(CAM10A)) < 0 )
    {
        fprintf(stderr, "unable to reset usb device A: %d (%s)\n", ret, ftdi_get_error_string(CAM10A));
        FT_OP_flag = false;
        return EXIT_FAILURE;
    }

    if ((ret = ftdi_usb_reset(CAM10B)) < 0 )
    {
        fprintf(stderr, "unable to reset usb device B: %d (%s)\n", ret, ftdi_get_error_string(CAM10B));
        FT_OP_flag = false;
        return EXIT_FAILURE;
    }

//    if ( ftdi_set_bitmode ( CAM10A, 0x00, BITMODE_RESET ) < 0 )
//    {
//        fprintf ( stderr,"libftdi error reset interface A\n" );
//        FT_OP_flag = false;
//    }

    // BitBang channel 2
    if ( ftdi_set_bitmode ( CAM10B, 0xF7, BITMODE_SYNCBB ) < 0 )
    {
        fprintf ( stderr,"libftdi error set bitbang mode interface B\n" );
        FT_OP_flag = false;
    }

    usleep(100000);

    fprintf ( stderr,"Initial BRA=%d BRB=%d rTA=%d wTB=%d\n",
              CAM10A->baudrate, CAM10B->baudrate, CAM10A->usb_read_timeout, CAM10B->usb_write_timeout );

    // Baudrate
    cameraSetBaudrate ( CAM10A, CAM10A_BAUDRATE );
    cameraSetBaudrate ( CAM10B, CAM10B_BAUDRATE );

    //timeouts - latency
    cameraSetLibftdiTimers ( CAM10_LATENCYA, CAM10_LATENCYB, CAM10_TIMERA, CAM10_TIMERB );

    //debug!
    //CAM10A->readbuffer_chunksize = 64;


    //debug!
    //if (ftdi_read_data_set_chunksize (CAM8A,256*1)<0 ) fprintf(stderr,"libftdi error set chunksize A\n");

//    if ( ftdi_read_data_set_chunksize ( CAM10A, 1 << 14 ) < 0 )
//    {
//        fprintf ( stderr,"libftdi error set chunksize A\n" );
//    }

    if ( ftdi_write_data_set_chunksize ( CAM10B, 256 ) < 0 )
    {
        fprintf ( stderr,"libftdi error set chunksize B\n" );
    }

    fprintf ( stderr,"libftdi interface A read chunksize %u\n",CAM10A->readbuffer_chunksize );
    fprintf ( stderr,"libftdi interface B read chunksize %u\n",CAM10B->readbuffer_chunksize );
    fprintf ( stderr,"libftdi interface A write chunksize %u\n",CAM10A->writebuffer_chunksize );
    fprintf ( stderr,"libftdi interface B write chunksize %u\n",CAM10B->writebuffer_chunksize );

    //Purge
    if ( ftdi_usb_purge_rx_buffer ( CAM10A ) < 0 )
    {
        fprintf ( stderr,"libftdi error purge RX interface A\n" );
    }
    if ( ftdi_usb_purge_tx_buffer ( CAM10A ) < 0 )
    {
        fprintf ( stderr,"libftdi error purge TX interface A\n" );
    }
    if ( ftdi_usb_purge_rx_buffer ( CAM10B ) < 0 )
    {
        fprintf ( stderr,"libftdi error purge RX interface B\n" );
    }
    if ( ftdi_usb_purge_tx_buffer ( CAM10B ) < 0 )
    {
        fprintf ( stderr,"libftdi error purge TX interface B\n" );
    }


    if (FT_OP_flag)
    {
        resetchip();
        /*
        Snapshot Mode—default is 0 (continuous mode).
        1 = enable (wait for TRIGGER; TRIGGER can come from outside signal (TRIGGER pin on the sensor)
        or from serial interface register restart, i.e. programming a “1” to bit 0 of Reg0x0B
        */
        writes(0x1E,0x8100); // 1000 0001 0000 0000 // Read Mode 1 bit8 -> Snapshot Mode
        writes(0x20,0x0104); // 0000 0001 0000 0100 // Read Mode 2 (was 0104 ??)
        cameraSetGain(15);
        writes(0x60,0);
        writes(0x61,0);
        writes(0x63,blevel);
        writes(0x64,blevel);
        writes(0x62,0x049D);
    }

    isConnected = FT_OP_flag;
    errorReadFlag = false;
    cameraState = cameraIdle;
    if ( FT_OP_flag == false )
    {
        ftdi_free ( CAM10B );
        ftdi_free ( CAM10A );
        cameraState = cameraError;
        fprintf ( stderr, "cameraConnect failed!\n" );
    }
    return isConnected;
}

/*===========================================================================*/

bool cameraDisconnect (void)
{
    bool FT_OP_flag = true;

    ftdi_disable_bitbang ( CAM10B );
    if (ftdi_usb_close ( CAM10B ) < 0)
    {
        fprintf ( stderr,"libftdi error usb interface B close failed\n" );
        FT_OP_flag = false;
    }
    ftdi_free ( CAM10B );
    if (ftdi_usb_close ( CAM10A ) < 0)
    {
        fprintf ( stderr,"libftdi error usb interface A close failed\n" );
        FT_OP_flag = false;
    }
    ftdi_free ( CAM10A );

    isConnected = false;
    if (FT_OP_flag)
        fprintf ( stderr, "FTDI disconnected successfully\n" );
    else
        fprintf ( stderr, "FTDI disconnect failed!\n" );
    return FT_OP_flag;
}

/*===========================================================================*/

bool cameraIsConnected()
{
    return isConnected;
}

/*===========================================================================*/

bool cameraStartExposure(int startY, int numY, double duration, int gain, uint16_t offset, bool autoOffset, int sblevel, bool overscan)
{
    int x;

    fprintf ( stderr,"starting exposure %f sec\n",duration );

    imageReady = false;
    errorReadFlag = false;
    //cameraState = cameraWaiting;
    cameraState = cameraExposing;
    blevel= sblevel;
    cameraSetGain(gain);
    cameraSetOffset(offset, autoOffset);
    cameraSetBlevel(blevel);

    writes(0x32, Pattern); // Test pattern
    writes(0x07, 0x42); // Use test data - 0x42, default 0x02

    //dlit1 = round(duration*1000)+1000;
    x = round(100 * duration * 1000 / 13);
    writes(0x09, x); // Number of rows of integration — default = 0x0419 (1049), see datasheet
    zad = round(duration * 1000 - 40);
    if (zad < 0)
    {
        zad = 0;
    }
    if (readframe(0, CameraWidth, startY, numY, overscan))
    {
        imageReady = true;
        fprintf ( stderr,"OK readframe()\n" );
        return true;
    } else
    {
        //debug!
        imageReady = true;
        fprintf ( stderr,"Error readframe()\n" );
        return false;
    }
}

/*===========================================================================*/

bool cameraSetBaudrate (struct ftdi_context *ftdi, int val )
{
    int ftdi_result;
    int  FT_Current_Baud;
    bool Result = true;
    /*setup FT2232 baud rate*/

    FT_Current_Baud = val * BAUDRATE_MULT;
    fprintf ( stderr,"trying to set baudrate to %d...\n", FT_Current_Baud);

    ftdi_result = ftdi_set_baudrate ( ftdi, FT_Current_Baud );
    if ( ftdi_result != 0 )
    {
        fprintf ( stderr,"error set baud intervace %d to %d\n", ftdi->interface, FT_Current_Baud);
        Result = false;
    }

    fprintf ( stderr,"SetBaudrate IF %d BR=%d ReadTO=%d WriteTO=%d\n",
              ftdi->interface, ftdi->baudrate, ftdi->usb_read_timeout, ftdi->usb_write_timeout );

    return Result;
}

/*===========================================================================*/

bool cameraSetLibftdiTimers ( int latA, int latB, int timerA, int timerB )
{
    int ftdi_result;
    ftdi_result = ftdi_set_latency_timer ( CAM10A, latA );
    if ( ftdi_result != 0 )
    {
        fprintf (stderr,"error set latency interface A\n");
    }
    ftdi_result = ftdi_set_latency_timer ( CAM10B, latB );
    if ( ftdi_result != 0 )
    {
        fprintf (stderr,"error set latency interface B\n");
    }

    CAM10A->usb_read_timeout = timerA;
    CAM10B->usb_read_timeout = timerB;
    CAM10A->usb_write_timeout = 100;
    CAM10B->usb_write_timeout = 100;

    fprintf ( stderr,"SetTimers IF A ReadTO=%d WriteTO=%d\n",
              CAM10A->usb_read_timeout, CAM10A->usb_write_timeout );

    fprintf ( stderr,"SetTimers IF B ReadTO=%d WriteTO=%d\n",
              CAM10B->usb_read_timeout, CAM10B->usb_write_timeout );

    if (ftdi_result != 0)
        return false;
    else
        return true;
}

/*===========================================================================*/

int cameraGetCameraState ()
{
    int Result;
    if (!errorWriteFlag)
    {
        Result = cameraState;
    }
    else
        Result = cameraError;
    return Result;
}

/*===========================================================================*/

bool cameraGetImageReady()
{
    return imageReady;
}

/*===========================================================================*/

uint8_t * cameraGetImage ()
{
    return bufim2;
}

/*===========================================================================*/

int cameraGetError()
{
    int res;
    res = 0;
    if ( errorWriteFlag )
    {
        res += 2;
    }
    if ( errorReadFlag )
    {
        res++;
    }
    return res;
}

/*===========================================================================*/

bool cameraSetBlevel (int val)
{
    if ((val > 24) || (val < 0))
    {
        val = 24;
    }
    blevel = val;
    writes(0x63, 2 * alevel + val); // Even row, odd column — analog offset correction value
    writes(0x64, 2 * alevel + val); // Odd row, even column — analog offset correction value
    return true;
}


/*===========================================================================*/

bool cameraSetOffset (uint16_t val, bool aut)
{
    if (aut) {
        writes(0x62,0x0498); // 0000 0100 1001 1000 // Manual override of black level correction
    }
    else {
        writes(0x62,0x049D); // 0000 0100 1001 1101 // Apply black level calibration continuously
    }
    alevel = val;
    writes(0x60,2 * val); // Even row, even column — analog offset correction value
    writes(0x61,2 * val); // Odd row, odd column — analog offset correction value
    writes(0x63,2 * val + blevel);
    writes(0x64,2 * val + blevel);

    return true;
}

/*===========================================================================*/

bool cameraSetGain(int val)
{
    const uint8_t gain[] = {0x08,0x10,0x18,0x20,0x54,0x58,0x5c,0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67};
    if ((val > 15) || (val < 0))
    {
        val = 15;
    }
    writes(0x35, gain[val]); //Global gain register
    return true;
}

/*===========================================================================*/

void *posExecute ( void *arg )
{
    int x,y,buf;
    int ex;
    double er;
    uint16_t fx,fbuf;

    cameraState = cameraReading;
    usleep(zad * 1000);
    bufa = 0;

    if (!errorWriteFlag)
    {

        buf = ftdi_read_data_modified( CAM10A, bufim, kolbyte);

        fprintf( stderr,"ftdi_read_data, data size= %d, %d, %d\n", buf, sizeof(bufim), kolbyte );
    }
    else
    {
        buf = 0;
    }
    kbyte = buf;
    if (buf == kolbyte)
    {
        fprintf( stderr,"buf = kolbyte\n" );


        for (y=0; y<= sdy-1; y++)
            for (x=0;  x <= sdx-1; x++)
                bufim2[x + CameraWidth * (y + sy0)] = bufim[x + y * (sdx + 0)];

        /* fill random data frame */
        //        int row_size = 1280;
        //        int height = 1024;
        //        for (int i = 0; i < height; i++)
        //            for (int j = 0; j < row_size; j++)
        //                bufim2[i * row_size + j] = rand() % 255;
        //        fprintf( stderr,"bufim2 ready\n");

    }
    else
    {
        bufa = 1;
        errorReadFlag = true;
    }
    if (!errorWriteFlag)
    {
        //Get_USB_Device_Status(FT_CAM10A);
        //if FT_Q_Bytes > 0 then Purge_USB_Device(FT_CAM10A,FT_PURGE_RX);
    }


    //hev.SetEvent;
    cameraState = cameraDownload;
    cameraState = cameraIdle;
    //imageReady = true; //moved upper
    ftdi_usb_purge_rx_buffer ( CAM10A );
    ( void ) arg;
    pthread_exit(NULL);
}

/*===========================================================================*/

bool readframe (int x0, int dx, int y0, int dy, bool komp)
{
    fprintf( stderr,"Reading frame x0= %d; dx= %d; y0= %d; dy= %d; \n", x0, dx, y0, dy);
    writes(0x01, 12 + y0);  //First row to be read out — default = 0x000C (12)
    writes(0x02, 20 + x0);  //First column to be read out — default = 0x0014 (20)
    writes(0x03, dy - 1);   //Window height (number of rows - 1) — default = 0x03FF (1023)
    writes(0x04, dx - 1);   //Window width (number of columns - 1) — default = 0x04FF (1279)
    writes(0x0B, 0x1);      //Frame restart
    kolbyte = dx * dy;
    sdx = dx;
    sdy = dy;
    sy0 = y0;

    pthread_t t1;
    pthread_create ( &t1, NULL, posExecute, NULL );
    pthread_join(t1,NULL);

    fprintf(stderr, "------ first 100 bytes dump ------\n");
    for (int i = 0; i < 100; i++) {
        if (i % 24 == 0)
            fprintf(stderr, "\n");
        fprintf(stderr, "%02x ", bufim[i]);
    }
    fprintf(stderr, "...\n");

    FILE * fileID;
    fileID = fopen("framedata_out.txt", "w");
    for (int i = 0; i < sizeof(bufim); i++) {
        fprintf(fileID, "%02x ", bufim[i]);
    }
    fclose(fileID);

    if (bufa < 1)
    {
        return true;
    }
    else if (!errorWriteFlag)
    {
        ftdi_usb_purge_rx_buffer(CAM10A);
        ftdi_usb_purge_tx_buffer(CAM10B);
    }
    return false;
}


/*===========================================================================*/

void writes (uint8_t adr, uint16_t val)
{
    adress = 0;
    starti();
    bytei(0xBA); //write data
    bytei(adr);
    bytei(val >> 8);    //hi
    bytei(val & 0xFF);  //lo
    stopi();
    writep();
    fprintf( stderr,"Channel B data 0x%02x < 0x%04x\n", adr, val );
    reads(adr);
}

/*===========================================================================*/

void starti ()
{
    FT_Out_Buffer[adress+0] = portfirst2 | 0x2;
    FT_Out_Buffer[adress+1] = portfirst2 | 0x6;
    FT_Out_Buffer[adress+2] = portfirst2 | 0x4;
    adress += 3;
}

/*===========================================================================*/

void stopi ()
{
    FT_Out_Buffer[adress+0] = portfirst2 | 0x4;
    FT_Out_Buffer[adress+1] = portfirst2 | 0x6;
    FT_Out_Buffer[adress+2] = portfirst2 | 0x2;
    FT_Out_Buffer[adress+3] = portfirst2;
    adress += 4;
}

/*===========================================================================*/

void bytei (uint8_t val)
{
    uint8_t buf;
    for (int i = 0; i <= 7; i++)
    {
        if ((val & 0x80) != 0) {
            buf = portfirst2;
        }
        else {
            buf = portfirst2 | 0x4;
        }
        val = val << 1;
        FT_Out_Buffer[adress+0] = buf;
        FT_Out_Buffer[adress+1] = buf | 0x2;
        FT_Out_Buffer[adress+2] = buf;
        adress += 3;
    }
    FT_Out_Buffer[adress+0] = portfirst2;
    FT_Out_Buffer[adress+1] = portfirst2 | 0x2;
    FT_Out_Buffer[adress+2] = portfirst2 | 0x4;
    adress += 3;
}

/*===========================================================================*/

void writep ()
{
    //FILE * fileID;

    if ( ftdi_write_data( CAM10B, FT_Out_Buffer, adress ) < 0 )
    {
        fprintf( stderr,"write failed on interface B\n" );
    }

    //    fileID = fopen("writep_out.txt", "w");
    //    fprintf(fileID, "------%d bytes-----\n", adress);

    //    for (int i = 0; i <= adress; i++) {
    //        // fprintf( stderr,"0x%04x\n", FT_Out_Buffer[i] );

    //        fprintf(fileID, "0x%04x, ", FT_Out_Buffer[i]);
    //    }
    //    fclose(fileID);

    adress = 0;
}

/*===========================================================================*/

void resetchip()
{
    adress = 0;
    for (adress = 0; adress <= 99; adress++) {
        FT_Out_Buffer[adress] = portfirst;
    }
    for (adress = 100; adress <= 199; adress++) {
        FT_Out_Buffer[adress] = portfirst - 0x1;
    }
    for (adress = 200; adress <= 299; adress++) {
        FT_Out_Buffer[adress] = portfirst;
    }
    writep();
}

/*===========================================================================*/

void byteo (uint8_t val)
{
    uint8_t b, buf;
    b = 0xFE;
    for (int i = 0; i <= 7; i++)
    {
        if ((b & 0x80) != 0)
            buf = portfirst2;
        else
            buf = portfirst2 + 0x4;
        b = 2 * b;
        FT_Out_Buffer[adress+0] = portfirst2;
        FT_Out_Buffer[adress+1] = portfirst2 | 0x2;
        FT_Out_Buffer[adress+2] = buf;
        adress += 3;
    }
    FT_Out_Buffer[adress+0] = portfirst2 | val;
    FT_Out_Buffer[adress+1] = portfirst2 | (val | 0x2);
    FT_Out_Buffer[adress+2] = portfirst2 | val;
    adress += 3;
}

/*===========================================================================*/

uint16_t reads(uint8_t adr)
{
    int bufs;
    uint16_t ou;
    uint8_t bufi[2048];

    if (ftdi_usb_purge_tx_buffer (CAM10B) < 0)
        fprintf( stderr,"Channel B ftdi_usb_purge_tx_buffer Failed!\n");

    adress = 0;
    starti();       //  3
    bytei(0xBA);    // 27
    bytei(adr);     // 27
    starti();       //  3
    bytei(0xBB);    // 27
    byteo(0x4);     // 27
    byteo(0x0);     // 27
    stopi();        //  4
    writep();
    usleep(10000); // 10 ms

    bufs = ftdi_read_data_modified( CAM10B, bufi, 145);

    ou = 0;
    if ((bufi[0x58] & 0x08) != 0)
        ou = ou + 0x8000;
    if ((bufi[0x5b] & 0x08) != 0)
        ou = ou + 0x4000;
    if ((bufi[0x5e] & 0x08) != 0)
        ou = ou + 0x2000;
    if ((bufi[0x61] & 0x08) != 0)
        ou = ou + 0x1000;
    if ((bufi[0x64] & 0x08) != 0)
        ou = ou + 0x0800;
    if ((bufi[0x67] & 0x08) != 0)
        ou = ou + 0x0400;
    if ((bufi[0x6a] & 0x08) != 0)
        ou = ou + 0x0200;
    if ((bufi[0x6d] & 0x08) != 0)
        ou = ou + 0x0100;
    if ((bufi[0x73] & 0x08) != 0)
        ou = ou + 0x80;
    if ((bufi[0x76] & 0x08) != 0)
        ou = ou + 0x40;
    if ((bufi[0x79] & 0x08) != 0)
        ou = ou + 0x20;
    if ((bufi[0x7c] & 0x08) != 0)
        ou = ou + 0x10;
    if ((bufi[0x7f] & 0x08) != 0)
        ou = ou + 0x08;
    if ((bufi[0x82] & 0x08) != 0)
        ou = ou + 0x04;
    if ((bufi[0x85] & 0x08) != 0)
        ou = ou + 0x02;
    if ((bufi[0x88] & 0x08) != 0)
        ou = ou + 0x01;

    fprintf( stderr,"Channel B data 0x%02x > 0x%04x\n", adr, ou );
    return ou;
}

/*===========================================================================*/

//int ftdi_read_data_modified ( struct  ftdi_context * ftdi, unsigned char * buf, int size )
//{
//    int rsize = ftdi_read_data ( ftdi, buf, size );
//    int nsize = size - rsize;
//    int retry = 0;
//    while ( ( nsize > 0 ) && ( retry < 20 ) )
//    {
//        retry++;
//        usleep ( 10000 );
//        fprintf ( stderr,"Try %d since %d<>%d \n",retry, rsize, size );
//        rsize = rsize + ftdi_read_data ( ftdi, buf + rsize, nsize );
//        nsize = size - rsize;
//    }
//    return rsize;
//}


int ftdi_read_data_modified ( struct ftdi_context *ftdi, unsigned char *buf, int size )
{
    const int uSECPERSEC = 1000000;
    const int uSECPERMILLISEC = 1000;

    int offset;
    int result;
    // Sleep interval, 1 microsecond
    struct timespec tm;
    tm.tv_sec = 0;
    tm.tv_nsec = 1000L;
    // Read timeout
    struct timeval startTime;
    struct timeval timeout;
    struct timeval now;

    gettimeofday ( &startTime, NULL );
    // usb_read_timeout in milliseconds
    // Calculate read timeout time of day
    timeout.tv_sec = startTime.tv_sec + ftdi->usb_read_timeout / uSECPERMILLISEC;
    timeout.tv_usec = startTime.tv_usec + ( ( ftdi->usb_read_timeout % uSECPERMILLISEC ) *uSECPERMILLISEC );
    if ( timeout.tv_usec >= uSECPERSEC ) {
        timeout.tv_sec++;
        timeout.tv_usec -= uSECPERSEC;
    }

    offset = 0;
    result = 0;

    while ( size > 0 ) {
        result = ftdi_read_data ( ftdi, buf+offset, size );
        if ( result < 0 ) {
            fprintf ( stderr,"Read failed -- error (%d))\n",result );
            break;
        }
        if ( result == 0 ) {
            gettimeofday ( &now, NULL );
            if ( now.tv_sec > timeout.tv_sec || ( now.tv_sec == timeout.tv_sec && now.tv_usec > timeout.tv_usec ) ) {
                fprintf ( stderr,"Read failed -- timeout %d \n", offset );
                break;
            }
            nanosleep ( &tm, NULL ); //sleep for 1 microsecond
            continue;
        }
        size -= result;
        offset += result;
    }
    return offset;
}











