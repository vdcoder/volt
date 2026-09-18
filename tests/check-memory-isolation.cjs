const assert = require('node:assert/strict');
const { resolve } = require('node:path');
(async () => {
    const factory = require(resolve(process.argv[2]));
    const [a, b] = await Promise.all([factory(), factory()]);
    a.initialize(10); b.initialize(20);
    assert.equal(a.changeCount(), 2); assert.equal(b.changeCount(), 2);
    await Promise.resolve().then(() => a.writeNumber(11));
    await new Promise(done => setTimeout(() => { b.writeNumber(21); done(); }, 1));
    assert.equal(a.readNumber(), 11); assert.equal(b.readNumber(), 21);
    a.writeNumber(11);
    assert.equal(a.changeCount(), 3); assert.equal(b.changeCount(), 3);
    console.log('PASS: authoring memory singletons and handle views are isolated per MEMORY64 instance');
})().catch(error => { console.error(error); process.exitCode = 1; });
