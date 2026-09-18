
function frame(text) {
  if (text instanceof ArrayBuffer) return text;
  const match = /^(ping|pong):(\d+)$/.exec(text);
  const op = text === 'ready' ? 1 : text === 'replaced' ? 4 : match?.[1] === 'ping' ? 2 : match ? 3 : 0;
  if (!op) return text;
  const bytes = new Uint8Array(match ? 11 : 3); bytes[2] = op;
  if (match) new DataView(bytes.buffer).setBigUint64(3, BigInt(match[2]), true);
  return bytes.buffer;
}
// Actual MEMORY64 C++/Embind/val interop, with simulated browser sockets.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');
const path = require('node:path');
const sockets = [], listeners = new Set();
global.window = { addEventListener: (_, fn) => listeners.add(fn), removeEventListener: (_, fn) => listeners.delete(fn) };
let cookie = '';
global.document = { get cookie() { return cookie; }, set cookie(v) { cookie = v.split(';')[0]; } };
global.location = { protocol: 'http:', host: 'localhost:8000' };
global.WebSocket = class {
    constructor() { this.sent = []; sockets.push(this); }
    send(message) { this.sent.push(message); }
    close() { this.closed = true; }
    message(data) { this.onmessage?.({data: frame(data)}); }
};
vm.runInThisContext(fs.readFileSync(path.join(__dirname, '../app-template-x-plus/client/public/session.js'), 'utf8'));
(async () => {
    const factory = require(path.resolve(process.argv[2]));
    const a = await factory(), b = await factory();
    try {
        a.initialize(); b.initialize(); assert.equal(sockets.length, 0);
        a.start(); b.start(); assert.equal(sockets.length, 2);
        assert.equal(a.state(), 'connecting'); assert.equal(b.state(), 'connecting');
        sockets[0].message('ready');
        assert.equal(a.state(), 'connected'); assert.equal(b.state(), 'connecting');
        assert.equal(a.invalidations(), 4); assert.equal(b.invalidations(), 1);
        a.writeFront();
        assert.deepEqual(Array.from(sockets[0].sent.pop()), [2, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0]);
        const beforeData = a.invalidations();
        sockets[0].message(new Uint8Array([3, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 14, 0, 0, 0, 0, 0, 0, 0]).buffer);
        assert.equal(a.readBack(), 14);
        assert.equal(a.invalidations(), beforeData + 1);
        a.sendTest();
        assert.deepEqual(Array.from(sockets[0].sent.pop()), [0x34, 0x12, 0, 255, 128]);
        sockets[0].message(new Uint8Array([0x34, 0x12, 0, 255, 128]).buffer);
        assert.equal(a.received(), 1); assert.equal(b.received(), 0);
        sockets[0].message('ping:42'); assert.deepEqual(Array.from(sockets[0].sent[0]), Array.from(new Uint8Array(frame('pong:42'))));
        sockets[1].message('ready'); sockets[0].message('replaced');
        assert.equal(a.state(), 'replaced'); assert.equal(b.state(), 'connected');
        assert.equal(listeners.size, 4);
        const stale = sockets[1].onmessage;
        sockets[1].message(new Uint8Array([0xff, 0xff]).buffer);
        assert.equal(b.state(), 'disconnected'); // unknown talker invalidates stream
        b.release(); assert(sockets[1].closed); assert.equal(listeners.size, 2);
        const before = b.invalidations(); stale({data: 'replaced'}); assert.equal(b.invalidations(), before);
        a.stop(); assert.equal(a.state(), 'stopped');
        a.release(); assert.equal(listeners.size, 0);
        console.log('PASS: real MEMORY64 DI/JS callbacks, runtime invalidation, module isolation and teardown');
    } finally { a.release(); b.release(); }
})().catch(error => { console.error(error); process.exitCode = 1; });
