/**********************************************************************
* Copyright (c) 2013 BFFT Gesellschaft fuer Fahrzeugtechnik mbH.
* All rights reserved.
**********************************************************************
* $Author:: spiesra $  $Date:: 2015-05-13 08:29:07#$ $Rev:: 35003   $
**********************************************************************/

#define DEPTH_CX 319.5
#define DEPTH_CY 239.5
#define DEPTH_FX 525.0
#define DEPTH_FY 525.0

/*! class xtionGrabber */
class xtionGrabber
{
private:

    openni::Device m_Device;

    openni::VideoStream m_DepthStream;
    openni::VideoStream m_ColorStream;

    std::vector <openni::VideoStream*> m_Streams;

    openni::VideoMode m_DepthVideoMode;
    openni::VideoMode m_ColorVideoMode;

    openni::VideoFrameRef m_DepthFrame;
    openni::VideoFrameRef m_ColorFrame;

    tBool m_GetDepth;
    tBool m_GetColor;
    tBool m_ImageRegistrationEnabled;
    tBool m_IsDepthColorSyncEnabled;
    tBool m_IsInitialized;
    tBool m_IsRunning;
    tBool m_IsAutoExposureEnabled;
    tBool m_IsAutoWhiteBalanceEnabled;

    tInt m_NumStreams;
    tInt m_DepthStreamIndex;
    tInt m_ColorStreamIndex;

public:

    xtionGrabber() {};
    ~xtionGrabber()
    {
        m_DepthStream.stop();
        m_ColorStream.stop();
        m_DepthStream.destroy();
        m_ColorStream.destroy();
        m_Streams.clear();
        m_Device.close();

        // delete m_DepthFrame and m_ColorFrame?
        openni::OpenNI::shutdown();
    };
    /*! initialize the grabber for depth and/or color acquisition
    @param depth_xResolution the x resolution of the depth stream
    @param depth_yResolution the x resolution of the depth stream
    @param depth_fps the frames per second of the depth stream
    @param color_xResolution the x resolution of the video stream 
    @param color_yResolution the y resolution of the video stream
    @param color_fps the frames per second of the video stream
    @param getDepth flag for enabling or disabling the depth stream
    @param getColor flag for enabling or disabling the video stream
    @param setRegistration flag for enabling or disabling the registration mode
    @param setDepthColorSync flag for enabling or disabling the depth - color sync mode
    */
    tBool initialize(tInt depth_xResolution, tInt depth_yResolution, tInt depth_fps, tInt color_xResolution, tInt color_yResolution, tInt color_fps, tBool getDepth, tBool getColor, tBool setDepthInMillimeter, tBool setRegistration, tBool setDepthColorSync, tBool IsAutoExposureEnabled, tBool IsAutoWhiteBalanceEnabled);

    /*! returns true if the grabber is initialized otherwise false*/
    tBool isInitialized();
    /*! returns true if the grabber is running otherwise false*/
    tBool isRunning();
  
    /*! sets the pointer to the depth image data
    @param ptrDest the pointer to be set
    @param size size of the data
    */
    tBool getDepthFramePtr(openni::DepthPixel*& ptrDest, tInt& size);    
    /*! sets the pointer to the video image data
    @param ptrDest the pointer to be set
    @param size size of the data
    */
    tBool getColorFramePtr(openni::RGB888Pixel*& ptrDest, tInt& size);

    /*! starts the grabber */
    tBool start();
    /*! stops the grabber */
    tVoid stop();
};

tBool xtionGrabber::initialize(tInt depth_xResolution=640, tInt depth_yResolution=480, tInt depth_fps=30, tInt color_xResolution=640, tInt color_yResolution=480, tInt color_fps=30, tBool getDepth = tTrue, tBool getColor = tTrue, tBool setDepthInMillimeter= tFalse, tBool setRegistration = tTrue, tBool setDepthColorSync = tTrue, tBool IsAutoExposureEnabled = tFalse, tBool IsAutoWhiteBalanceEnabled = tFalse)
{
    m_ImageRegistrationEnabled = tFalse;
    m_GetDepth = tFalse;
    m_GetColor = tFalse;
    m_IsInitialized = tFalse;
    m_IsRunning = tFalse;
    m_NumStreams = 0;
    m_DepthStreamIndex = 0;
    m_ColorStreamIndex = 0;

    m_IsAutoExposureEnabled = IsAutoExposureEnabled;
    m_IsAutoWhiteBalanceEnabled = IsAutoWhiteBalanceEnabled;

    // return if no stream is specified
    if(!getDepth && !getColor)
    {
        std::cout << "No acquisition streams specified! " << std::endl;
        return tFalse;
    }

    // initialize the OpenNI API
    openni::OpenNI::initialize();

    
    // return tFalse if unable to initialize the device
    if (m_Device.open(openni::ANY_DEVICE) != openni::STATUS_OK)
    {
        std::cout << "Unable to initialize OpenNI device!" << std::endl;
        openni::OpenNI::shutdown();
        return tFalse;
    }


    // initialize the stream pointer array
    m_GetDepth = getDepth;
    m_GetColor = getColor;
    if(m_GetDepth && m_GetColor)
    {
        m_NumStreams = 2;
        m_DepthStreamIndex = 0;
        m_ColorStreamIndex = 1;
    }
    else if(m_GetDepth && !m_GetColor)
    {
        m_NumStreams = 1;
        m_DepthStreamIndex = 0;
    }
    else if(!m_GetDepth && m_GetColor)
    {
        m_NumStreams = 1;
        m_ColorStreamIndex = 0;
    }

    // attempt to start stream acquisition
    try
    {
        // initialize the depth stream if necessary
        if(m_GetDepth)
        {
            m_DepthStream.create(m_Device, openni::SENSOR_DEPTH);
            m_DepthVideoMode = m_DepthStream.getVideoMode();
            m_DepthVideoMode.setResolution(depth_xResolution, depth_yResolution);
            m_DepthVideoMode.setFps(depth_fps);
            if (setDepthInMillimeter)
               m_DepthVideoMode.setPixelFormat(openni::PIXEL_FORMAT_DEPTH_1_MM);
            m_DepthStream.setVideoMode(m_DepthVideoMode);
            m_DepthStream.setMirroringEnabled(tFalse);

            // update the pointer array
            m_Streams.push_back(&m_DepthStream);
        }

        // initialize the color stream if necessary
        if(m_GetColor)
        {
            m_ColorStream.create(m_Device, openni::SENSOR_COLOR);
            openni::VideoMode m_ColorVideoMode = m_ColorStream.getVideoMode();
            m_ColorVideoMode.setResolution(color_xResolution, color_yResolution);            
            m_ColorVideoMode.setPixelFormat(openni::PIXEL_FORMAT_RGB888);
            m_ColorVideoMode.setFps(color_fps);
            m_ColorStream.setVideoMode(m_ColorVideoMode);
            m_ColorStream.setMirroringEnabled(tFalse);
            m_ColorStream.getCameraSettings()->setAutoExposureEnabled(m_IsAutoExposureEnabled);
   	        m_ColorStream.getCameraSettings()->setAutoWhiteBalanceEnabled(m_IsAutoWhiteBalanceEnabled);
std::cout<<"Exposure!!!!!!!!!!"<<m_ColorStream.getCameraSettings()->getExposure()<<std::endl;
openni::Status rc	=	m_ColorStream.getCameraSettings()->setExposure(3392);
if (rc != openni::STATUS_OK)
	{
		std::cout<<"Can't change exposure"<<std::endl;

	}
std::cout<<"Exposure!!!!!!!!!!"<<m_ColorStream.getCameraSettings()->getExposure()<<std::endl;
std::cout<<"Gain!!!!!!!!!!"<<m_ColorStream.getCameraSettings()->getGain()<<std::endl;
rc	=	m_ColorStream.getCameraSettings()->setGain(3392);
if (rc != openni::STATUS_OK)
	{
		std::cout<<"Can't change gain"<<std::endl;

	}
std::cout<<"Gain!!!!!!!!!!"<<m_ColorStream.getCameraSettings()->getGain()<<std::endl;

            // update the pointer array
            m_Streams.push_back(&m_ColorStream);
std::cout<<"AutoExp"<<m_ColorStream.getCameraSettings()->getAutoExposureEnabled()<<std::endl;
std::cout<<"AutoWhite"<<m_ColorStream.getCameraSettings()->getAutoWhiteBalanceEnabled()<<std::endl;
        }

        // set depth color synchronization if necessary
        if(setDepthColorSync & m_GetDepth && m_GetColor)
        {
            tInt status = m_Device.setDepthColorSyncEnabled(tTrue);
            if (status != openni::STATUS_OK)
            {
                std::cout << "Unable to set depth/color synchronization: " << openni::OpenNI::getExtendedError() << std::endl;
                m_IsDepthColorSyncEnabled = tFalse;
            }
            else
            {
                m_IsDepthColorSyncEnabled = tTrue;
                std::cout << "Depth/color synchronization enabled!" << std::endl;
            }
        }

        // set depth to color registration mode
        if(setRegistration && m_Device.isImageRegistrationModeSupported(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR))
        {
            tInt status = m_Device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);
            if (status != openni::STATUS_OK)
            {
                std::cout << "Unable to set registration mode: " << openni::OpenNI::getExtendedError() << std::endl;
                m_ImageRegistrationEnabled = tFalse;
            }
            else
            {
                m_ImageRegistrationEnabled = tTrue;
                std::cout << "Registration mode enabled!" << std::endl;
            }
        }
        else
        {
            std::cout << "Registration not supported by this device!" << std::endl;
            m_ImageRegistrationEnabled = tFalse;
        }

        // streams opened with no error, return tTrue
        m_IsInitialized = tTrue;

        return tTrue;
    }
    catch(char* error)
    {
        // we encountered an error, return tFalse
        std::cout << "Exception occured while opening stream: " << error << std::endl;
        m_ImageRegistrationEnabled = tFalse;
        m_GetDepth = tFalse;
        m_GetColor = tFalse;
        m_IsInitialized = tFalse;
        m_IsRunning = tFalse;
        m_NumStreams = 0;
        m_DepthStreamIndex = 0;
        m_ColorStreamIndex = 0;
        m_Device.close();
        openni::OpenNI::shutdown();

        return tFalse;
    }
}

tBool xtionGrabber::isInitialized()
{
    return m_IsInitialized;
}

tBool xtionGrabber::isRunning()
{
    return m_IsRunning;
}


tBool xtionGrabber::getDepthFramePtr(openni::DepthPixel*& ptrDest, tInt& size)
{
    // read the frame from the depth stream
    m_DepthStream.readFrame(&m_DepthFrame);
    
    ptrDest = (openni::DepthPixel*) m_DepthFrame.getData();
    size = tInt(m_DepthFrame.getStrideInBytes() * m_DepthFrame.getHeight());
    return tTrue;
}

tBool xtionGrabber::getColorFramePtr(openni::RGB888Pixel*& ptrDest, tInt& size)
{

    // read the frame from the color stream

static int counter = 0;
static int gain = 64;
counter++;
/*
if ((counter == 64)){
m_ColorStream.getCameraSettings()->setAutoExposureEnabled(true);
m_ColorStream.getCameraSettings()->setAutoWhiteBalanceEnabled(true);
//m_ColorStream.getCameraSettings()->setExposure(1);
//m_ColorStream.getCameraSettings()->setGain(100);
std::cout<<"enable Auto"<<std::endl;
}else if ((counter == 128))
{
m_ColorStream.getCameraSettings()->setAutoExposureEnabled(false);
m_ColorStream.getCameraSettings()->setAutoWhiteBalanceEnabled(false);
m_ColorStream.getCameraSettings()->setExposure(1);
m_ColorStream.getCameraSettings()->setGain(100);
std::cout<<"disable Auto"<<std::endl;

}
   

else if ((counter > 256)){
std::cout<<"AutoExp"<<m_ColorStream.getCameraSettings()->getAutoExposureEnabled()<<std::endl;
std::cout<<"AutoWhite"<<m_ColorStream.getCameraSettings()->getAutoWhiteBalanceEnabled()<<std::endl;
m_ColorStream.getCameraSettings()->setExposure(8);
m_ColorStream.getCameraSettings()->setGain(gain);
std::cout<<"gain"<<gain<<std::endl;
gain = gain*4;

counter = 0;
}*/

if ((counter == 2)){
m_ColorStream.getCameraSettings()->setAutoExposureEnabled(false);
m_ColorStream.getCameraSettings()->setAutoWhiteBalanceEnabled(false);
//m_ColorStream.getCameraSettings()->setExposure(1);
//m_ColorStream.getCameraSettings()->setGain(100);
std::cout<<"disable Auto"<<std::endl;
}
else if ((counter == 4)){
std::cout<<"AutoExp"<<m_ColorStream.getCameraSettings()->getAutoExposureEnabled()<<std::endl;
std::cout<<"AutoWhite"<<m_ColorStream.getCameraSettings()->getAutoWhiteBalanceEnabled()<<std::endl;
m_ColorStream.getCameraSettings()->setExposure(8);
m_ColorStream.getCameraSettings()->setGain(256);
}


  m_ColorStream.readFrame(&m_ColorFrame);    
    ptrDest =  (openni::RGB888Pixel*) m_ColorFrame.getData();
    size = tInt(m_ColorFrame.getStrideInBytes() * m_ColorFrame.getHeight());
    return tTrue;
}

tBool xtionGrabber::start()
{
    if(!m_IsInitialized)
    {
        std::cout << "Grabber is not initialized, unable to start acquisition!" << std::endl;
        return tFalse;
    }
    else
    {
        // start the depth stream if necessary
        if(m_GetDepth)
        {
            if(m_DepthStream.start() != openni::STATUS_OK)
            {
                std::cout << "Unable to start depth stream!" << std::endl;
                return tFalse;
            }
        }

        // start the color stream if necessary
        if(m_GetColor)
        {
            if (m_ColorStream.start() != openni::STATUS_OK)
            {
                std::cout << "Unable to start color stream!" << std::endl;
                return tFalse;
            }


        }

        // the streams were started successfully
        m_IsRunning = tTrue;
        return tTrue;
    }
}

tVoid xtionGrabber::stop()
{
    // stop the depth stream if necessary
    if(m_GetDepth)
    {
        m_DepthStream.stop();
    }

    // stop the color stream if necessary
    if(m_GetColor)
    {
        m_ColorStream.stop();
    }
}
