#pragma once

#include "pch.h"

#include <atomic>

#include "GSTMediaStream.h"
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
    GSTMediaSource() : _refCount(1), _mediaStream(nullptr), _isStarted(false)
    {
        g_print("GSTMediaSource::GSTMediaSource()\n");
    }

    HRESULT Start(IMFAsyncCallback* pCallback)
    {
        if (!_mediaStream)
        {
            g_print("GSTMediaSource::Start()\n");
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
