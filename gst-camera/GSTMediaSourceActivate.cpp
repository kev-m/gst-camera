#include "pch.h"

#include "GSTMediaSourceActivate.h"

HRESULT GSTMediaSourceActivate::Initialize()
{
    printf("GSTMediaSourceActivate::Initialize()\n");
    _mediaSource = new GSTMediaSource();
    RETURN_IF_FAILED(SetUINT32(MF_VIRTUALCAMERA_PROVIDE_ASSOCIATED_CAMERA_SOURCES, 1));
    RETURN_IF_FAILED(SetGUID(MFT_TRANSFORM_CLSID_Attribute, CLSID_GSTVirtualCamera));
    RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    RETURN_IF_FAILED(m_spActivateAttributes->SetUINT32(MF_VIRTUALCAMERA_PROVIDE_ASSOCIATED_CAMERA_SOURCES, 1));
    RETURN_IF_FAILED(_mediaSource->Initialize(this));
    printf("GSTMediaSourceActivate::Initialize() - complete!!\n");
    return S_OK;
}

// Implement QueryInterface, AddRef, and Release
STDMETHODIMP GSTMediaSourceActivate::QueryInterface(REFIID riid, void** ppv)
{
    printf("GSTMediaSourceActivate::QueryInterface(...)\n");
    if (riid == __uuidof(IMFActivate) || riid == __uuidof(IUnknown))
    {
        *ppv = static_cast<IMFActivate*>(this);
        AddRef();
        return S_OK;
    }
    *ppv = nullptr;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) GSTMediaSourceActivate::AddRef() { return InterlockedIncrement(&_refCount); }
STDMETHODIMP_(ULONG) GSTMediaSourceActivate::Release()
{
    ULONG count = InterlockedDecrement(&_refCount);
    if (count == 0)
        delete this;
    return count;
}

STDMETHODIMP GSTMediaSourceActivate::ActivateObject(REFIID riid, void** ppv)
{
    printf("GSTMediaSourceActivate::ActivateObject(...)\n");
    if (riid == __uuidof(IMFMediaSource))
    {
        return _mediaSource.CopyTo(reinterpret_cast<IMFMediaSource**>(ppv));
    }
    return E_NOINTERFACE;
}

IFACEMETHODIMP GSTMediaSourceActivate::ShutdownObject() { return S_OK; }
IFACEMETHODIMP GSTMediaSourceActivate::DetachObject() { _mediaSource = nullptr; return S_OK; }


// IMFAttributes (inherits by IMFActivate)
IFACEMETHODIMP GSTMediaSourceActivate::GetItem(_In_ REFGUID guidKey, _Inout_opt_  PROPVARIANT* pValue)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->GetItem(guidKey, pValue);
}

IFACEMETHODIMP GSTMediaSourceActivate::GetItemType(_In_ REFGUID guidKey, _Out_ MF_ATTRIBUTE_TYPE* pType)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->GetItemType(guidKey, pType);
}

IFACEMETHODIMP GSTMediaSourceActivate::CompareItem(_In_ REFGUID guidKey, _In_ REFPROPVARIANT Value, _Out_ BOOL* pbResult)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->CompareItem(guidKey, Value, pbResult);
}

IFACEMETHODIMP GSTMediaSourceActivate::Compare(_In_ IMFAttributes* pTheirs, _In_ MF_ATTRIBUTES_MATCH_TYPE MatchType, _Out_ BOOL* pbResult)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->Compare(pTheirs, MatchType, pbResult);
}

IFACEMETHODIMP GSTMediaSourceActivate::GetUINT32(_In_ REFGUID guidKey, _Out_ UINT32* punValue)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->GetUINT32(guidKey, punValue);
}

IFACEMETHODIMP GSTMediaSourceActivate::GetUINT64(_In_ REFGUID guidKey, _Out_ UINT64* punValue)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->GetUINT64(guidKey, punValue);
}

IFACEMETHODIMP GSTMediaSourceActivate::GetDouble(_In_ REFGUID guidKey, _Out_ double* pfValue)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->GetDouble(guidKey, pfValue);
}
IFACEMETHODIMP GSTMediaSourceActivate::GetGUID(_In_ REFGUID guidKey, _Out_ GUID* pguidValue)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->GetGUID(guidKey, pguidValue);
}

IFACEMETHODIMP GSTMediaSourceActivate::GetStringLength(_In_ REFGUID guidKey, _Out_ UINT32* pcchLength)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->GetStringLength(guidKey, pcchLength);
}

IFACEMETHODIMP GSTMediaSourceActivate::GetString(_In_ REFGUID guidKey, _Out_writes_(cchBufSize) LPWSTR pwszValue, _In_ UINT32 cchBufSize, _Inout_opt_ UINT32* pcchLength)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->GetString(guidKey, pwszValue, cchBufSize, pcchLength);
}

IFACEMETHODIMP GSTMediaSourceActivate::GetAllocatedString(_In_ REFGUID guidKey, _Out_writes_(*pcchLength + 1) LPWSTR* ppwszValue, _Inout_  UINT32* pcchLength)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->GetAllocatedString(guidKey, ppwszValue, pcchLength);
}

IFACEMETHODIMP GSTMediaSourceActivate::GetBlobSize(_In_ REFGUID guidKey, _Out_ UINT32* pcbBlobSize)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->GetBlobSize(guidKey, pcbBlobSize);
}

IFACEMETHODIMP GSTMediaSourceActivate::GetBlob(_In_ REFGUID  guidKey, _Out_writes_(cbBufSize) UINT8* pBuf, UINT32 cbBufSize, _Inout_  UINT32* pcbBlobSize)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->GetBlob(guidKey, pBuf, cbBufSize, pcbBlobSize);
}

IFACEMETHODIMP GSTMediaSourceActivate::GetAllocatedBlob(__RPC__in REFGUID guidKey, __RPC__deref_out_ecount_full_opt(*pcbSize) UINT8** ppBuf, __RPC__out UINT32* pcbSize)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->GetAllocatedBlob(guidKey, ppBuf, pcbSize);
}

IFACEMETHODIMP GSTMediaSourceActivate::GetUnknown(__RPC__in REFGUID guidKey, __RPC__in REFIID riid, __RPC__deref_out_opt LPVOID* ppv)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->GetUnknown(guidKey, riid, ppv);
}

IFACEMETHODIMP GSTMediaSourceActivate::SetItem(_In_ REFGUID guidKey, _In_ REFPROPVARIANT Value)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->SetItem(guidKey, Value);
}

IFACEMETHODIMP GSTMediaSourceActivate::DeleteItem(_In_ REFGUID guidKey)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->DeleteItem(guidKey);
}

IFACEMETHODIMP GSTMediaSourceActivate::DeleteAllItems()
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->DeleteAllItems();
}

IFACEMETHODIMP GSTMediaSourceActivate::SetUINT32(_In_ REFGUID guidKey, _In_ UINT32  unValue)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->SetUINT32(guidKey, unValue);
}

IFACEMETHODIMP GSTMediaSourceActivate::SetUINT64(_In_ REFGUID guidKey, _In_ UINT64  unValue)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->SetUINT64(guidKey, unValue);
}

IFACEMETHODIMP GSTMediaSourceActivate::SetDouble(_In_ REFGUID guidKey, _In_ double  fValue)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->SetDouble(guidKey, fValue);
}

IFACEMETHODIMP GSTMediaSourceActivate::SetGUID(_In_ REFGUID guidKey, _In_ REFGUID guidValue)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->SetGUID(guidKey, guidValue);
}

IFACEMETHODIMP GSTMediaSourceActivate::SetString(_In_ REFGUID guidKey, _In_ LPCWSTR wszValue)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->SetString(guidKey, wszValue);
}

IFACEMETHODIMP GSTMediaSourceActivate::SetBlob(_In_ REFGUID guidKey, _In_reads_(cbBufSize) const UINT8* pBuf, UINT32 cbBufSize)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->SetBlob(guidKey, pBuf, cbBufSize);
}

IFACEMETHODIMP GSTMediaSourceActivate::SetUnknown(_In_ REFGUID guidKey, _In_ IUnknown* pUnknown)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->SetUnknown(guidKey, pUnknown);
}

IFACEMETHODIMP GSTMediaSourceActivate::LockStore()
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->LockStore();
}

IFACEMETHODIMP GSTMediaSourceActivate::UnlockStore()
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->UnlockStore();
}

IFACEMETHODIMP GSTMediaSourceActivate::GetCount(_Out_ UINT32* pcItems)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->GetCount(pcItems);
}

IFACEMETHODIMP GSTMediaSourceActivate::GetItemByIndex(UINT32 unIndex, _Out_ GUID* pguidKey, _Inout_ PROPVARIANT* pValue)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->GetItemByIndex(unIndex, pguidKey, pValue);
}

IFACEMETHODIMP GSTMediaSourceActivate::CopyAllItems(_In_ IMFAttributes* pDest)
{
    if (!m_spActivateAttributes)
        RETURN_IF_FAILED(MFCreateAttributes(&m_spActivateAttributes, 1));
    return m_spActivateAttributes->CopyAllItems(pDest);
}

