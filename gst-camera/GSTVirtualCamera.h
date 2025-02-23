#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files
#include <windows.h>
#include <evntprov.h>
#include <strsafe.h>
#include <initguid.h>
#include <propvarutil.h>
#include <ks.h>
#include <ksproxy.h>
#include <ksmedia.h>
#include <wincodec.h>
#include <uuids.h>

HRESULT RegisterVirtualCamera();
HRESULT StartVirtualCamera();
HRESULT UnregisterVirtualCamera();


HRESULT ProcessGstBuffer(const void* frame_data, gsize frame_size, GstClockTime pts);
HRESULT DeliverSampleToStream(IMFSample* sample);

