#pragma once

#include "pch.h"

//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
// GSTMediaStream
//------------------------------------------------------------------------------------------------------------------------------------
#include <mferror.h>
#include <wrl.h>
#include <queue>


using namespace Microsoft::WRL;
//------------------------------------------------------------------------------------------------------------------------------------
class GSTMediaStream : public IMFMediaStream2
{
private:
    long _refCount;
    IMFMediaEventQueue* _eventQueue;
    IMFAttributes* _attributes;
    IMFMediaType* _mediaType;
    IMFMediaSource* _source;
    std::queue<ComPtr<IMFSample>> _sampleQueue;
    bool _isActive;

public:
	GSTMediaStream() : _refCount(1), _eventQueue(nullptr), _attributes(nullptr), _mediaType(nullptr), _isActive(false), _source(nullptr)
    {
        g_print("GSTMediaStream::GSTMediaStream()\n");
        MFCreateEventQueue(&_eventQueue);
    }

    ~GSTMediaStream()
    {
        if (_eventQueue) _eventQueue->Shutdown();
        if (_attributes) _attributes->Release();
        if (_mediaType) _mediaType->Release();
    }

    HRESULT Initialize(IMFMediaSource* source)
    {
        g_print("GSTMediaStream::Initialize()\n");
		_source = source;
		return S_OK;
    }

    // IUnknown Implementation
    ULONG STDMETHODCALLTYPE AddRef() override
    {
        return InterlockedIncrement(&_refCount);
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG uCount = InterlockedDecrement(&_refCount);
        if (uCount == 0) delete this;
        return uCount;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv) override
    {
        if (!ppv) return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_IMFMediaEventGenerator || riid == IID_IMFMediaStream || riid == IID_IMFMediaStream2)
        {
            *ppv = static_cast<IMFMediaStream2*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    // IMFMediaEventGenerator Implementation
    HRESULT STDMETHODCALLTYPE BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState) override
    {
        return _eventQueue->BeginGetEvent(pCallback, punkState);
    }

    HRESULT STDMETHODCALLTYPE EndGetEvent(IMFAsyncResult* pResult, IMFMediaEvent** ppEvent) override
    {
        return _eventQueue->EndGetEvent(pResult, ppEvent);
    }

    HRESULT STDMETHODCALLTYPE GetEvent(DWORD dwFlags, IMFMediaEvent** ppEvent) override
    {
        return _eventQueue->GetEvent(dwFlags, ppEvent);
    }

    HRESULT STDMETHODCALLTYPE QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT* pvValue) override
    {
        return _eventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
    }

    // IMFMediaStream2 Implementation
    HRESULT STDMETHODCALLTYPE GetMediaSource(IMFMediaSource** ppMediaSource) override
    {
        g_print("GSTMediaStream::GetMediaSource(...)\n");
		//return _source->QueryInterface(IID_PPV_ARGS(ppMediaSource));
        *ppMediaSource = _source;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetStreamDescriptor(IMFStreamDescriptor** ppStreamDescriptor) override
    {
        g_print("GSTMediaStream::GetStreamDescriptor(...)\n");
        return MF_E_NOT_AVAILABLE;  //MF_E_NOT_IMPLEMENTED; // Implement if needed
    }

    HRESULT STDMETHODCALLTYPE RequestSample(IUnknown* pToken) override
    {
        g_print("GSTMediaStream::RequestSample(...)\n");
        if (_sampleQueue.empty())
            return MF_E_NO_MORE_TYPES;

        // Get next sample
        ComPtr<IMFSample> sample = _sampleQueue.front();
        _sampleQueue.pop();

        bool wasSelected = true;
        MediaEventType met = (wasSelected ? MEUpdatedStream : MENewStream);

        // Deliver it to Media Foundation
        return _eventQueue->QueueEventParamUnk(met, GUID_NULL, S_OK, sample.Get());
    }

    HRESULT STDMETHODCALLTYPE SetStreamState(MF_STREAM_STATE state) override
    {
        g_print("GSTMediaStream::SetStreamState(...)\n");
        if (state == MF_STREAM_STATE_RUNNING)
        {
            _isActive = true;
            QueueEvent(MEStreamStarted, GUID_NULL, S_OK, nullptr);
        }
        else if (state == MF_STREAM_STATE_STOPPED)
        {
            _isActive = false;
            QueueEvent(MEStreamStopped, GUID_NULL, S_OK, nullptr);
        }
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetStreamState(MF_STREAM_STATE* pState) override
    {
        if (!pState) return E_POINTER;
        *pState = _isActive ? MF_STREAM_STATE_RUNNING : MF_STREAM_STATE_STOPPED;
        return S_OK;
    }

    HRESULT ProcessSample(IMFSample* pSample)
    {
        g_print("GSTMediaStream::ProcessSample(...)\n");
        if (!_isActive || !pSample)
            return MF_E_INVALIDREQUEST;

        _sampleQueue.push(pSample);
        pSample->AddRef();

        // Notify the media pipeline that a new sample is available
        // The correct event type to use is MEStreamTick or MEStreamSinkSampleRequest, depending on the use case.
        return _eventQueue->QueueEventParamUnk(MEUnknown, GUID_NULL, S_OK, pSample);
    }
};

