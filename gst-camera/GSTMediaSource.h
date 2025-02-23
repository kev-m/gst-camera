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
    ComPtr <GSTMediaStream> _mediaStream;
    bool _isStarted;

public:
    GSTMediaSource() : _refCount(1), _mediaStream(nullptr), _isStarted(false)
    {
        g_print("GSTMediaSource::GSTMediaSource()\n");
    }

    HRESULT Initialize(IMFAttributes* attributes)
    {
		g_print("GSTMediaSource::Initialize()\n");
        //_mediaStream->Initialize(this);
		return S_OK;
    }

    HRESULT Start(IMFAsyncCallback* pCallback)
    {
        if (!_mediaStream)
        {
            g_print("GSTMediaSource::Start()\n");
            _mediaStream = new GSTMediaStream(); // Create the media stream
            _mediaStream->Initialize(this);
        }
        _isStarted = true;

        // Notify that the stream is ready
        if (pCallback) pCallback->Invoke(nullptr);

        return S_OK;
    }

    HRESULT GetStream(ComPtr <GSTMediaStream>* ppStream)
    {
        g_print("GSTMediaSource::GetStream()\n");
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
        g_print("GSTMediaSource::QueryInterface(...)\n");
        if (!ppvObject) return E_POINTER;
        if (riid == IID_IUnknown || riid == IID_IMFMediaSource || riid == IID_IMFMediaSource2) {
            *ppvObject = static_cast<IMFMediaSource*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
    HRESULT IMFMediaEventGenerator::GetEvent(DWORD, IMFMediaEvent**) override { return S_OK /* E_NOTIMPL */; }
    HRESULT IMFMediaEventGenerator::BeginGetEvent(IMFAsyncCallback*, IUnknown*) override { return S_OK /* E_NOTIMPL */; }
    HRESULT IMFMediaEventGenerator::EndGetEvent(IMFAsyncResult*, IMFMediaEvent**) override { return S_OK /* E_NOTIMPL */; }
    HRESULT IMFMediaEventGenerator::QueueEvent(MediaEventType, REFGUID, HRESULT, const PROPVARIANT*) override { return S_OK /* E_NOTIMPL */; }
    HRESULT IMFMediaSource::GetCharacteristics(DWORD*) override { return S_OK /* E_NOTIMPL */; }
    HRESULT IMFMediaSource::CreatePresentationDescriptor(IMFPresentationDescriptor**) override { return S_OK /* E_NOTIMPL */; }
    HRESULT IMFMediaSource::Start(IMFPresentationDescriptor*, const GUID*, const PROPVARIANT*) override { return S_OK /* E_NOTIMPL */; }
    HRESULT IMFMediaSource::Stop() override { return S_OK /* E_NOTIMPL */; }
    HRESULT IMFMediaSource::Pause() override { return S_OK /* E_NOTIMPL */; }
    HRESULT IMFMediaSource::Shutdown() override { return S_OK /* E_NOTIMPL */; }
    HRESULT IMFMediaSourceEx::GetSourceAttributes(IMFAttributes**) override { return S_OK /* E_NOTIMPL */; }
    HRESULT IMFMediaSourceEx::GetStreamAttributes(DWORD, IMFAttributes**) override { return S_OK /* E_NOTIMPL */; }
    HRESULT IMFMediaSourceEx::SetD3DManager(IUnknown*) override { return S_OK /* E_NOTIMPL */; }
    HRESULT IMFMediaSource2::SetMediaType(DWORD, IMFMediaType*) override { return S_OK /* E_NOTIMPL */; }
};
