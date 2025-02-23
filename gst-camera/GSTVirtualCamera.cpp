#include "pch.h"

#include <xstring>

#include <winrt\base.h>

#include <gst\gstbuffer.h>

#include <wrl.h>

#include "GSTGuids.h"

using namespace Microsoft::WRL;

#include "GSTVirtualCamera.h"
#include "GSTMediaSource.h"
#include "GSTMediaStream.h"

#define S_FAIL ((HRESULT)-1L)


// regsvr32 C:\Dev\WindowsWDK\gst-camera\x64\Debug\mf_camera.dll (you must run this as administrator)
WCHAR title[100] = L"GSTVirtualCamera";

ComPtr<IMFVirtualCamera> _vcam;
ComPtr<GSTMediaSource> _media_source;
ComPtr<GSTMediaStream> _stream;

HRESULT RegisterVirtualCamera()
{
	if (_vcam)
		return S_OK;
    // Initialize Media Foundation
    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr)) {
        printf("MFStartup failed. %d\n", hr);
        return hr;
    }

	// Create the virtual camera
    GUID categories[] = { MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID };
    auto clsid = GUID_ToStringW(CLSID_GSTVirtualCamera);
    //auto clsid = GUID_ToStringW(CLSID_VCam);

    // Create attributes for the virtual camera
    ComPtr<IMFAttributes> attributes;
    hr = MFCreateAttributes(&attributes, 1);
    if (FAILED(hr)) {
        printf("MFCreateAttributes failed. %d\n", hr);
        return hr;
    }

    // Associate the virtual camera with GSTMediaSourceActivate CLSID
    hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_PROVIDER_DEVICE_ID, CLSID_GSTVirtualCamera);
    if (FAILED(hr)) {
        printf("SetGUID failed. %d\n", hr);
        return hr;
    }

	hr = MFCreateVirtualCamera(
		MFVirtualCameraType_SoftwareCameraSource,
		MFVirtualCameraLifetime_Session,
		MFVirtualCameraAccess_CurrentUser,
		title,
		clsid.c_str(),
        0, //categories,
        NULL, //ARRAYSIZE(categories),
		&_vcam);

    if (FAILED(hr)) {
		printf("Failed to create virtual camera. %d\n", hr);
		return S_FAIL;
	}
	printf("RegisterVirtualCamera '%S' ok!\n", clsid.c_str());

	//if (FAILED(_vcam->AddDeviceSourceInfo(clsid.c_str())))
	//{
	//	printf("RegisterVirtualCamera: Cannot add device source info!\n");
	//	return S_FAIL;
	//}   


	printf("RegisterVirtualCamera: Success!\n");
    return S_OK;
}

HRESULT StartVirtualCamera()
{
    printf("StartVirtualCamera()\n");
    if (!_vcam)
    {
        printf("StartVirtualCamera() -- Fail, _vcam is not initialised!!\n");
        return S_FAIL;
    }
    if (FAILED(_vcam->Start(nullptr))) {
    	printf("RegisterVirtualCamera: Cannot start VCam!\n");
    	return S_FAIL;
    };

    //IMFMediaSource* source;
    //if (FAILED(_vcam->GetMediaSource(&source)))
    //{
    //    _media_source = dynamic_cast<GSTMediaSource*>(source);
    //    if (FAILED(_media_source->GetStream(&_stream)))
    //    {
    //        printf("StartVirtualCamera() - Failed to get stream!\n");
    //    }
    //    else
    //    {
    //        printf("StartVirtualCamera() - Got Stream\n");
    //    }
    //}
    //else
    //{
	   // printf("StartVirtualCamera() - Failed to get media source!\n");
    //}

    printf("StartVirtualCamera() - Started!\n");
    return S_OK;
}


HRESULT UnregisterVirtualCamera()
{
	if (!_vcam)
		return S_OK;

    // NOTE: we don't call Shutdown or this will cause 2 Shutdown calls to the media source and will prevent proper removing
    //auto hr = _vcam->Shutdown();
    auto hr = _vcam->Remove();
	_vcam = nullptr;
	return S_OK;
}

// Function to deliver a sample to the source reader
HRESULT ProcessGstBuffer(const void* frame_data, gsize frame_size, GstClockTime pts) {
    // Create an IMFSample
    IMFSample* sample = nullptr;
    HRESULT hr = MFCreateSample(&sample);
    if (FAILED(hr)) return hr;

    // Create an IMFMediaBuffer to hold the frame data
    IMFMediaBuffer* buffer = nullptr;
    hr = MFCreateMemoryBuffer(frame_size, &buffer);
    if (FAILED(hr)) {
        sample->Release();
        return hr;
    }

    // Copy the frame data into the IMFMediaBuffer
    BYTE* pBufferData = nullptr;
    hr = buffer->Lock(&pBufferData, nullptr, nullptr);
    if (SUCCEEDED(hr)) {
        memcpy(pBufferData, frame_data, frame_size);
        buffer->Unlock();
    }

    // Add the buffer to the sample
    if (SUCCEEDED(hr)) {
        hr = sample->AddBuffer(buffer);
    }

    // Set the sample timestamp (in 100-nanosecond units)
    if (SUCCEEDED(hr) && pts != GST_CLOCK_TIME_NONE) {
        LONGLONG sampleTime = pts / 100; // Convert to 100-nanosecond units
        hr = sample->SetSampleTime(sampleTime);
    }

    // Clean up
    buffer->Release();

    // Pass the sample to the media stream
    if (SUCCEEDED(hr)) {
        hr = DeliverSampleToStream(sample);
    }

    sample->Release();
    return hr;
}

// Function to deliver a sample to the source reader
HRESULT DeliverSampleToStream(IMFSample* sample) {
    printf("DeliverSampleToStream(...)\n");
    //if (!g_mediaStream) { // g_mediaStream is your IMFMediaStream2 instance
    //    return E_FAIL;
    //}

    //// Deliver the sample to the stream
    //return g_mediaStream->ProcessSample(sample);
    return S_OK;
}


#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfsensorgroup")
#pragma comment(lib, "comctl32")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")