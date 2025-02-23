#include "pch.h"

#include <wrl.h>
#include <string>

#include "GSTMediaSourceActivate.h"


HMODULE _hModule;
BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Save the module handle
        g_print("Call to DllMain(DLL_PROCESS_ATTACH)\n");
        _hModule = hModule;
		break;
	case DLL_PROCESS_DETACH:
        g_print("Call to DllMain(DLL_PROCESS_DETACH)\n");
        break;
	}
	return TRUE;
}


__control_entrypoint(DllExport)
STDAPI DllCanUnloadNow()
{
    printf("Call to DllCanUnloadNow().\n");
    // Return OK if the camera is not in use.

	return S_OK;
}

// DLL entry point
__control_entrypoint(DllExport)
HRESULT __stdcall DllGetClassObject(GUID const& clsid, GUID const& iid, void** result)
{
    printf("Call to DllGetClassObject(....).\n");
    if (clsid == CLSID_GSTVirtualCamera)
    {
        Microsoft::WRL::ComPtr<GSTMediaSourceActivate> spActivate = new GSTMediaSourceActivate();
        if (!spActivate)
            return E_OUTOFMEMORY;

        HRESULT hr = spActivate->Initialize();
        if (FAILED(hr))
            return hr;

        return spActivate.CopyTo(iid, result);
    }
    return CLASS_E_CLASSNOTAVAILABLE;
}


// DLL registration
STDAPI DllRegisterServer()
{
    printf("Call to DllRegisterServer(....).\n");
    // Register the virtual camera with Windows
    auto clsid = GUID_ToStringW(CLSID_GSTVirtualCamera);
    std::wstring path = L"Software\\Classes\\CLSID\\" + clsid + L"\\InprocServer32";
    HKEY hKey;
    LONG lResult;

    // Create the registry key
    lResult = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        path.c_str(),
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &hKey,
        NULL
    );

    if (lResult != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(lResult);
    }

    // Set registry values
    DWORD vcamType = (DWORD)MFVirtualCameraType_SoftwareCameraSource;
    lResult = RegSetValueEx(hKey, L"Type", 0, REG_DWORD, (BYTE*)&vcamType, sizeof(DWORD));
    if (lResult != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(lResult);
    }

    wchar_t szModulePath[MAX_PATH];
    if (GetModuleFileName(NULL, szModulePath, MAX_PATH) == 0)
    {
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Store the DLL path
    lResult = RegSetValueEx(hKey, L"DllPath", 0, REG_SZ, (BYTE*)szModulePath, (DWORD)((wcslen(szModulePath) + 1) * sizeof(wchar_t)));
    RegCloseKey(hKey);

    return HRESULT_FROM_WIN32(lResult);
}

//STDAPI DllRegisterServer()
//{
//    printf("Call to DllRegisterServer(....).\n");
//    auto clsid = GUID_ToStringW(CLSID_GSTVirtualCamera);
//    std::wstring path = L"Software\\Classes\\CLSID\\" + clsid + L"\\InprocServer32";
//
//    wchar_t szModulePath[MAX_PATH];
//    if (GetModuleFileName(NULL, szModulePath, MAX_PATH) == 0)
//    {
//        RegCloseKey(hKey);
//        return HRESULT_FROM_WIN32(GetLastError());
//    }
//
//
//    // note: a vcam *must* be registered in HKEY_LOCAL_MACHINE
//    // for the frame server to be able to talk with it.
//    registry_key key;
//    RETURN_IF_WIN32_ERROR(RegWriteKey(HKEY_LOCAL_MACHINE, path.c_str(), key.put()));
//    RETURN_IF_WIN32_ERROR(RegWriteValue(key.get(), nullptr, exePath));
//    RETURN_IF_WIN32_ERROR(RegWriteValue(key.get(), L"ThreadingModel", L"Both"));
//    return S_OK;
//}

STDAPI DllUnregisterServer()
{
    printf("Call to DllUnregisterServer(....).\n");
    // Unregister the virtual camera
    auto clsid = GUID_ToStringW(CLSID_GSTVirtualCamera);
    std::wstring path = L"Software\\Classes\\CLSID\\" + clsid;
    LONG lResult = RegDeleteTree(HKEY_LOCAL_MACHINE, path.c_str());
    if (lResult != ERROR_SUCCESS && lResult != ERROR_FILE_NOT_FOUND)
    {
        return HRESULT_FROM_WIN32(lResult);
    }
    return S_OK;
}

const std::wstring GUID_ToStringW(const GUID& guid)
{
    wchar_t name[64];
    std::ignore = StringFromGUID2(guid, name, _countof(name));
    return name;
}