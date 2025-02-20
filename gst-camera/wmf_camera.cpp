#include "pch.h"

#include <xstring>

#include <winrt\base.h>

#include <gst\gstbuffer.h>

#include "wmf_camera.h"

#define S_FAIL ((HRESULT)-1L)


// regsvr32 C:\Dev\WindowsWDK\VCamSample\x64\Debug\VCamSampleSource.dll (you must run this as administrator)
static GUID CLSID_VCam = { 0x3cad447d,0xf283,0x4af4,{0xa3,0xb2,0x6f,0x53,0x63,0x30,0x9f,0x52} };

WCHAR title[100] = L"WMFVirtualCamera";

IMFVirtualCamera* _vcam = 0;
IMFSourceReader* _sourceReader = nullptr;
IMFStreamSink* streamSink = nullptr;

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

	auto clsid = GUID_ToStringW(CLSID_VCam);
	HRESULT hr = MFCreateVirtualCamera(
		MFVirtualCameraType_SoftwareCameraSource,
		MFVirtualCameraLifetime_Session,
		MFVirtualCameraAccess_CurrentUser,
		title,
		clsid.c_str(),
		nullptr,
		0,
		&_vcam);
	if (FAILED(hr)) {
		g_print("Failed to create virtual camera.\n");
		return S_FAIL;
	}
	g_print("RegisterVirtualCamera '%S' ok\n", clsid.c_str());

	if (FAILED(_vcam->Start(nullptr))) {
		g_print("RegisterVirtualCamera: Cannot start VCam!\n");
		return S_FAIL;
	};


    IMFMediaSource* mediaSource = nullptr;
    if (SUCCEEDED(_vcam->GetMediaSource(&mediaSource)))
    {
        g_print("RegisterVirtualCamera: Got mediaSource! :D\n");
        // Create the source reader
        if (FAILED(MFCreateSourceReaderFromMediaSource(mediaSource, nullptr, &_sourceReader))) {
            g_print("Failed to create source reader.\n");
            return S_FAIL;
        }
        else
        {
            g_print("RegisterVirtualCamera: created source reader! :D\n");
        }
    }
    else
    {
        g_print("RegisterVirtualCamera: Unable to get mediaSource :(\n");
    }

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
	_vcam = 0;
	return S_OK;
}

HRESULT ProcessFrame(GstBuffer* buf)
{
    if (!_sourceReader)
        return S_FAIL;

    // Map the GstBuffer to access the frame data
    GstMapInfo map;
    if (!gst_buffer_map(buf, &map, GST_MAP_READ))
    {
        g_print("Failed to map GstBuffer.\n");
        return S_FAIL;
    }

    // Create a memory buffer over the mapped data
    IMFMediaBuffer* mediaBuffer = nullptr;
    HRESULT hr = MFCreateMemoryBuffer((DWORD)map.size, &mediaBuffer);
    if (FAILED(hr))
    {
        g_print("Failed to create memory buffer.\n");
        gst_buffer_unmap(buf, &map);
        return S_FAIL;
    }

    // Copy the data to the media buffer
    BYTE* bufferData = nullptr;
    DWORD maxLength = 0, currentLength = 0;
    hr = mediaBuffer->Lock(&bufferData, &maxLength, &currentLength);
    if (SUCCEEDED(hr))
    {
        memcpy(bufferData, map.data, map.size);
        mediaBuffer->Unlock();
        mediaBuffer->SetCurrentLength(map.size);
    }
    else
    {
        g_print("Failed to lock media buffer.\n");
        mediaBuffer->Release();
        gst_buffer_unmap(buf, &map);
        return S_FAIL;
    }

    // Create a sample and add the media buffer to it
    IMFSample* sample = nullptr;
    hr = MFCreateSample(&sample);
    if (SUCCEEDED(hr))
    {
        sample->AddBuffer(mediaBuffer);
        mediaBuffer->Release();

        // Process the sample using the source reader
        hr = _sourceReader->ReadSample(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            0,
            nullptr,
            nullptr,
            nullptr,
            &sample);
        sample->Release();
    }
    else
    {
        g_print("Failed to create sample.\n");
        mediaBuffer->Release();
    }

    gst_buffer_unmap(buf, &map);
    return hr;
}

// Function to inject a sample into a stream sink
HRESULT InjectSample(IMFStreamSink* streamSink, GstBuffer* buf)
{
    // Map the GstBuffer to access the frame data
    GstMapInfo map;
    if (!gst_buffer_map(buf, &map, GST_MAP_READ))
    {
        g_print("Failed to map GstBuffer.\n");
        return S_FAIL;
    }

    // Create a memory buffer over the mapped data
    IMFMediaBuffer* mediaBuffer = nullptr;
    HRESULT hr = MFCreateMemoryBuffer((DWORD)map.size, &mediaBuffer);
    if (FAILED(hr))
    {
        g_print("Failed to create memory buffer.\n");
        gst_buffer_unmap(buf, &map);
        return S_FAIL;
    }

    // Copy the data to the media buffer
    BYTE* bufferData = nullptr;
    DWORD maxLength = 0, currentLength = 0;
    hr = mediaBuffer->Lock(&bufferData, &maxLength, &currentLength);
    if (SUCCEEDED(hr))
    {
        memcpy(bufferData, map.data, map.size);
        mediaBuffer->Unlock();
        mediaBuffer->SetCurrentLength((DWORD)map.size);
    }
    else
    {
        g_print("Failed to lock media buffer.\n");
        mediaBuffer->Release();
        gst_buffer_unmap(buf, &map);
        return S_FAIL;
    }

    // Create a sample and add the media buffer to it
    IMFSample* sample = nullptr;
    hr = MFCreateSample(&sample);
    if (SUCCEEDED(hr))
    {
        sample->AddBuffer(mediaBuffer);
        mediaBuffer->Release();

        // Set the sample time and duration (optional)
        // sample->SetSampleTime(...);
        // sample->SetSampleDuration(...);

        // Process the sample using the stream sink
        hr = streamSink->ProcessSample(sample);
        sample->Release();
    }
    else
    {
        g_print("Failed to create sample.\n");
        mediaBuffer->Release();
    }

    gst_buffer_unmap(buf, &map);
    return hr;
}

#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfsensorgroup")
#pragma comment(lib, "comctl32")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")