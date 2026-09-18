"""Real HTTP/WebSocket tests; standard-library only. Pass a built Server.exe."""
import base64
import os
from pathlib import Path
import socket
import struct
import subprocess
import sys
import time

class WS:
    def __init__(self, port, cookie='a' * 32, origin=True):
        self.sock = socket.create_connection(('127.0.0.1', port), timeout=3)
        self.sock.settimeout(9)
        self.buffer = b''
        key = base64.b64encode(os.urandom(16)).decode()
        headers = f'GET /session HTTP/1.1\r\nHost: 127.0.0.1:{port}\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: {key}\r\nSec-WebSocket-Version: 13\r\n'
        if cookie is not None: headers += f'Cookie: other=value; volt_session={cookie}\r\n'
        headers += f'Origin: http://127.0.0.1:{port}\r\n' if origin else 'Origin: http://other.invalid\r\n'
        self.sock.sendall((headers + '\r\n').encode())
        while b'\r\n\r\n' not in self.buffer:
            chunk = self.sock.recv(4096)
            if not chunk: raise EOFError('Rejected handshake')
            self.buffer += chunk
        head, self.buffer = self.buffer.split(b'\r\n\r\n', 1)
        assert b'101' in head.split(b'\r\n')[0], head
    def read(self, n):
        while len(self.buffer) < n:
            data = self.sock.recv(4096)
            if not data: raise EOFError('Closed')
            self.buffer += data
        data, self.buffer = self.buffer[:n], self.buffer[n:]
        return data
    def receive(self):
        first, second = self.read(2)
        length = second & 127
        if length == 126: length = struct.unpack('!H', self.read(2))[0]
        if length == 127: length = struct.unpack('!Q', self.read(8))[0]
        assert not second & 128
        data = self.read(length)
        if first & 15 == 8: raise EOFError('Close frame')
        assert first & 15 == 2
        return data
    def send(self, message):
        data = message if isinstance(message, bytes) else message.encode(); mask = os.urandom(4)
        assert len(data) < 126
        self.sock.sendall(bytes([0x82 if isinstance(message, bytes) else 0x81, 0x80 | len(data)]) + mask + bytes(c ^ mask[i % 4] for i, c in enumerate(data)))
    def close(self): self.sock.close()

def closed(ws):
    try: ws.receive()
    except (EOFError, ConnectionError): return
    raise AssertionError('Expected disconnect')

def main():
    exe = Path(sys.argv[1]).resolve()
    public = Path(__file__).resolve().parents[1] / 'app-template-x-plus/client/public'
    with socket.socket() as probe:
        probe.bind(('127.0.0.1', 0)); port = probe.getsockname()[1]
    child = subprocess.Popen([str(exe), str(public), str(port)], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
                             creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
    peers = []
    def connect(cookie='a' * 32):
        ws = WS(port, cookie); peers.append(ws); assert ws.receive() == b'\x00\x00\x01'; return ws
    try:
        for _ in range(50):
            try:
                with socket.create_connection(('127.0.0.1', port), timeout=.1): break
            except OSError: time.sleep(.1)
        first = connect()
        # Client Front allocation -> server observer -> Back allocation, no generations.
        first.send(struct.pack('<HBBIIi', 2, 1, 1, 0, 0, 7))
        assert first.receive() == struct.pack('<HBBIIq', 3, 1, 2, 0, 0, 14)
        first.send(struct.pack('<HBBIi', 2, 2, 1, 0, 8))
        assert first.receive() == struct.pack('<HBBIq', 3, 2, 2, 0, 16)
        first.send(struct.pack('<HBQ', 0, 2, 77)); assert first.receive() == struct.pack('<HBQ', 0, 3, 77)
        second = connect()
        # Replacement is a fresh author: slot zero is available again on both stores.
        second.send(struct.pack('<HBBIIi', 2, 1, 1, 0, 0, 3))
        assert second.receive() == struct.pack('<HBBIIq', 3, 1, 2, 0, 0, 6)
        assert first.receive() == b'\x00\x00\x04'
        first.send(struct.pack('<HBQ', 0, 2, 88)) # inactive sockets must not be accepted after replacement
        independent = connect('b' * 32)
        independent.send(struct.pack('<HBQ', 0, 2, 1)); assert independent.receive() == struct.pack('<HBQ', 0, 3, 1)
        second.send(struct.pack('<HBQ', 0, 2, 2)); assert second.receive() == struct.pack('<HBQ', 0, 3, 2)
        closed(first)
        independent.close()
        # Survive two five-second server ping cycles.
        for _ in range(2):
            ping = second.receive(); assert len(ping) == 11 and ping[:3] == b'\x00\x00\x02'
            second.send(b'\x00\x00\x03' + ping[3:])
        for payload in (b'', b'\x00\xff\x80binary'):
            frame = b'\x01\x00' + payload
            second.send(frame); assert second.receive() == frame
        second.close()
        silent = connect('c' * 32)
        ping = silent.receive(); assert len(ping) == 11 and ping[:3] == b'\x00\x00\x02'
        sent = time.monotonic(); closed(silent)
        assert 1.5 <= time.monotonic() - sent < 3.5
        malformed = connect('d' * 32)
        malformed.send('not-a-message'); closed(malformed)
        for frame in (b'\x01', b'\xff\xff', b'\x00\x00\x02', b'\x00\x00\x03' + bytes(8)):
            bad = connect('f' * 32); bad.send(frame); closed(bad)
        for cookie, origin in [(None, True), ('invalid', True), ('e' * 32, False)]:
            try:
                invalid = WS(port, cookie, origin); peers.append(invalid); closed(invalid)
            except (EOFError, ConnectionError): pass
        print('PASS: cookie validation, origin rejection, independent sessions, takeover notice, two ping cycles, timeout and protocol-error close')
    finally:
        for peer in peers: peer.close()
        child.terminate(); child.wait(timeout=5)
if __name__ == '__main__': main()
