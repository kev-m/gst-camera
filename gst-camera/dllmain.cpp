#include "pch.h"

#include <wrl.h>



// Define the class GUID for the virtual camera
// {79c2dd91-230b-419f-b356-895dc329716d}
static const GUID CLSID_GSTVirtualCamera =
{ 0x79c2dd91, 0x230b, 0x419f, { 0xb3, 0x56, 0x89, 0x5d, 0xc3, 0x29, 0x71, 0x6d } };
#define VCAM_REG_PATH L"SOFTWARE\\Microsoft\\Windows Media Foundation\\VirtualCamera\\{79c2dd91-230b-419f-b356-895dc329716d}"

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

// Factory function for COM registration
HRESULT CreateVirtualCamera(IMFVirtualCamera** ppCamera)
{
    g_print("Call to CreateVirtualCamera(...)\n");
    if (!ppCamera) return E_POINTER;
    //ComPtr<GSTVirtualCamera> spCamera = Make<GSTVirtualCamera>();
    //return spCamera.CopyTo(ppCamera);
    return S_OK;
}

__control_entrypoint(DllExport)
STDAPI DllCanUnloadNow()
{
    g_print("Call to DllCanUnloadNow().\n");
    // Return OK if the camera is not in use.

	return S_OK;
}

// DLL entry point
STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID FAR* ppv)
{
    g_print("Call to DllGetClassObject(....).\n");
    if (rclsid == CLSID_GSTVirtualCamera)
    {
        return CreateVirtualCamera(reinterpret_cast<IMFVirtualCamera**>(ppv));
    }
    return CLASS_E_CLASSNOTAVAILABLE;
}

// DLL registration
STDAPI DllRegisterServer()
{
    g_print("Call to DllRegisterServer(....).\n");
    // Register the virtual camera with Windows
    HKEY hKey;
    LONG lResult;

    // Create the registry key
    lResult = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        VCAM_REG_PATH,
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

STDAPI DllUnregisterServer()
{
    g_print("Call to DllUnregisterServer(....).\n");
    // Unregister the virtual camera
    LONG lResult = RegDeleteTree(HKEY_LOCAL_MACHINE, VCAM_REG_PATH);
    if (lResult != ERROR_SUCCESS && lResult != ERROR_FILE_NOT_FOUND)
    {
        return HRESULT_FROM_WIN32(lResult);
    }
    return S_OK;
}
