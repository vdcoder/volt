#pragma once
#include <windows.h>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

namespace desktop {
inline void check(bool success, const char* operation)
{
    if (!success) throw std::runtime_error(std::string(operation) + " (Windows error "
        + std::to_string(GetLastError()) + ")");
}

class Handle {
public:
    HANDLE value = nullptr;
    Handle() = default;
    ~Handle() { reset(); }
    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;
    void reset(HANDLE next = nullptr) {
        if (value && value != INVALID_HANDLE_VALUE) CloseHandle(value);
        value = next;
    }
};

// Owns only the server we start. A job also stops it if Desktop crashes.
class DesktopServer {
public:
    ~DesktopServer() { stop(); }

    void start(const std::filesystem::path& executable, const std::filesystem::path& log)
    {
        SECURITY_ATTRIBUTES security{sizeof(security), nullptr, TRUE};
        Handle readyWrite, output, input;
        check(CreatePipe(&readyRead_.value, &readyWrite.value, &security, 0), "Create readiness pipe");
        check(SetHandleInformation(readyRead_.value, HANDLE_FLAG_INHERIT, 0), "Protect readiness reader");
        stopEvent_.value = CreateEventW(&security, TRUE, FALSE, nullptr);
        check(stopEvent_.value != nullptr, "Create stop event");
        output.value = CreateFileW(log.c_str(), GENERIC_WRITE, FILE_SHARE_READ, &security,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        check(output.value != INVALID_HANDLE_VALUE, "Create server log");
        input.value = CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
            &security, OPEN_EXISTING, 0, nullptr);
        check(input.value != INVALID_HANDLE_VALUE, "Open server input");

        job_.value = CreateJobObjectW(nullptr, nullptr);
        check(job_.value != nullptr, "Create server job");
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
        limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        check(SetInformationJobObject(job_.value, JobObjectExtendedLimitInformation, &limits, sizeof(limits)), "Configure server job");

        SIZE_T size = 0;
        InitializeProcThreadAttributeList(nullptr, 1, 0, &size);
        std::vector<unsigned char> buffer(size);
        auto attributes = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(buffer.data());
        check(InitializeProcThreadAttributeList(attributes, 1, 0, &size), "Create process attributes");
        struct Cleanup { LPPROC_THREAD_ATTRIBUTE_LIST p; ~Cleanup() { DeleteProcThreadAttributeList(p); } } cleanup{attributes};
        HANDLE inherited[]{readyWrite.value, stopEvent_.value, output.value, input.value};
        check(UpdateProcThreadAttribute(attributes, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
            inherited, sizeof(inherited), nullptr, nullptr), "Set inherited handles");
        STARTUPINFOEXW startup{};
        startup.StartupInfo.cb = sizeof(startup);
        startup.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
        startup.StartupInfo.hStdInput = input.value;
        startup.StartupInfo.hStdOutput = output.value;
        startup.StartupInfo.hStdError = output.value;
        startup.lpAttributeList = attributes;
        auto command = L"\"" + executable.wstring() + L"\" --desktop "
            + std::to_wstring(reinterpret_cast<uintptr_t>(readyWrite.value)) + L" "
            + std::to_wstring(reinterpret_cast<uintptr_t>(stopEvent_.value));
        PROCESS_INFORMATION process{};
        check(CreateProcessW(executable.c_str(), command.data(), nullptr, nullptr, TRUE,
            CREATE_NO_WINDOW | CREATE_SUSPENDED | EXTENDED_STARTUPINFO_PRESENT, nullptr,
            executable.parent_path().c_str(), &startup.StartupInfo, &process), "Start Server.exe");
        process_.value = process.hProcess;
        Handle thread;
        thread.value = process.hThread;
        if (!AssignProcessToJobObject(job_.value, process_.value)) {
            const auto error = GetLastError();
            TerminateProcess(process_.value, 1); // Still suspended; never leave an unowned child.
            SetLastError(error);
            check(false, "Assign server job");
        }
        check(ResumeThread(thread.value) != DWORD(-1), "Resume server");
    }

    bool running() const { return process_.value && WaitForSingleObject(process_.value, 0) == WAIT_TIMEOUT; }
    DWORD processId() const { return GetProcessId(process_.value); }

    uint16_t readyPort()
    {
        DWORD available = 0;
        check(PeekNamedPipe(readyRead_.value, nullptr, 0, nullptr, &available, nullptr), "Read server readiness");
        if (available < sizeof(uint16_t)) return 0;
        uint16_t port = 0;
        DWORD read = 0;
        check(ReadFile(readyRead_.value, &port, sizeof(port), &read, nullptr)
            && read == sizeof(port) && port, "Read server port");
        readyRead_.reset();
        return port;
    }

    void stop() noexcept
    {
        if (stopEvent_.value) SetEvent(stopEvent_.value);
        if (process_.value) WaitForSingleObject(process_.value, 2000);
        job_.reset();
        process_.reset();
        stopEvent_.reset();
        readyRead_.reset();
    }
private:
    Handle job_, process_, stopEvent_, readyRead_;
};
}
