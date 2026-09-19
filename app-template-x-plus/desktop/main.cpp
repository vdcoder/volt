#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <wrl.h>
#include <WebView2.h>
#include <WebView2EnvironmentOptions.h>
#include "DesktopServer.hpp"
#include <fstream>

using Microsoft::WRL::Callback;
using Microsoft::WRL::ComPtr;
namespace fs = std::filesystem;

namespace {
void checked(HRESULT result, const char* operation)
{
    if (FAILED(result)) throw std::runtime_error(std::string(operation)
        + " (HRESULT " + std::to_string(static_cast<unsigned long>(result)) + ")");
}

class Desktop {
public:
    HWND window = nullptr;
    int exitCode = 0;
    fs::path smokeReport; // Optional integration test; ordinary launches have no test behavior.

    void start()
    {
        wchar_t module[32768];
        auto length = GetModuleFileNameW(nullptr, module, 32768);
        desktop::check(length && length < 32768, "Locate Desktop.exe");
        PWSTR local = nullptr;
        checked(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local), "Locate local app data");
        data_ = fs::path(local) / L"Volt" / L"VOLT_APP_NAME-VOLT_DESKTOP_PROJECT_GUID";
        CoTaskMemFree(local);
        fs::create_directories(data_);
        log_ = data_ / (L"server-" + std::to_wstring(GetCurrentProcessId()) + L".log");
        server_.start(fs::path(module).parent_path().parent_path() / L"server/Server.exe", log_);
        started_ = GetTickCount64();
        desktop::check(SetTimer(window, 1, 100, nullptr) != 0, "Start desktop timer");
        if (!smokeReport.empty()) report("starting");
    }

    void tick()
    {
        if (closing_) return;
        if (!server_.running()) throw std::runtime_error("The application server stopped unexpectedly.");
        if (!port_) {
            port_ = server_.readyPort();
            if (port_) {
                if (!smokeReport.empty()) report("listening");
                createWebView();
            } else if (GetTickCount64() - started_ > 15000)
                throw std::runtime_error("The application server did not become ready in 15 seconds.");
        }
        if (!smokeReport.empty()) {
            if (GetTickCount64() - started_ > 45000)
                throw std::runtime_error("Desktop smoke test timed out waiting for Volt and its session.");
            if (web_ && !probePending_) {
                probePending_ = true;
                checked(web_->ExecuteScript(
                    LR"JS((() => {
                        const text = document.body?.innerText || '';
                        if (!text.includes('Server connection: connected')) return text;
                        const buttons = [...document.querySelectorAll('button')];
                        if (!window.__desktopTest) {
                            window.__desktopTest = true;
                            buttons.find(b => b.innerText === 'Send increment')?.click();
                            buttons.find(b => b.innerText === 'Call hello API')?.click();
                        }
                        return (text.includes('Client: 1 / Server doubled: 2')
                            && text.includes('Hello from Volt X+!')) || text;
                    })())JS",
                    Callback<ICoreWebView2ExecuteScriptCompletedHandler>([this](HRESULT result, LPCWSTR json) -> HRESULT {
                        probePending_ = false;
                        if (json) lastProbe_ = json;
                        if (!closing_ && SUCCEEDED(result) && json && std::wstring(json) == L"true") {
                            report("passed");
                            DestroyWindow(window);
                        }
                        return S_OK;
                    }).Get()), "Probe client");
            }
        }
    }

    void resize()
    {
        if (controller_) {
            RECT bounds{};
            GetClientRect(window, &bounds);
            controller_->put_Bounds(bounds);
        }
    }

    void focus() { if (controller_) controller_->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC); }

    void close()
    {
        closing_ = true;
        KillTimer(window, 1);
        if (controller_) controller_->Close();
        web_.Reset();
        controller_.Reset();
        server_.stop();
    }

    void fail(const std::exception& error)
    {
        if (closing_) return;
        exitCode = 1;
        const std::string message = error.what();
        const auto text = std::wstring(message.begin(), message.end())
            + L"\n\nServer log: " + log_.wstring()
            + L"\n\nWebView2 requires the Microsoft Edge WebView2 Evergreen Runtime.";
        if (smokeReport.empty()) MessageBoxW(window, text.c_str(), L"VOLT_APP_NAME — Desktop", MB_OK | MB_ICONERROR);
        else report("failed: " + message);
        DestroyWindow(window);
    }

private:
    void report(const std::string& status)
    {
        std::ofstream file(smokeReport);
        file << server_.processId() << '\n' << port_ << '\n' << status << '\n';
        if (!lastProbe_.empty()) {
            const auto size = WideCharToMultiByte(CP_UTF8, 0, lastProbe_.data(), static_cast<int>(lastProbe_.size()), nullptr, 0, nullptr, nullptr);
            std::string utf8(size, '\0');
            WideCharToMultiByte(CP_UTF8, 0, lastProbe_.data(), static_cast<int>(lastProbe_.size()), utf8.data(), size, nullptr, nullptr);
            file << "Page: " << utf8 << '\n';
        }
    }

    void createWebView()
    {
        auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
        // A hidden integration-test window must still get browser render/timer work.
        // Normal Desktop launches retain the runtime's default power-saving behavior.
        if (!smokeReport.empty()) checked(options->put_AdditionalBrowserArguments(
            L"--disable-backgrounding-occluded-windows --disable-renderer-backgrounding --disable-background-timer-throttling"), "Configure test browser");
        const auto profile = data_ / (smokeReport.empty() ? L"profile" : L"test-profile");
        checked(CreateCoreWebView2EnvironmentWithOptions(nullptr, profile.c_str(), options.Get(),
            Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
                [this](HRESULT result, ICoreWebView2Environment* environment) -> HRESULT {
                    if (closing_) return S_OK;
                    try {
                        checked(result, "Create WebView2 environment");
                        if (!environment) throw std::runtime_error("WebView2 returned no environment.");
                        checked(environment->CreateCoreWebView2Controller(window,
                            Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                                [this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                                    if (closing_) return S_OK;
                                    try {
                                        checked(result, "Create WebView2 controller");
                                        if (!controller) throw std::runtime_error("WebView2 returned no controller.");
                                        controller_ = controller;
                                        checked(controller_->get_CoreWebView2(&web_), "Get WebView2");
                                        ComPtr<ICoreWebView2Settings> settings;
                                        checked(web_->get_Settings(&settings), "Get WebView2 settings");
#ifdef NDEBUG
                                        settings->put_AreDefaultContextMenusEnabled(FALSE);
                                        settings->put_AreDevToolsEnabled(FALSE);
#endif
                                        resize();
                                        checked(controller_->put_IsVisible(TRUE), "Show WebView2");
                                        const auto url = L"http://127.0.0.1:" + std::to_wstring(port_) + L"/";
                                        checked(web_->Navigate(url.c_str()), "Open client");
                                        SetWindowTextW(window, L"VOLT_APP_NAME");
                                        focus();
                                    } catch (const std::exception& error) { fail(error); }
                                    return S_OK;
                                }).Get()), "Request WebView2 controller");
                    } catch (const std::exception& error) { fail(error); }
                    return S_OK;
                }).Get()), "Request WebView2 environment");
    }

    desktop::DesktopServer server_;
    ComPtr<ICoreWebView2Controller> controller_;
    ComPtr<ICoreWebView2> web_;
    fs::path data_, log_;
    ULONGLONG started_ = 0;
    uint16_t port_ = 0;
    bool closing_ = false, probePending_ = false;
    std::wstring lastProbe_;
};

LRESULT CALLBACK windowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    auto app = reinterpret_cast<Desktop*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
        app = static_cast<Desktop*>(reinterpret_cast<CREATESTRUCTW*>(lparam)->lpCreateParams);
        app->window = window;
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    }
    if (app) {
        try {
            switch (message) {
            case WM_SIZE: app->resize(); return 0;
            case WM_SETFOCUS: app->focus(); return 0;
            case WM_TIMER: app->tick(); return 0;
            case WM_DPICHANGED: {
                auto bounds = reinterpret_cast<RECT*>(lparam);
                SetWindowPos(window, nullptr, bounds->left, bounds->top, bounds->right - bounds->left,
                    bounds->bottom - bounds->top, SWP_NOZORDER | SWP_NOACTIVATE);
                return 0;
            }
            case WM_DESTROY: app->close(); PostQuitMessage(app->exitCode); return 0;
            }
        } catch (const std::exception& error) { app->fail(error); return 0; }
    }
    return DefWindowProcW(window, message, wparam, lparam);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show)
{
    const auto com = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(com)) return 1;
    int result = 1;
    {
        Desktop app;
        int argc = 0;
        auto argv = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (argv && argc == 3 && std::wstring(argv[1]) == L"--smoke-test") app.smokeReport = argv[2];
        if (argv) LocalFree(argv);
        WNDCLASSEXW type{};
        type.cbSize = sizeof(type);
        type.lpfnWndProc = windowProc;
        type.hInstance = instance;
        type.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        type.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        type.lpszClassName = L"VoltDesktop";
        if (RegisterClassExW(&type) && CreateWindowExW(0, type.lpszClassName,
            L"VOLT_APP_NAME — Starting…", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
            1200, 800, nullptr, nullptr, instance, &app)) {
            try {
                app.start();
                ShowWindow(app.window, app.smokeReport.empty() ? show : SW_HIDE);
                MSG message{};
                BOOL status;
                while ((status = GetMessageW(&message, nullptr, 0, 0)) > 0) {
                    TranslateMessage(&message);
                    DispatchMessageW(&message);
                }
                result = status == -1 ? 1 : static_cast<int>(message.wParam);
            } catch (const std::exception& error) { app.fail(error); }
        }
        if (IsWindow(app.window)) DestroyWindow(app.window);
    }
    CoUninitialize();
    return result;
}
