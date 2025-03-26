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
    // [HKEY_LOCAL_MACHINE\SOFTWARE\Classes\CLSID\{CBA38424-2226-419C-9705-60D59BE792D3}\InprocServer32]
    //  @ = "C:\\Dev\\WindowsWDK\\gst-camera\\x64\\Debug\\mf_camera.dll"
    //    "ThreadingModel" = "Both"

    printf("Call to DllRegisterServer(....).\n");

    // Register the virtual camera with Windows
    auto clsid = GUID_ToStringW(CLSID_GSTVirtualCamera);
    std::wstring path = L"Software\\Classes\\CLSID\\" + clsid + L"\\InprocServer32";

    // Get the module path
    wchar_t szModulePath[MAX_PATH];
    if (GetModuleFileName(_hModule, szModulePath, MAX_PATH) == 0)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

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

    // Set the default value to the module path
    lResult = RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE*)szModulePath, (DWORD)((wcslen(szModulePath) + 1) * sizeof(wchar_t)));
    if (lResult != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(lResult);
    }

    // Set the ThreadingModel value to "Both"
    lResult = RegSetValueEx(hKey, L"ThreadingModel", 0, REG_SZ, (BYTE*)L"Both", (DWORD)(wcslen(L"Both") + 1) * sizeof(wchar_t));
    if (lResult != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        return HRESULT_FROM_WIN32(lResult);
    }

    RegCloseKey(hKey);
    return S_OK;
}

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