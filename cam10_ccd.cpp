/*
   INDI Developers Manual
   Tutorial #3

   "Cam10 Driver"

   We develop a Cam10 driver.

   Refer to README, which contains instruction on how to build this driver, and use it
   with an INDI-compatible client.

*/

/** \file simpleccd.cpp
    \brief Construct a basic INDI CCD device that simulates exposure & temperature settings. It also generates a random pattern and uploads it as a FITS file.
    \author Jasem Mutlaq

    \example simpleccd.cpp
    A Cam10 device that can capture images and control temperature. It returns a FITS image to the client. To build drivers for complex CCDs, please
    refer to the INDI Generic CCD driver template in INDI SVN (under 3rdparty).
*/

#define TESTING

#include <sys/time.h>
#include <memory>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include "cam10_ccd.h"
#include "libcam10.h"

const int POLLMS           = 500;       /* Polling interval 500 ms */


std::unique_ptr<Cam10CCD> simpleCCD(new Cam10CCD());

void ISGetProperties(const char *dev)
{
    simpleCCD->ISGetProperties(dev);
}

void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int num)
{
    simpleCCD->ISNewSwitch(dev, name, states, names, num);
}

void ISNewText(	const char *dev, const char *name, char *texts[], char *names[], int num)
{
    simpleCCD->ISNewText(dev, name, texts, names, num);
}

void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int num)
{
    simpleCCD->ISNewNumber(dev, name, values, names, num);
}

void ISNewBLOB (const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[], char *names[], int n)
{
    INDI_UNUSED(dev);
    INDI_UNUSED(name);
    INDI_UNUSED(sizes);
    INDI_UNUSED(blobsizes);
    INDI_UNUSED(blobs);
    INDI_UNUSED(formats);
    INDI_UNUSED(names);
    INDI_UNUSED(n);
}

void ISSnoopDevice (XMLEle *root)
{
    //simpleCCD->ISSnoopDevice(root);
    INDI_UNUSED(root);
}

Cam10CCD::Cam10CCD()
{
    InExposure = false;
}

/*******************************************************************************
** Client is asking us to set a new number
*******************************************************************************/
bool Cam10CCD::ISNewNumber(const char *dev, const char *name,
                           double values[], char *names[], int n)
{
    if (!strcmp(dev, getDeviceName()))
    {
        if (!strcmp(name, GainNP.name))
        {
            IUUpdateNumber(&GainNP, values, names, n);
            GainNP.s = IPS_OK;
            IDSetNumber(&GainNP, NULL);
            cameraSetGain (GainN[0].value);
            IDMessage(getDeviceName(), "Cam10 set gain = %d",(int) GainN[0].value);
            return true;
        }

        if (!strcmp(name, OffsetNP.name))
        {
            IUUpdateNumber(&OffsetNP, values, names, n);
            OffsetNP.s = IPS_OK;
            IDSetNumber(&OffsetNP, NULL);
            cameraSetOffset (OffsetN[0].value, false);
            IDMessage(getDeviceName(), "Cam10 set offset = %d",(int) OffsetN[0].value);
            return true;
        }

        if (!strcmp(name, BaudrateNP.name))
        {
            IUUpdateNumber(&BaudrateNP, values, names, n);
            BaudrateNP.s = IPS_OK;
            IDSetNumber(&BaudrateNP, NULL);
//            if (!cameraSetBaudrate(BaudrateN[0].value))
//            {
//                BaudrateNP.s = IPS_BUSY;
//                IDMessage(getDeviceName(), "Cam10 set baudrate Failed!");
//            } else
//                IDMessage(getDeviceName(), "Cam10 set baudrate = %d",(int) BaudrateN[0].value);
            return true;
        }

        if (!strcmp(name, LibftditimerANP.name))
        {
            IUUpdateNumber(&LibftditimerANP, values, names, n);
            LibftditimerANP.s = IPS_OK;
            IDSetNumber(&LibftditimerANP, NULL);
            cameraSetLibftdiTimers(LibftdilatencyAN[0].value,LibftdilatencyBN[0].value,LibftditimerAN[0].value,LibftditimerBN[0].value);
            IDMessage(getDeviceName(), "Cam10 set libftdi timerA = %d",(int) LibftditimerAN[0].value);
            IDMessage(getDeviceName(), "Cam10 set libftdi latencyA = %d",(int) LibftdilatencyAN[0].value);
            IDMessage(getDeviceName(), "Cam10 set libftdi timerB = %d",(int) LibftditimerBN[0].value);
            IDMessage(getDeviceName(), "Cam10 set libftdi latencyB = %d",(int) LibftdilatencyBN[0].value);
            return true;
        }

        if (!strcmp(name, LibftdilatencyANP.name))
        {
            IUUpdateNumber(&LibftdilatencyANP, values, names, n);
            LibftdilatencyANP.s = IPS_OK;
            IDSetNumber(&LibftdilatencyANP, NULL);
            cameraSetLibftdiTimers(LibftdilatencyAN[0].value,LibftdilatencyBN[0].value,LibftditimerAN[0].value,LibftditimerBN[0].value);
            IDMessage(getDeviceName(), "Cam10 set libftdi timerA = %d",(int) LibftditimerAN[0].value);
            IDMessage(getDeviceName(), "Cam10 set libftdi latencyA = %d",(int) LibftdilatencyAN[0].value);
            IDMessage(getDeviceName(), "Cam10 set libftdi timerB = %d",(int) LibftditimerBN[0].value);
            IDMessage(getDeviceName(), "Cam10 set libftdi latencyB = %d",(int) LibftdilatencyBN[0].value);
            return true;
        }
        if (!strcmp(name, LibftditimerBNP.name))
        {
            IUUpdateNumber(&LibftditimerBNP, values, names, n);
            LibftditimerBNP.s = IPS_OK;
            IDSetNumber(&LibftditimerBNP, NULL);
            cameraSetLibftdiTimers(LibftdilatencyAN[0].value,LibftdilatencyBN[0].value,LibftditimerAN[0].value,LibftditimerBN[0].value);
            IDMessage(getDeviceName(), "Cam10 set libftdi timerA = %d",(int) LibftditimerAN[0].value);
            IDMessage(getDeviceName(), "Cam10 set libftdi latencyA = %d",(int) LibftdilatencyAN[0].value);
            IDMessage(getDeviceName(), "Cam10 set libftdi timerB = %d",(int) LibftditimerBN[0].value);
            IDMessage(getDeviceName(), "Cam10 set libftdi latencyB = %d",(int) LibftdilatencyBN[0].value);
            return true;
        }

        if (!strcmp(name, LibftdilatencyBNP.name))
        {
            IUUpdateNumber(&LibftdilatencyBNP, values, names, n);
            LibftdilatencyBNP.s = IPS_OK;
            IDSetNumber(&LibftdilatencyBNP, NULL);
            cameraSetLibftdiTimers(LibftdilatencyAN[0].value,LibftdilatencyBN[0].value,LibftditimerAN[0].value,LibftditimerBN[0].value);
            IDMessage(getDeviceName(), "Cam10 set libftdi timerA = %d",(int) LibftditimerAN[0].value);
            IDMessage(getDeviceName(), "Cam10 set libftdi latencyA = %d",(int) LibftdilatencyAN[0].value);
            IDMessage(getDeviceName(), "Cam10 set libftdi timerB = %d",(int) LibftditimerBN[0].value);
            IDMessage(getDeviceName(), "Cam10 set libftdi latencyB = %d",(int) LibftdilatencyBN[0].value);
            return true;
        }

    }

    // If we didn't process anything above, let the parent handle it.
    return INDI::CCD::ISNewNumber(dev,name,values,names,n);
}

/*******************************************************************************
** Client is asking us to set a new switch
*******************************************************************************/

bool Cam10CCD::ISNewSwitch (const char *dev, const char *name,
                            ISState *states, char *names[], int n)
{
    if (strcmp(dev, getDeviceName()) == 0)
    {
        /* AutoOffset */
        if (!strcmp(name, AutoOffsetSP.name))
        {
            const char *actionName = IUFindOnSwitchName(states, names, n);
            // If switch is the same state as actionName, then we do nothing.
            int currentAOffsetIndex = IUFindOnSwitchIndex(&AutoOffsetSP);
            if (!strcmp(actionName, AutoOffsetS[currentAOffsetIndex].name))
            {
                DEBUGF(INDI::Logger::DBG_SESSION, "AutoOffset is already %s", AutoOffsetS[currentAOffsetIndex].label);
                IDSetSwitch(&AutoOffsetSP, NULL);
                return true;
            }

            // Otherwise, let us update the switch state
            IUUpdateSwitch(&AutoOffsetSP, states, names, n);
            currentAOffsetIndex = IUFindOnSwitchIndex(&AutoOffsetSP);
            DEBUGF(INDI::Logger::DBG_SESSION, "AutoOffset is now %s", AutoOffsetS[currentAOffsetIndex].label);
            if (!strcmp(AutoOffsetS[currentAOffsetIndex].name, "AUTOOFFSET_ON")) {
                AutoOffsetSP.s = IPS_OK;
            } else {
                AutoOffsetSP.s = IPS_IDLE;
            }

            IDSetSwitch(&AutoOffsetSP, NULL);
            return true;
        }
    }

    return INDI::CCD::ISNewSwitch (dev, name, states, names,  n);
}


/**************************************************************************************
** Client is asking us to establish connection to the device
***************************************************************************************/
bool Cam10CCD::Connect()
{
    // Let's set a timer that checks teleCCDs status every POLLMS milliseconds.
    SetTimer(POLLMS);

    try
    {
        cameraConnect();
    }
    catch (std::runtime_error err)
    {
        IDMessage(getDeviceName(), "Cam10 connect failed! %s", err.what());
        return false;
    }


    //    cameraSetBaudrate(80);
    //    cameraSetOffset(100);
    IDMessage(getDeviceName(), "Cam10 connected successfully!");

    return true;
}

/**************************************************************************************
** Client is asking us to terminate connection to the device
***************************************************************************************/
bool Cam10CCD::Disconnect()
{
    cameraDisconnect();
    IDMessage(getDeviceName(), "Cam10 disconnected!");
    return true;
}

/**************************************************************************************
** INDI is asking us for our default device name
***************************************************************************************/
const char * Cam10CCD::getDefaultName()
{
    return "Cam10";
}

/**************************************************************************************
** INDI is asking us to init our properties.
***************************************************************************************/
bool Cam10CCD::initProperties()
{
    // Must init parent properties first!
    INDI::CCD::initProperties();

    /*const short minGain = 0;
    const short maxGain = 63;
    const short minOffset = -127;
    const short maxOffset = 127;
    const short minBaudrate = 80;
    const short maxBaudrate = 240;*/

    /* Add Gain number property (gs) */
    IUFillNumber(GainN, "GAIN", "Gain", "%g", 0, 15, 1, CAM10_GAIN);
    IUFillNumberVector(&GainNP, GainN, 1, getDeviceName(),"GAIN",
                       "Gain", MAIN_CONTROL_TAB, IP_RW, 0, IPS_IDLE);

    /* Now let us initilize the switch properties. */
    IUFillSwitch(&AutoOffsetS[AUTOOFFSET_ON], "AUTOOFFSET_ON", "On", ISS_OFF);
    IUFillSwitch(&AutoOffsetS[AUTOOFFSET_OFF], "AUTOOFFSET_OFF", "Off", ISS_ON);
    IUFillSwitchVector(&AutoOffsetSP, AutoOffsetS, 2, getDeviceName(), "AUTO_OFFSET",
                       "Auto Offset", MAIN_CONTROL_TAB, IP_RW, ISR_1OFMANY, 60, IPS_IDLE);

    /* Add Offset number property (gs) */
    IUFillNumber(OffsetN, "OFFSET", "Offset", "%g", -63, 63, 1, CAM10_OFFSET);
    IUFillNumberVector(&OffsetNP, OffsetN, 1, getDeviceName(),"OFFSET",
                       "Offset", MAIN_CONTROL_TAB, IP_RW, 0, IPS_IDLE);

    /* Add Baudrate number property (gs) */
    IUFillNumber(BaudrateN, "BAUDRATE", "Baudrate", "%g", 10, 300, 5, CAM10A_BAUDRATE);
    IUFillNumberVector(&BaudrateNP, BaudrateN, 1, getDeviceName(),"BAUDRATE",
                       "Baudrate", MAIN_CONTROL_TAB, IP_RW, 0, IPS_IDLE);

    /* Add Latency number property (gs) */
    IUFillNumber(LibftdilatencyAN, "LATENCYA", "LatencyA", "%g", 1, 50, 1, CAM10_LATENCYA);
    IUFillNumberVector(&LibftdilatencyANP, LibftdilatencyAN, 1, getDeviceName(),"LATENCYA",
                       "LatencyA", MAIN_CONTROL_TAB, IP_RW, 0, IPS_IDLE);

    /* Add timers number property (gs) */
    IUFillNumber(LibftditimerAN, "TIMERA", "TimerA", "%g", 100, 20000000, 100, CAM10_TIMERA);
    IUFillNumberVector(&LibftditimerANP, LibftditimerAN, 1, getDeviceName(),"TIMERA",
                       "TimerA", MAIN_CONTROL_TAB, IP_RW, 0, IPS_IDLE);

    /* Add Latency number property (gs) */
    IUFillNumber(LibftdilatencyBN, "LATENCYB", "LatencyB", "%g", 1, 50, 1, CAM10_LATENCYB);
    IUFillNumberVector(&LibftdilatencyBNP, LibftdilatencyBN, 1, getDeviceName(),"LATENCYB",
                       "LatencyB", MAIN_CONTROL_TAB, IP_RW, 0, IPS_IDLE);

    /* Add timers number property (gs) */
    IUFillNumber(LibftditimerBN, "TIMERB", "TimerB", "%g",  100, 150000, 100, CAM10_TIMERB);
    IUFillNumberVector(&LibftditimerBNP, LibftditimerBN, 1, getDeviceName(),"TIMERB",
                       "TimerB", MAIN_CONTROL_TAB, IP_RW, 0, IPS_IDLE);

    // We set the CCD capabilities
    //uint32_t cap = CCD_CAN_ABORT | CCD_CAN_BIN | CCD_CAN_SUBFRAME | CCD_HAS_BAYER;
    //SetCCDCapability(cap);

    //IUSaveText(&BayerT[2], "GRBG");
    // Add Debug, Simulator, and Configuration controls
    addAuxControls();

    return true;
}

/********************************************************************************************
** INDI is asking us to update the properties because there is a change in CONNECTION status
** This fucntion is called whenever the device is connected or disconnected.
*********************************************************************************************/
bool Cam10CCD::updateProperties()
{
    // Call parent update properties first
    INDI::CCD::updateProperties();

    if (isConnected())
    {
        // Let's get parameters now from CCD
        setupParams();

        // Start the timer
        SetTimer(POLLMS);
        defineNumber(&GainNP);
        defineSwitch(&AutoOffsetSP);
        defineNumber(&OffsetNP);
        defineNumber(&BaudrateNP);
        defineNumber(&LibftditimerANP);
        defineNumber(&LibftdilatencyANP);
        defineNumber(&LibftditimerBNP);
        defineNumber(&LibftdilatencyBNP);
    }
    else
    {
        deleteProperty(GainNP.name);
        deleteProperty(AutoOffsetSP.name);
        deleteProperty(OffsetNP.name);
        deleteProperty(BaudrateNP.name);
        deleteProperty(LibftditimerANP.name);
        deleteProperty(LibftdilatencyANP.name);
        deleteProperty(LibftditimerBNP.name);
        deleteProperty(LibftdilatencyBNP.name);
    }

    return true;
}

/**************************************************************************************
** Setting up CCD parameters
***************************************************************************************/
void Cam10CCD::setupParams()
{
    // MT09M001 is an (10)8 bit CCD, 1280x1024 resolution, with 5.4um square pixels.
    SetCCDParams(1280, 1024, 8, 5.4, 5.4);

    // Let's calculate how much memory we need for the primary CCD buffer
    int nbuf;
    nbuf = PrimaryCCD.getXRes() * PrimaryCCD.getYRes() * PrimaryCCD.getBPP() / 8;
    nbuf += 512;                      //  leave a little extra at the end
    PrimaryCCD.setFrameBufferSize(nbuf);
}

/**************************************************************************************
** Client is asking us to start an exposure
***************************************************************************************/
bool Cam10CCD::StartExposure(float duration)
{
    bool expResult;
    ExposureRequest = duration;
    IDMessage(getDeviceName(), "Start exposure %g sec", duration);

    // Since we have only have one CCD with one chip, we set the exposure duration of the primary CCD
    PrimaryCCD.setExposureDuration(duration);

    //cameraStartExposure(1,0,0,3000,2000, duration,true);
    //int r = cameraStartExposure(PrimaryCCD.getBinX(),PrimaryCCD.getSubX(),PrimaryCCD.getSubY(),PrimaryCCD.getSubW(),PrimaryCCD.getSubH(), duration, true);
    expResult = cameraStartExposure(0, 1024, duration, 15, 10, false, 10, false);

    gettimeofday(&ExpStart,NULL);

    InExposure = true;

    // We're done
    return expResult;
}

/**************************************************************************************
** Client is asking us to abort an exposure
***************************************************************************************/
bool Cam10CCD::AbortExposure()
{
    InExposure = false;
    return true;
}

/**************************************************************************************
** Client is asking us to set a new temperature
***************************************************************************************/
int Cam10CCD::SetTemperature(double temperature)
{
    TemperatureRequest = temperature;

    // 0 means it will take a while to change the temperature
    return 0;
}

/**************************************************************************************
** How much longer until exposure is done?
***************************************************************************************/
float Cam10CCD::CalcTimeLeft()
{
    double timesince;
    double timeleft;
    struct timeval now;
    gettimeofday(&now, NULL);

    timesince = (double)(now.tv_sec * 1000.0 + now.tv_usec/1000) - (double)(ExpStart.tv_sec * 1000.0 + ExpStart.tv_usec/1000);
    timesince = timesince / 1000;

    timeleft = ExposureRequest - timesince;
    return timeleft;
}

/**************************************************************************************
** Main device loop. We check for exposure and temperature progress here
***************************************************************************************/
void Cam10CCD::TimerHit()
{
    long timeleft;

    if(isConnected() == false)
        return;  //  No need to reset timer if we are not connected anymore

    if (InExposure)
    {
        timeleft=CalcTimeLeft();

        // Less than a 0.1 second away from exposure completion
        // This is an over simplified timing method, check CCDSimulator and simpleCCD for better timing checks
        if(timeleft < 0.1)
        {
            /* We're done exposing */
            IDMessage(getDeviceName(), "Exposure done, downloading image...");

            // Set exposure left to zero
            PrimaryCCD.setExposureLeft(0);

            // We're no longer exposing...
            InExposure = false;

            /* grab and save image */
            grabImage();

        }
        else
            // Just update time left in client
            PrimaryCCD.setExposureLeft(timeleft);

    }


    SetTimer(POLLMS);
    return;
}

/**************************************************************************************
** Get image and return it to client
***************************************************************************************/
void Cam10CCD::grabImage()
{
    // Let's get a pointer to the frame buffer
    uint8_t *image = PrimaryCCD.getFrameBuffer();
    int width = PrimaryCCD.getSubW() / PrimaryCCD.getBinX()   * PrimaryCCD.getBPP() / 8;
    //int row_size   = PrimaryCCD.getSubW() / PrimaryCCD.getBinX() * PrimaryCCD.getBPP() / 8;
    int height = PrimaryCCD.getSubH() / PrimaryCCD.getBinY();


    IDMessage(getDeviceName(), "grabImage width=%d height=%d BPP=%d\n", width, height, PrimaryCCD.getBPP() );


    while (!cameraGetImageReady() ); // waiting image


    //cameraGetImage(image);
    //*bufim = (unsigned short *) image;
    //image = *bufim;
    PrimaryCCD.setFrameBuffer(image);

    //    for (int i = 0; i < height; i++)
    //        for (int j = 0; j < row_size; j++)
    //            image[i * row_size + j] = rand() % 255;


    int k = 0;
    uint8_t *img;
    img = cameraGetImage();

    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
        {
            image[i * width + j] = * (img + k);
            k++;
        }


    IDMessage(getDeviceName(), "Download complete.");

    // Let INDI::CCD know we're done filling the image buffer
    ExposureComplete(&PrimaryCCD);
}



#ifdef TESTING

int main(int argc, char *argv[])
{
    int opt;
    int baud = 10;
    double exp = 1;
    while ((opt = getopt (argc, argv, "e:b:")) != -1)
    {
        switch (opt)
        {
        case 'e':
            exp = atof (optarg);
            break;

        case 'b':
            baud = atoi (optarg);
            break;

        default: /* '?' */
            fprintf(stderr, "Usage: %s [-e nsecs] [-b] baudrate\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }


    if (cameraConnect()) {
        //cameraSetBaudrate(baud);
        //cameraSetOffset(0);
        //cameraSetGain(0);
        int r = cameraStartExposure(0, 1024, exp, 15, 10, false, 10, false);
        while (!cameraGetImageReady())
            //  fprintf(stdout,"wait\n");
            ; // waiting image
        //for (int i=0; i < 10 ; i++)
        //  for (int j=0; j < 10; j++)
        //      fprintf(stdout,"img %d/%d:%d\n",i,j,cameraGetImage(i,j));
        cameraDisconnect();
    }

}

#endif


