const assert = require('node:assert/strict');
const fs = require('node:fs'), path = require('node:path'), vm = require('node:vm');
const { setTimeout: delay } = require('node:timers/promises');
const requests = [];
global.fetch = (url, options) => new Promise((resolve, reject) => requests.push({url, options, resolve, reject}));
vm.runInThisContext(fs.readFileSync(path.join(__dirname, '../app-template-x-plus/client/public/http.js'), 'utf8'));
const response = (status, body) => ({status, headers: new Map([['x-reply', 'ok']]), text: async () => body});
(async () => {
    const factory = require(path.resolve(process.argv[2]));
    const a = await factory(), b = await factory();
    a.initialize(); b.initialize();
    try {
        a.request('https://example.invalid/api', 'POST', 'hello', 1000);
        b.request('/missing', 'GET', '', 1000);
        assert.equal(requests[0].url, 'https://example.invalid/api');
        assert.equal(requests[0].options.body, 'hello');
        assert.equal(requests[0].options.headers['X-Test'], 'yes');
        assert(!('body' in requests[1].options));
        requests[0].resolve(response(201, 'created')); requests[1].resolve(response(404, 'missing'));
        await delay(0);
        assert.equal(a.calls(), 1); assert.equal(a.status(), 201); assert.equal(a.body(), 'created'); assert.equal(a.header(), 'ok');
        assert.equal(b.status(), 404); assert.equal(b.error(), '');
        const id = a.request('/cancel', 'GET', '', 1000);
        a.cancel(id); a.cancel(id);
        assert(requests[2].options.signal.aborted); assert.equal(a.error(), 'cancelled');
        requests[2].resolve(response(200, 'late')); await delay(0); assert.equal(a.calls(), 2);
        a.request('/timeout', 'GET', '', 10); await delay(30);
        assert.equal(a.error(), 'timeout'); assert(requests[3].options.signal.aborted);
        a.request('/network', 'GET', '', 1000); requests[4].reject(new Error('offline')); await delay(0);
        assert.match(a.error(), /offline/); assert.equal(a.status(), 0);
        a.request('/dispose', 'GET', '', 1000); a.release();
        assert(requests[5].options.signal.aborted);
        requests[5].resolve(response(200, 'late')); await delay(0); assert.equal(a.calls(), 4);
        console.log('PASS: MEMORY64 HTTP request/response, headers, HTTP errors, network errors, timeout, cancellation, disposal and module isolation');
    } finally { a.release(); b.release(); }
})().catch(error => { console.error(error); process.exitCode = 1; });
