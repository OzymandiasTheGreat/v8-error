import test from 'brittle'

test('V8', async (t) => {
  t.comment('V8 version', global._v8runtime().version)
})
