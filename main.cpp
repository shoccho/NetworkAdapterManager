#include <Windows.h>
#include <string>
#include <comdef.h>
#include <wbemidl.h>
#include <memory>

#define ID_TRAY_EXIT 1001
#define ID_TRAY_OPTION1 1002
#define ID_TRAY_OPTION2 1003
#define ID_TRAY_ICON 1005

#pragma comment(lib, "wbemuuid.lib")

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow) {
    HWND hwnd;
    NOTIFYICONDATAW nid; // Use NOTIFYICONDATAW for wide character strings
    
    WNDCLASSW wc = {0}; // Use WNDCLASSW for wide character strings
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"SystemTrayApp";

    RegisterClassW(&wc); // Use RegisterClassW for wide character strings

    hwnd = CreateWindowExW(0, L"SystemTrayApp", L"System Tray App", 0, 0, 0, 0, 0, nullptr, nullptr, hInstance, nullptr); // Use CreateWindowExW for wide character strings and nullptr for null pointer constants

    ZeroMemory(&nid, sizeof(NOTIFYICONDATAW));
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hwnd;
     nid.uID = ID_TRAY_ICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_USER + 1;
    nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION); // Use LoadIconW for wide character strings
    std::wstring tip = L"My App"; // Use std::wstring for wide character strings
    wcsncpy_s(nid.szTip, tip.c_str(), _TRUNCATE); // Use wcsncpy_s for wide character strings

    Shell_NotifyIconW(NIM_ADD, &nid); // Use Shell_NotifyIconW for wide character strings

    MSG msg;
    while(GetMessageW(&msg, nullptr, 0, 0)) { // Use GetMessageW for wide character strings and nullptr for null pointer constants
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
void displayNetworkConnections(HMENU hMenu) {
    // Initialize COM
    HRESULT hres = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        return;
    }

    IWbemLocator* pLoc = nullptr;
    IWbemServices* pSvc = nullptr;
    IEnumWbemClassObject* pEnumerator = nullptr;

    // Connect to the local WMI namespace
    hres = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<void**>(&pLoc));
    if (FAILED(hres)) {
        CoUninitialize();
        return;
    }

    // Connect to the root\cimv2 namespace
    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &pSvc);
    if (FAILED(hres)) {
        pLoc->Release();
        CoUninitialize();
        return;
    }

    // Set security levels on the proxy
    hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    // Execute a query to retrieve network adapters
    hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_NetworkAdapter WHERE NetConnectionStatus=2"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator);
    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return;
    }

    // Enumerate network adapters and populate the context menu
    ULONG uReturn;
    IWbemClassObject* pclsObj = nullptr;
    while (pEnumerator) {
        hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (uReturn == 0) {
            break;
        }

        VARIANT vtProp;
        BSTR bstrName = nullptr;

        // Get the name of the network adapter
        hres = pclsObj->Get(L"NetConnectionID", 0, &vtProp, nullptr, nullptr);
        if (SUCCEEDED(hres)) {
            bstrName = vtProp.bstrVal;
        }

        // Add the network adapter to the context menu
        if (bstrName) {
            std::wstring name(bstrName);
            AppendMenuW(hMenu, MF_STRING, 0, name.c_str());
            SysFreeString(bstrName);
        }

        VariantClear(&vtProp);
        pclsObj->Release();
    }

    // Release resources
    if (pEnumerator) pEnumerator->Release();
    if (pSvc) pSvc->Release();
    if (pLoc) pLoc->Release();
    CoUninitialize();
}

// void displayNetworkConnections(HMENU hMenu) {
//     // Initialize COM
//     CoInitializeEx(nullptr, COINIT_MULTITHREADED);

//     HRESULT hres;
//     std::unique_ptr<IWbemLocator, decltype(&CoTaskMemFree)> pLoc(nullptr, CoTaskMemFree);
//     std::unique_ptr<IWbemServices, decltype(&CoTaskMemFree)> pSvc(nullptr, CoTaskMemFree);
//     std::unique_ptr<IEnumWbemClassObject, decltype(&CoTaskMemFree)> pEnumerator(nullptr, CoTaskMemFree);

//     // Connect to the local WMI namespace
//     hres = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<void**>(&pLoc));
//     if (FAILED(hres)) {
//         CoUninitialize();
//         return;
//     }

//     // Connect to the root\cimv2 namespace
//     hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &pSvc.get());
//     // hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), nullptr, nullptr, 0, 0, 0, 0, &pSvc);
//     if (FAILED(hres)) {
//         CoUninitialize();
//         return;
//     }

//     // Set security levels on the proxy
//     hres = CoSetProxyBlanket(pSvc.get(), RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
//     if (FAILED(hres)) {
//         CoUninitialize();
//         return;
//     }

//     // Execute a query to retrieve network adapters
//     hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_NetworkAdapter WHERE NetConnectionStatus=2"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator);
//     if (FAILED(hres)) {
//         CoUninitialize();
//         return;
//     }

//     // Enumerate network adapters and populate the context menu
//     ULONG uReturn;
//     IWbemClassObject* pclsObj = nullptr;
//     while (pEnumerator) {
//         hres = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
//         if (uReturn == 0) {
//             break;
//         }

//         VARIANT vtProp;
//         BSTR bstrName = nullptr;

//         // Get the name of the network adapter
//         hres = pclsObj->Get(L"NetConnectionID", 0, &vtProp, nullptr, nullptr);
//         if (SUCCEEDED(hres)) {
//             bstrName = vtProp.bstrVal;
//         }

//         // Add the network adapter to the context menu
//         if (bstrName) {
//             std::wstring name(bstrName);
//             AppendMenuW(hMenu, MF_STRING, 0, name.c_str());
//             SysFreeString(bstrName);
//         }

//         VariantClear(&vtProp);
//         pclsObj->Release();
//     }

//     // Release resources
//     pEnumerator->Release();
//     CoUninitialize();
// }
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_USER + 1:
            switch(LOWORD(lParam)) {
                case WM_RBUTTONDOWN: {
                    HMENU hMenu = CreatePopupMenu();

                    // Display network connections in the context menu
                    displayNetworkConnections(hMenu);

                    AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
                    AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit");

                    POINT pt;
                    GetCursorPos(&pt);
                    SetForegroundWindow(hwnd);
                    TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, nullptr);
                }
                break;
            }
            break;
        
        case WM_COMMAND:
            switch(LOWORD(wParam)) {
                case ID_TRAY_OPTION1:
                    MessageBoxW(hwnd, L"Option 1 selected", L"Info", MB_OK | MB_ICONINFORMATION);
                    break;

                case ID_TRAY_OPTION2:
                    MessageBoxW(hwnd, L"Option 2 selected", L"Info", MB_OK | MB_ICONINFORMATION);
                    break;

                case ID_TRAY_EXIT:
                    DestroyWindow(hwnd);
                    break;
            }
            break;

        case WM_DESTROY:
            NOTIFYICONDATAW nid;
            memset(&nid, 0, sizeof(NOTIFYICONDATAW)); // Clear the structure
            nid.cbSize = sizeof(NOTIFYICONDATAW);
            nid.hWnd = hwnd;
            nid.uID = ID_TRAY_ICON;

Shell_NotifyIconW(NIM_DELETE, &nid);
            // Shell_NotifyIconW(NIM_DELETE, &(NOTIFYICONDATAW){sizeof(NOTIFYICONDATAW), hwnd, IDI_APPLICATION});
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

