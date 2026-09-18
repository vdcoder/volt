// Node 22+; launches only this test's server and stops it afterward.
const assert = require('node:assert/strict');
const { spawn, spawnSync } = require('node:child_process');
const { readFileSync } = require('node:fs');
const path = require('node:path');
const { setTimeout: delay } = require('node:timers/promises');

async function main() {
  const config = process.argv[3] || 'Debug';
  assert(['Debug', 'Release'].includes(config));
  assert(process.argv[2], 'Usage: node smoke-xplus.cjs <generated-app> [Debug|Release]');
  const root = path.resolve(process.argv[2]);
  const exe = path.join(root, 'output', config, 'server', 'Server.exe');
  const child = spawn(exe, [], { cwd: __dirname, windowsHide: true });
  let log = '';
  child.stdout.on('data', data => { log += data; });
  child.stderr.on('data', data => { log += data; });
  const exited = new Promise(resolve => child.once('exit', resolve));
  let spawnError;
  child.on('error', error => { spawnError = error; });
  try {
    let ready = false;
    for (let i = 0; i < 50; i++) {
      if (spawnError) throw spawnError;
      if (child.exitCode !== null) throw new Error(log);
      try {
        const response = await fetch('http://127.0.0.1:8000/', { signal: AbortSignal.timeout(500) });
        await response.arrayBuffer();
        ready = response.ok;
      } catch {}
      if (ready) break;
      await delay(100);
    }
    assert(ready, `Server did not start: ${log}`);
    await delay(100);
    assert.equal(child.exitCode, null, `Server exited (port already in use?): ${log}`);
    for (const [url, file, mime] of [
      ['/', 'index.html', 'text/html'],
      ['/app.js', 'app.js', 'text/javascript'],
      ['/volt.js', 'volt.js', 'text/javascript'],
      ['/session.js', 'session.js', 'text/javascript'],
      ['/global.css', 'global.css', 'text/css'],
      ['/app.wasm', 'app.wasm', 'application/wasm'],
    ]) {
      const response = await fetch(`http://127.0.0.1:8000${url}`);
      assert.equal(response.status, 200);
      assert(response.headers.get('content-type').startsWith(mime));
      assert.deepEqual(Buffer.from(await response.arrayBuffer()), readFileSync(path.join(root, 'output', config, 'web', file)));
    }
    await new Promise((resolve, reject) => {
      const socket = new WebSocket('ws://127.0.0.1:8000/ws');
      socket.binaryType = 'arraybuffer';
      const timer = setTimeout(() => { socket.close(); reject(new Error('WebSocket timeout')); }, 5000);
      let messages = 0;
      socket.onopen = () => socket.send('Hello from the test: café!');
      socket.onerror = event => { clearTimeout(timer); reject(new Error(`WebSocket error (exit ${child.exitCode}): ${event.message} ${event.error?.stack}\n${log}`)); };
      socket.onmessage = event => {
        try {
          if (messages++ === 0) {
            assert.equal(event.data, 'Hello from the test: café!');
            socket.send(new Uint8Array([0, 1, 127, 255]));
          } else {
            assert.deepEqual(Buffer.from(event.data), Buffer.from([0, 1, 127, 255]));
            socket.close();
          }
        } catch (error) { clearTimeout(timer); socket.close(); reject(error); }
      };
      socket.onclose = event => {
        clearTimeout(timer);
        if (messages === 2 && event.wasClean) resolve(); else reject(new Error('WebSocket closed before both echoes or without clean handshake'));
      };
    });
    const missing = await fetch('http://127.0.0.1:8000/does-not-exist');
    assert.equal(missing.status, 404);
    await missing.arrayBuffer();
    const invalidPort = spawnSync(exe, [path.join(root, 'output', config, 'web'), 'invalid'], { windowsHide: true });
    assert.equal(invalidPort.status, 1);
    const missingRoot = spawnSync(exe, [path.join(root, 'out', 'missing-static-folder')], { windowsHide: true });
    assert.equal(missingRoot.status, 1);
    console.log(`${config}: HTTP files match, WASM MIME correct, missing file 404, text/binary WebSocket echo, startup errors PASS`);
  } catch (error) {
    console.error(log);
    throw error;
  } finally {
    child.kill();
    await exited;
  }
}
main().catch(error => { console.error(error); process.exitCode = 1; });
