#include "pch.h"

#include <xstring>

#include <winrt\base.h>

#include <gst\gstbuffer.h>

#include <wrl.h>

using namespace Microsoft::WRL;

#include "GSTVirtualCamera.h"

#define S_FAIL ((HRESULT)-1L)


// regsvr32 C:\Dev\WindowsWDK\gst-camera\x64\Debug\mf_camera.dll (you must run this as administrator)
static GUID CLSID_VCam = { 0x79c2dd91,0x230b, 0x419f, {0xb3,0x56,0x89,0x5d,0xc3,0x29,0x71,0x6d} };
WCHAR title[100] = L"GSTVirtualCamera";

ComPtr<IMFVirtualCamera> _vcam;
IMFSourceReader* _sourceReader = nullptr;

const std::wstring GUID_ToStringW(const GUID& guid)
{
	wchar_t name[64];
	std::ignore = StringFromGUID2(guid, name, _countof(name));
	return name;
}


HRESULT RegisterVirtualCamera()
{
	if (_vcam)
		return S_OK;

	// Create the virtual camera
    GUID categories[] = { MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID };
    auto clsid = GUID_ToStringW(CLSID_VCam);

    // Create attributes for the virtual camera
    ComPtr<IMFAttributes> attributes;
    HRESULT hr = MFCreateVirtualCamera(
		MFVirtualCameraType_SoftwareCameraSource,
		MFVirtualCameraLifetime_Session,
		MFVirtualCameraAccess_CurrentUser,
		title,
		clsid.c_str(),
        categories,
        ARRAYSIZE(categories),
		&_vcam);

    if (FAILED(hr)) {
		g_print("Failed to create virtual camera. %d\n", hr);
		return S_FAIL;
	}
	g_print("RegisterVirtualCamera '%S' ok!\n", clsid.c_str());

    _vcam->AddDeviceSourceInfo(clsid.c_str());

	if (FAILED(_vcam->Start(nullptr))) {
		g_print("RegisterVirtualCamera: Cannot start VCam!\n");
		return S_FAIL;
	};

	g_print("RegisterVirtualCamera: Success!\n");
    return S_OK;
}

HRESULT UnregisterVirtualCamera()
{
	if (!_vcam)
		return S_OK;

    if (_sourceReader) {
        _sourceReader->Release();
        _sourceReader = nullptr;
    }
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