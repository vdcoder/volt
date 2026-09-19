"""Windows/WebView2 integration test against a generated, built starter Desktop.exe."""
import ctypes
from ctypes import wintypes
from pathlib import Path
import subprocess
import shutil
import sys
import tempfile
import time

kernel = ctypes.WinDLL('kernel32', use_last_error=True)
kernel.OpenProcess.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.DWORD]
kernel.OpenProcess.restype = wintypes.HANDLE
kernel.WaitForSingleObject.argtypes = [wintypes.HANDLE, wintypes.DWORD]
kernel.CloseHandle.argtypes = [wintypes.HANDLE]
kernel.TerminateProcess.argtypes = [wintypes.HANDLE, wintypes.UINT]


def alive(pid):
    handle = kernel.OpenProcess(0x100000, False, pid)
    if not handle:
        return False
    try:
        return kernel.WaitForSingleObject(handle, 0) == 258
    finally:
        kernel.CloseHandle(handle)


def wait_for(predicate, seconds=15):
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        result = predicate()
        if result:
            return result
        time.sleep(.05)
    raise AssertionError('Timed out')


def read_report(path):
    try:
        lines = path.read_text(encoding='utf-8').splitlines()
        if len(lines) >= 3:
            return int(lines[0]), int(lines[1]), lines[2]
    except (OSError, ValueError):
        pass


def start(exe, report):
    info = subprocess.STARTUPINFO()
    info.dwFlags |= subprocess.STARTF_USESHOWWINDOW
    info.wShowWindow = 0
    return subprocess.Popen([str(exe), '--smoke-test', str(report)], startupinfo=info)


def main(exe):
    children = []
    with tempfile.TemporaryDirectory(prefix='volt-desktop-') as temp:
        temp = Path(temp)
        try:
            # Concurrent instances must own different ports and independent servers.
            runs = [(start(exe, temp / f'{i}.txt'), temp / f'{i}.txt') for i in range(2)]
            children.extend(p for p, _ in runs)
            ports = []
            for process, report in runs:
                code = process.wait(timeout=55)
                result = read_report(report)
                assert code == 0 and result and result[2] == 'passed', (code, report.read_text(encoding='utf-8'))
                pid, port, _ = result
                assert port > 0
                ports.append(port)
                wait_for(lambda: not alive(pid))
            assert ports[0] != ports[1], ports
            print('PASS: two WebView2 instances, WASM UI, HTTP, data sync, normal cleanup')

            # Kill the host while its child is live: the job must reap that child.
            report = temp / 'host-crash.txt'
            host = start(exe, report)
            children.append(host)
            pid, _, _ = wait_for(lambda: read_report(report))
            assert alive(pid)
            host.kill()
            host.wait(timeout=5)
            wait_for(lambda: not alive(pid))
            print('PASS: terminating Desktop cleans up its hidden server')

            # Kill only the owned child: the host must report failure and exit.
            report = temp / 'server-crash.txt'
            host = start(exe, report)
            children.append(host)
            pid, _, _ = wait_for(lambda: read_report(report))
            handle = kernel.OpenProcess(1, False, pid)
            assert handle
            try:
                assert kernel.TerminateProcess(handle, 1)
            finally:
                kernel.CloseHandle(handle)
            assert host.wait(timeout=10) == 1
            assert read_report(report)[2].startswith('failed: ')
            print('PASS: unexpected server exit closes Desktop with a failure')

            # Incomplete packages must fail promptly, with no abandoned server.
            isolated = temp / 'incomplete package'
            (isolated / 'desktop').mkdir(parents=True)
            copied = isolated / 'desktop/Desktop.exe'
            shutil.copy2(exe, copied)
            report = temp / 'missing-server.txt'
            host = start(copied, report)
            children.append(host)
            assert host.wait(timeout=10) == 1
            assert read_report(report)[2].startswith('failed: ')
            (isolated / 'server').mkdir()
            shutil.copy2(exe.parent.parent / 'server/Server.exe', isolated / 'server/Server.exe')
            report = temp / 'missing-client.txt'
            host = start(copied, report)
            children.append(host)
            assert host.wait(timeout=10) == 1
            pid, _, status = read_report(report)
            assert status.startswith('failed: ')
            wait_for(lambda: not alive(pid))
            print('PASS: missing Server.exe or client output fails cleanly')
        finally:
            for child in children:
                if child.poll() is None:
                    child.kill()
                child.wait(timeout=5)


if __name__ == '__main__':
    main(Path(sys.argv[1]).resolve())
