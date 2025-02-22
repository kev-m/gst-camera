#include "pch.h"

// Show me an implementation of CustomMediaSource, which implements IMFMediaSource2, and show me how to attach this CustomMediaSource to

//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
// GSTMediaStream
//------------------------------------------------------------------------------------------------------------------------------------
#include <mferror.h>
#include <wrl.h>
#include <atomic>
#include <queue>

#include "GSTVirtualCamera.h"

using namespace Microsoft::WRL;
//------------------------------------------------------------------------------------------------------------------------------------
class GSTMediaStream : public IMFMediaStream2
{
private:
    long _refCount;
    IMFMediaEventQueue* _eventQueue;
    IMFAttributes* _attributes;
    IMFMediaType* _mediaType;
    std::queue<ComPtr<IMFSample>> _sampleQueue;
    bool _isActive;

public:
    GSTMediaStream() : _refCount(1), _eventQueue(nullptr), _attributes(nullptr), _mediaType(nullptr), _isActive(false)
    {
        MFCreateEventQueue(&_eventQueue);
    }

    ~GSTMediaStream()
    {
        if (_eventQueue) _eventQueue->Shutdown();
        if (_attributes) _attributes->Release();
        if (_mediaType) _mediaType->Release();
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
        return MF_E_NOT_AVAILABLE;  //MF_E_NOT_IMPLEMENTED; // Implement if needed
    }

    HRESULT STDMETHODCALLTYPE GetStreamDescriptor(IMFStreamDescriptor** ppStreamDescriptor) override
    {
        return MF_E_NOT_AVAILABLE;  //MF_E_NOT_IMPLEMENTED; // Implement if needed
    }

    HRESULT STDMETHODCALLTYPE RequestSample(IUnknown* pToken) override
    {
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
        if (!_isActive || !pSample)
            return MF_E_INVALIDREQUEST;

        _sampleQueue.push(pSample);
        pSample->AddRef();

        // Notify the media pipeline that a new sample is available
        // The correct event type to use is MEStreamTick or MEStreamSinkSampleRequest, depending on the use case.
        return _eventQueue->QueueEventParamUnk(MEUnknown, GUID_NULL, S_OK, pSample);
    }
};

//------------------------------------------------------------------------------------------------------------------------------------
// GSTMediaSource
//------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------
class GSTMediaSource : public IMFMediaSource2
{
private:
    std::atomic<ULONG> _refCount; // Reference counter
    IMFMediaStream2* _mediaStream;
    bool _isStarted;

public:
    GSTMediaSource() : _refCount(1), _mediaStream(nullptr), _isStarted(false) {}

    HRESULT Start(IMFAsyncCallback* pCallback)
    {
        if (!_mediaStream)
        {
            _mediaStream = new GSTMediaStream(); // Create the media stream
        }
        _isStarted = true;

        // Notify that the stream is ready
        if (pCallback) pCallback->Invoke(nullptr);

        return S_OK;
    }

    HRESULT GetStream(IMFMediaStream2** ppStream)
    {
        if (!_mediaStream) return MF_E_NOT_INITIALIZED;
        *ppStream = _mediaStream;
        (*ppStream)->AddRef();
        return S_OK;
    }

    ULONG AddRef() override { return ++_refCount; }
    ULONG Release() override {
        ULONG count = --_refCount;
        if (count == 0) {
            delete this;
        }
        return count;
    }
    // Abstract methods
    // IUnknown methods
    HRESULT QueryInterface(const IID& riid, void** ppvObject) override {
        if (!ppvObject) return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_IMFMediaSource || riid == IID_IMFMediaSource2) {
            *ppvObject = static_cast<IMFMediaSource*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
    HRESULT IMFMediaEventGenerator::GetEvent(DWORD, IMFMediaEvent**) override { return E_NOTIMPL; }
	HRESULT IMFMediaEventGenerator::BeginGetEvent(IMFAsyncCallback*, IUnknown*) override { return E_NOTIMPL; }
	HRESULT IMFMediaEventGenerator::EndGetEvent(IMFAsyncResult*, IMFMediaEvent**) override { return E_NOTIMPL; }
	HRESULT IMFMediaEventGenerator::QueueEvent(MediaEventType, REFGUID, HRESULT, const PROPVARIANT*) override { return E_NOTIMPL; }
    HRESULT IMFMediaSource::GetCharacteristics(DWORD*) override { return E_NOTIMPL; }
	HRESULT IMFMediaSource::CreatePresentationDescriptor(IMFPresentationDescriptor**) override { return E_NOTIMPL; }
	HRESULT IMFMediaSource::Start(IMFPresentationDescriptor*, const GUID*, const PROPVARIANT*) override { return E_NOTIMPL; }
	HRESULT IMFMediaSource::Stop() override { return E_NOTIMPL; }
	HRESULT IMFMediaSource::Pause() override { return E_NOTIMPL; }
	HRESULT IMFMediaSource::Shutdown() override { return E_NOTIMPL; }
	HRESULT IMFMediaSourceEx::GetSourceAttributes(IMFAttributes**) override { return E_NOTIMPL; }
	HRESULT IMFMediaSourceEx::GetStreamAttributes(DWORD, IMFAttributes**) override { return E_NOTIMPL; }
	HRESULT IMFMediaSourceEx::SetD3DManager(IUnknown*) override { return E_NOTIMPL; }
	HRESULT IMFMediaSource2::SetMediaType(DWORD, IMFMediaType*) override { return E_NOTIMPL; }


};
//------------------------------------------------------------------------------------------------------------------------------------
// GSTMediaSourceActivate
//------------------------------------------------------------------------------------------------------------------------------------
#define RETURN_IF_FAILED(x) { HRESULT hr = (x); if (FAILED(hr)) return hr; }
//------------------------------------------------------------------------------------------------------------------------------------
class GSTMediaSourceActivate : public IMFActivate
{
public:
    GSTMediaSourceActivate() : _refCount(1), _mediaSource(nullptr) {};

    HRESULT Initialize()
    {
        _mediaSource = new GSTMediaSource();
        RETURN_IF_FAILED(SetUINT32(MF_VIRTUALCAMERA_PROVIDE_ASSOCIATED_CAMERA_SOURCES, 1));
        RETURN_IF_FAILED(SetGUID(MFT_TRANSFORM_CLSID_Attribute, CLSID_VCam));
        //RETURN_IF_FAILED(_mediaSource->Initialize(this));
        return S_OK;
    }
    // Implement QueryInterface, AddRef, and Release
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        if (riid == __uuidof(IMFActivate) || riid == __uuidof(IUnknown))
        {
            *ppv = static_cast<IMFActivate*>(this);
            AddRef();
            return S_OK;
        }
        *ppv = nullptr;
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&_refCount); }
    STDMETHODIMP_(ULONG) Release()
    {
        ULONG count = InterlockedDecrement(&_refCount);
        if (count == 0)
            delete this;
        return count;
    }

    STDMETHODIMP ActivateObject(REFIID riid, void** ppv)
    {
        if (riid == __uuidof(IMFMediaSource))
        {
            return _mediaSource.CopyTo(reinterpret_cast<IMFMediaSource**>(ppv));
        }
        return E_NOINTERFACE;
    }

    STDMETHODIMP ShutdownObject() { return S_OK; }
    STDMETHODIMP DetachObject() { _mediaSource = nullptr; return S_OK; }

private:
    ULONG _refCount; // Reference counter
    ComPtr<GSTMediaSource> _mediaSource;
};
