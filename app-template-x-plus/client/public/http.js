// Each C++ HttpClientService owns an instance; callbacks belong to its WASM module.
globalThis.VoltHttp = {
  create(onResponse) {
    const pending = new Map();
    let disposed = false;
    function finish(id, status, headers, body, error) {
      const entry = pending.get(id);
      if (!entry || disposed) return;
      pending.delete(id); clearTimeout(entry.timer);
      // Application callback exceptions are not transport failures or second completions.
      try { onResponse(id, status, headers, body, error); }
      catch (error) { console.error('Volt HTTP callback failed', error); }
    }
    return {
      request(id, url, method, headers, body, timeoutMs) {
        if (disposed || pending.has(id)) throw new Error('Invalid HTTP request lifetime');
        const controller = new AbortController();
        const entry = { controller, timer: null };
        pending.set(id, entry);
        entry.timer = setTimeout(() => {
          controller.abort(); finish(id, 0, {}, '', 'timeout');
        }, timeoutMs);
        (async () => {
          try {
            const response = await fetch(url, {
              method, headers, signal: controller.signal,
              ...(method === 'GET' || method === 'HEAD' ? {} : { body })
            });
            const text = await response.text();
            finish(id, response.status, Object.fromEntries(response.headers.entries()), text, '');
          } catch (error) { finish(id, 0, {}, '', String(error)); }
        })();
      },
      cancel(id) {
        const entry = pending.get(id);
        if (!entry) return;
        entry.controller.abort(); finish(id, 0, {}, '', 'cancelled');
      },
      dispose() {
        if (disposed) return;
        disposed = true;
        for (const entry of pending.values()) { clearTimeout(entry.timer); entry.controller.abort(); }
        pending.clear(); onResponse = null;
      }
    };
  }
};
