const assert = require('node:assert/strict');
const path = require('node:path');
const factory = require(path.resolve(process.argv[2]));
(async () => {
    const [a, b] = await Promise.all([factory(), factory()]);
    a.initialize(100); b.initialize(200);
    a.schedule(25); b.schedule(5); a.schedule(10); b.schedule(30);
    await Promise.resolve().then(() => a.invalidate());
    await new Promise(resolve => setTimeout(() => { b.invalidate(); resolve(); }, 1));
    const deadline = Date.now() + 5000;
    while (a.readInvalidations() !== 3 || b.readInvalidations() !== 3) {
        assert.ok(Date.now() < deadline, 'async callbacks timed out');
        await new Promise(resolve => setTimeout(resolve, 10));
    }
    assert.equal(a.readService(), 102);
    assert.equal(b.readService(), 202);
    a.invalidate();
    assert.equal(a.readInvalidations(), 4);
    assert.equal(b.readInvalidations(), 3);
    console.log('PASS: AppDI services and runtime references remain isolated across MEMORY64 instances and async callbacks.');
})().catch(error => { console.error(error); process.exitCode = 1; });
