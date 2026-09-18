
function frame(text) {
  if (text instanceof ArrayBuffer) return text;
  const match = /^(ping|pong):(\d+)$/.exec(text);
  const op = text === 'ready' ? 1 : text === 'replaced' ? 4 : match?.[1] === 'ping' ? 2 : match ? 3 : 0;
  if (!op) return text;
  const bytes = new Uint8Array(match ? 11 : 3); bytes[2] = op;
  if (match) new DataView(bytes.buffer).setBigUint64(3, BigInt(match[2]), true);
  return bytes.buffer;
}
const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');
const script = fs.readFileSync(require('node:path').join(__dirname, '../app-template-x-plus/client/public/session.js'), 'utf8');
function browser(cookie = '') {
  let now = 0, id = 0;
  const tasks = new Map(), sockets = [], states = [], handlers = {};
  const document = { get cookie() { return cookie; }, set cookie(v) { cookie = v.split(';')[0]; } };
  class WebSocket {
    constructor(url) { this.url = url; this.sent = []; sockets.push(this); }
    send(text) { this.sent.push(text); }
    close() { this.closed = true; this.onclose?.(); }
    message(data) { this.onmessage?.({ data: frame(data) }); }
  }
  const context = { document, WebSocket, location: { protocol: 'http:', host: 'localhost:8000' },
    crypto: require('node:crypto').webcrypto, Uint8Array, ArrayBuffer, DataView, performance: { now: () => now },
    console: { info() {}, error() {} }, CustomEvent: class { constructor(type, args) { this.detail = args.detail; } },
    window: { dispatchEvent: e => states.push(e.detail), addEventListener: (name, fn) => handlers[name] = fn, removeEventListener: name => delete handlers[name] },
    setInterval: (fn, ms) => { tasks.set(++id, { fn, at: now + ms, ms }); return id; },
    setTimeout: (fn, ms) => { tasks.set(++id, { fn, at: now + ms }); return id; },
    clearInterval: id => tasks.delete(id), clearTimeout: id => tasks.delete(id) };
  vm.runInNewContext(script, context);
  assert.equal(sockets.length, 0); // loading JS never starts a connection
  const client = context.VoltSession.create(state => states.push(state));
  client.start();
  return { client, sockets, states, document, advance(ms) {
    const end = now + ms;
    while (true) {
      const entry = [...tasks].filter(([, t]) => t.at <= end).sort((a,b) => a[1].at - b[1].at)[0];
      if (!entry) break;
      const [id, t] = entry; now = t.at;
      if (t.ms) t.at += t.ms; else tasks.delete(id);
      t.fn();
    }
    now = end;
  }};
}
const a = browser();
assert.match(a.document.cookie, /^volt_session=[0-9a-f]{32}$/);
const first = a.sockets[0]; first.message('ready'); first.message('ping:3');
assert.deepEqual(Array.from(first.sent[0]), Array.from(new Uint8Array(frame('pong:3'))));
a.advance(5000); assert.deepEqual(Array.from(first.sent.at(-1)), Array.from(new Uint8Array(frame('ping:1'))));
first.message('pong:1'); a.advance(5000); assert.deepEqual(Array.from(first.sent.at(-1)), Array.from(new Uint8Array(frame('ping:2'))));
first.message('replaced'); a.advance(20000);
assert.equal(a.sockets.length, 1); assert(first.closed); assert.equal(a.states.at(-1), 'replaced');
const b = browser(a.document.cookie); assert.equal(b.document.cookie, a.document.cookie);
b.sockets[0].message('ready'); b.advance(7000); assert(b.sockets[0].closed);
b.advance(1000); assert.equal(b.sockets.length, 2);
b.sockets[0].message('ready'); assert.equal(b.states.at(-1), 'connecting');
b.sockets[1].message('ready'); assert.equal(b.states.at(-1), 'connected');
const c = browser(); c.sockets[0].message('ready'); c.sockets[0].message('bad'); c.advance(1000); assert.equal(c.sockets.length, 2);
console.log('PASS: browser cookie reuse, immediate pong, periodic ping, timeout reconnect, stale callback rejection and no reconnect after replacement');

const d = browser(); d.sockets[0].message('ready');
const stale = d.sockets[0].onmessage;
d.client.dispose(); const notifications = d.states.length;
stale({data: 'replaced'}); d.advance(20000);
assert.equal(d.states.length, notifications); assert.equal(d.sockets.length, 1);
d.client.dispose(); d.client.start(); assert.equal(d.sockets.length, 1);
const e = browser(); e.client.stop(); e.advance(20000); assert.equal(e.sockets.length, 1);
e.client.start(); assert.equal(e.sockets.length, 2);
console.log('PASS: explicit start/stop and disposal cancel timers and stale callbacks');
