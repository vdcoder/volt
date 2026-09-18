"""Live cookie-free HTTP route checks against a generated X+ server."""
import http.client
import json
from pathlib import Path
import socket
import subprocess
import sys
import time

exe = Path(sys.argv[1]).resolve()
with socket.socket() as probe:
    probe.bind(('127.0.0.1', 0))
    port = probe.getsockname()[1]
child = subprocess.Popen([str(exe), str(exe.parent.parent / 'web'), str(port)],
    stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
    creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
def request(method, path, body=None):
    connection = http.client.HTTPConnection('127.0.0.1', port, timeout=3)
    try:
        connection.request(method, path, body=body)
        response = connection.getresponse()
        return response.status, dict(response.getheaders()), response.read()
    finally:
        connection.close()
try:
    for _ in range(50):
        try:
            with socket.create_connection(('127.0.0.1', port), timeout=.1): break
        except OSError: time.sleep(.1)
    status, headers, body = request('GET', '/api/hello?name=world')
    assert status == 200 and json.loads(body)['message'] == 'Hello from Volt X+!'
    assert any(k.lower() == 'content-type' and 'application/json' in v for k,v in headers.items())
    status, headers, body = request('POST', '/api/echo', b'a\x00b\xff')
    assert status == 200 and body == b'a\x00b\xff'
    assert request('POST', '/api/echo', b'')[0] == 200
    status, headers, body = request('POST', '/api/hello')
    assert status == 405 and any(k.lower() == 'allow' and v == 'GET' for k,v in headers.items()), (status, headers, body)
    assert request('GET', '/api/missing')[0] == 404
    assert request('GET', '/')[0] == 200
    print('PASS: cookie-free HTTP routes, query matching, binary/empty bodies, 405/Allow, 404 and static fallback')
finally:
    child.terminate()
    child.wait(timeout=5)
