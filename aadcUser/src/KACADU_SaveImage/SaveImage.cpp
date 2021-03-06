/**
  Copyright (c) 
  Audi Autonomous Driving Cup. All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  3.  All advertising materials mentioning features or use of this software must display the following acknowledgement: “This product includes software developed by the Audi AG and its contributors for Audi Autonomous Driving Cup.”
  4.  Neither the name of Audi nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY AUDI AG AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL AUDI AG OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


 **********************************************************************
 * $Author:: forchhe#$  $Date:: 2014-09-11 10:10:54#$ $Rev:: 25921   $
 **********************************************************************/

// ADTF headers
#include <adtf_platform_inc.h>
#include <adtf_plugin_sdk.h>
#include <iostream>
#include <iomanip>
using namespace adtf;

// zbar headers
//#include "zbar.h"
//using namespace zbar;

#include "SaveImage.h"

// Functions for converting images to OpenCV and back:
#include "adtf2OpenCV.h"

//#include "/opt/arm-linux-gnueabihf/SDK/gcc-4.8/opencv/3.0.0/include/opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

ADTF_FILTER_PLUGIN("KACADU Save Image", OID_ADTF_KACADU_SAVEIMAGE, cSaveImage)

/*namespace {
	cv::Mat cImageToCV(const cImage& c)
	{
		//if (c.GetBitsPerPixel()) CV_8U
		int type=CV_8UC3;
		if (c.GetBitsPerPixel()==8)
			type=CV_8U;
		else if (c.GetBitsPerPixel()==16)
			type=CV_16U;
		else if (c.GetBitsPerPixel()==24)
			type=CV_8UC3;
		else
		{
			char buffer[256];
			sprintf(buffer,"Wrong bpp: %d",c.GetBitsPerPixel());
			LOG_ERROR(buffer);
		}
		//char buffer[256];
		//sprintf(buffer,"In bpp: %d",c.GetBitsPerPixel());
		//LOG_ERROR(buffer);
		return cv::Mat(c.GetHeight(),c.GetWidth(),type,c.GetBitmap());
	}
}*/

cSaveImage::cSaveImage(const tChar* __info)
	: cFilter(__info)
	, mDefaultPath("/tmp/recordedImages")
{
	SetPropertyStr("Save images to", mDefaultPath.c_str() );
	//SetPropertyBool("Save images to" NSSUBPROP_DIRECTORY, tTrue); 
	numberDepth = 0;
	numberVideo = 0;
}

cSaveImage::~cSaveImage()
{
}

tResult cSaveImage::Init(tInitStage eStage, __exception)
{
	RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));
	if (eStage == StageFirst)
	{
		//this is a VideoPin
		RETURN_IF_FAILED(m_oPinInputVideo.Create("Video_RGB_input",IPin::PD_Input, static_cast<IPinEventSink*>(this))); 
		RETURN_IF_FAILED(RegisterPin(&m_oPinInputVideo));

		RETURN_IF_FAILED(m_oPinInputDepth.Create("Depth_Image", adtf::IPin::PD_Input, static_cast<IPinEventSink*> (this)));
		RETURN_IF_FAILED(RegisterPin(&m_oPinInputDepth));

	}
	else if (eStage == StageNormal)
	{
	}
	else if (eStage == StageGraphReady)
	{
		cObjectPtr<IMediaType> pType;
		RETURN_IF_FAILED(m_oPinInputVideo.GetMediaType(&pType));

		cObjectPtr<IMediaTypeVideo> pTypeVideo;
		RETURN_IF_FAILED(pType->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid**)&pTypeVideo));

		m_sInputFormat = *(pTypeVideo->GetFormat());  


		cObjectPtr<IMediaType> pType2;
		RETURN_IF_FAILED(m_oPinInputDepth.GetMediaType(&pType2));

		cObjectPtr<IMediaTypeVideo> pTypeVideo2;
		RETURN_IF_FAILED(pType2->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid**)&pTypeVideo2));

		m_sBitmapFormatDepth = *(pTypeVideo2->GetFormat()); 

		// Set up output path:
		tChar mTmpPath[512];
		GetPropertyStr("Save images to",
				mTmpPath, 512 );
		mOutputPath.assign( mTmpPath );
		if( mOutputPath.size() == 0 )
		{
			mOutputPath = mDefaultPath;
		} else {
			mOutputPath += "/";
		}
		// Generate the output folders:
		cFileSystem::CreatePath( mOutputPath.c_str(), true );
		std::string depthPath = mOutputPath;
		depthPath += "depth/";
		cFileSystem::CreatePath( depthPath.c_str(), true );
		std::string output("Writing images to folder: ");
		output += mOutputPath;
		LOG_WARNING( output.c_str() );
		output.assign("Writing depth images to folder: ");
		output += depthPath;
		LOG_WARNING( output.c_str() );

		/*
		   m_sBitmapFormatDepth.nWidth = 640;
		   m_sBitmapFormatDepth.nHeight = 480;
		   m_sBitmapFormatDepth.nBitsPerPixel = 16;
		   m_sBitmapFormatDepth.nPixelFormat = cImage::PF_GREYSCALE_16;
		   m_sBitmapFormatDepth.nBytesPerLine = 640 * 2;
		   m_sBitmapFormatDepth.nSize = m_sBitmapFormatDepth.nBytesPerLine * 480;
		   m_sBitmapFormatDepth.nPaletteSize = 0;
		   m_oPinInputDepth.SetFormat(&m_sBitmapFormatDepth, NULL);
		 */

	}
	RETURN_NOERROR;
}

tResult cSaveImage::Shutdown(tInitStage eStage, __exception)
{		
	return cFilter::Shutdown(eStage, __exception_ptr);
}

tResult cSaveImage::OnPinEvent(IPin* pSource,
		tInt nEventCode,
		tInt nParam1,
		tInt nParam2,
		IMediaSample* pMediaSample)
{
	switch (nEventCode)
	{
		case IPinEventSink::PE_MediaSampleReceived:
			{
				if (pSource == &m_oPinInputVideo)
				{				
					ProcessVideo(pMediaSample);		
				}	
				if (pSource == &m_oPinInputDepth)
				{				
					ProcessDepth(pMediaSample);		
				}				
				break;
			}
		case IPinEventSink::PE_MediaTypeChanged:
			{
				if (pSource == &m_oPinInputVideo)
				{                
					{
						cObjectPtr<IMediaType> pType;
						RETURN_IF_FAILED(m_oPinInputVideo.GetMediaType(&pType));

						cObjectPtr<IMediaTypeVideo> pTypeVideo;
						RETURN_IF_FAILED(pType->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid**)&pTypeVideo));

						m_sInputFormat = *(pTypeVideo->GetFormat());  
					}
				}
				if (pSource == &m_oPinInputDepth)
				{                
					{
						cObjectPtr<IMediaType> pType;
						RETURN_IF_FAILED(m_oPinInputDepth.GetMediaType(&pType));

						cObjectPtr<IMediaTypeVideo> pTypeVideo;
						RETURN_IF_FAILED(pType->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid**)&pTypeVideo));

						m_sBitmapFormatDepth = *(pTypeVideo->GetFormat());  
						/*
						   m_sBitmapFormatDepth.nWidth = 640;
						   m_sBitmapFormatDepth.nHeight = 480;
						   m_sBitmapFormatDepth.nBitsPerPixel = 16;
						   m_sBitmapFormatDepth.nPixelFormat = cImage::PF_GREYSCALE_16;
						   m_sBitmapFormatDepth.nBytesPerLine = 640 * 2;
						   m_sBitmapFormatDepth.nSize = m_sBitmapFormatDepth.nBytesPerLine * 480;
						   m_sBitmapFormatDepth.nPaletteSize = 0;
						   m_oPinInputDepth.SetFormat(&m_sBitmapFormatDepth, NULL);
						 */
					}
				}
				break;
			}
		default:
			{
				break;
			}
	}
	RETURN_NOERROR;
}

tResult cSaveImage::ProcessVideo(adtf::IMediaSample* pISample)
{
	const tVoid* l_pSrcBuffer;
	adtf::cScopedSampleReadLock rl1(pISample, &l_pSrcBuffer);

	cImage iImage;
	RETURN_IF_FAILED(iImage.Attach((tUInt8*)l_pSrcBuffer, &m_sInputFormat, NULL));
	cv::Mat image;
	image = cImageToCV(iImage);
	stringstream ss;
	ss << mOutputPath;
	//ss << pISample->GetTime();
	ss<<setfill('0')<<setw(7)<<numberVideo;
	ss << ".png";
	cv::imwrite(ss.str().c_str(), image);	
	numberVideo++;
	RETURN_NOERROR;
}

tResult cSaveImage::ProcessDepth(adtf::IMediaSample* pISample)
{
	const tVoid* l_pSrcBuffer;
	adtf::cScopedSampleReadLock rl1(pISample, &l_pSrcBuffer);

	cImage iImage;
	RETURN_IF_FAILED(iImage.Attach((tUInt8*)l_pSrcBuffer, &m_sBitmapFormatDepth, NULL));
	cv::Mat image;
	image=cImageToCV(iImage);
	//std::cout<<"ImageDepth: "<<image.depth()<<std::endl;
	//std::cout<<"ImageDepth soll: "<<CV_16U<<std::endl;
	stringstream ss;
	ss << mOutputPath;
	ss << "depth/";
	//ss << pISample->GetTime();
	ss<<setfill('0')<<setw(7)<<numberDepth;
	ss << ".png";
	cv::imwrite(ss.str().c_str(), image);	
	numberDepth++;
	RETURN_NOERROR;
}

tResult cSaveImage::Start(__exception)
{
	return cFilter::Start(__exception_ptr);
}

tResult cSaveImage::Stop(__exception)
{
	return cFilter::Stop(__exception_ptr);
}

