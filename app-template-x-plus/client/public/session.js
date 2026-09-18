// Reusable JS transport. Each C++ service owns one instance from this factory.
globalThis.VoltSession = {
  envelope(id, payload = new Uint8Array()) {
    if (!Number.isInteger(id) || id < 0 || id > 65535) throw new Error('Invalid talker ID');
    const bytes = new Uint8Array(2 + payload.length);
    new DataView(bytes.buffer).setUint16(0, id, true); bytes.set(payload, 2); return bytes;
  },
  control(op, token = 0n) {
    const payload = new Uint8Array(op === 2 || op === 3 ? 9 : 1);
    payload[0] = op;
    if (payload.length === 9) new DataView(payload.buffer).setBigUint64(1, BigInt(token), true);
    return this.envelope(0, payload);
  },
  create(onState, onMessage) {
    let socket, timer, retry, transmit, stopped = true, disposed = false, superseded = false, resume = false;
    const status = state => {
      if (disposed) return;
      console.info(`Volt session: ${state}`);
      onState?.(state);
    };
    const clear = () => {
      transmit = null; clearInterval(timer); clearTimeout(retry);
      const old = socket; socket = null;
      if (old) { old.onmessage = old.onerror = old.onclose = null; try { old.close(); } catch {} }
    };
    function ensureCookie() {
      const name = 'volt_session';
      const read = () => document.cookie.split(';').map(x => x.trim()).find(x => x.startsWith(name + '='))?.slice(name.length + 1);
      let id = read();
      if (!/^[0-9a-f]{32}$/.test(id || '')) {
        id = [...crypto.getRandomValues(new Uint8Array(16))].map(x => x.toString(16).padStart(2, '0')).join('');
        document.cookie = `${name}=${id}; Path=/; SameSite=Strict${location.protocol === 'https:' ? '; Secure' : ''}`;
      }
      if (read() !== id) throw new Error('Volt session requires cookies.');
    }
    function connect() {
      if (stopped || disposed) return;
      let ws;
      try {
        ensureCookie();
        ws = new WebSocket(`${location.protocol === 'https:' ? 'wss:' : 'ws:'}//${location.host}/session`);
      } catch { stopped = true; status('error'); return; }
      socket = ws; ws.binaryType = 'arraybuffer';
      let ready = false, nextPing = 0, deadline = 0, sequence = 0n, pending = null;
      const openingDeadline = performance.now() + 7000;
      const current = () => socket === ws && !stopped && !disposed;
      const fail = (reason = "connection failure") => {
        if (!current()) return;
        console.warn?.(`Volt session disconnected: ${reason}`);
        clear(); status('disconnected');
        if (!stopped && !disposed) retry = setTimeout(connect, 1000);
      };
      const check = () => {
        const now = performance.now();
        if ((!ready && now >= openingDeadline) || (pending !== null && now >= deadline)) { fail(ready ? 'pong deadline exceeded' : 'server Ready deadline exceeded'); return false; }
        return true;
      };
      ws.onmessage = event => {
        if (!current()) return;
        try {
          if (!(event.data instanceof ArrayBuffer) || event.data.byteLength < 2) { fail('expected a binary envelope'); return; }
          const bytes = new Uint8Array(event.data), data = new DataView(event.data);
          const id = data.getUint16(0, true);
          if (id) {
            if (!check() || !ready) { fail(); return; }
            if (onMessage?.(id, bytes.subarray(2)) !== true) fail(`talker ${id} rejected message`);
            return;
          }
          const op = bytes[2];
          if ((op === 1 || op === 4) ? bytes.length !== 3 : ((op !== 2 && op !== 3) || bytes.length !== 11)) { fail(); return; }
          if (op === 4) { superseded = true; stopped = true; clear(); status('replaced'); return; }
          if (!check()) return;
          if (!ready) {
            if (op !== 1) { fail('expected initial Ready'); return; }
            ready = true; nextPing = performance.now() + 5000; status('connected'); return;
          }
          if (op !== 2 && op !== 3) { fail(); return; }
          const token = data.getBigUint64(3, true);
          if (!token) { fail(); return; }
          if (op === 2) ws.send(VoltSession.control(3, token));
          else if (token === pending) pending = null;
          else fail();
        } catch (error) { fail(`message handler threw: ${error}`); }
      };
      transmit = (id, payload) => {
        if (!current() || !check() || !ready || id === 0) return false;
        try { ws.send(VoltSession.envelope(id, payload)); return true; }
        catch { fail(); return false; }
      };
      ws.onerror = () => fail('WebSocket error');
      ws.onclose = event => fail(`peer closed connection (code ${event.code})`);
      timer = setInterval(() => {
        if (!current() || !check() || !ready) return;
        const now = performance.now();
        if (pending === null && now >= nextPing) {
          pending = ++sequence; deadline = now + 2000; nextPing = now + 5000;
          try { ws.send(VoltSession.control(2, pending)); } catch { fail(); }
        }
      }, 100);
      status('connecting');
    }
    const hide = () => { resume = !stopped; stopped = true; clear(); status('disconnected'); };
    const show = event => { if (event.persisted && resume && !superseded && !disposed) { stopped = false; connect(); } };
    window.addEventListener('pagehide', hide);
    window.addEventListener('pageshow', show);
    return {
      reconnect() {
        if (stopped || disposed) return;
        clear(); status('disconnected'); retry = setTimeout(connect, 1000);
      },
      send(id, payload) { return transmit ? transmit(id, payload) : false; },
      start() { if (disposed || !stopped) return; superseded = false; stopped = false; connect(); },
      stop() { if (disposed) return; stopped = true; resume = false; clear(); status('stopped'); },
      dispose() {
        if (disposed) return;
        disposed = true; stopped = true; onState = onMessage = null; clear();
        window.removeEventListener('pagehide', hide);
        window.removeEventListener('pageshow', show);
      }
    };
  }
};
