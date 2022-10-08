import test, { configure } from 'brittle'
import RAF from 'random-access-file'
import Buffer from 'b4a'
import path from 'path'
import { mkdir } from 'fs'
import os from 'os'

configure({ serial: true, bail: true, timeout: 1 * 60 * 60 * 1000 })

const tmp = path.join(os.tmpdir(), 'raf-lock-' + process.pid + '-' + Date.now())
mkdir(tmp, { recursive: true }, () => {})

const ensureFile = async (filepath) =>
  new Promise((r) => {
    const file = new RAF(filepath)
    file.write(0, Buffer.from(''), function (err) {
      if (err) throw err
      file.close(function (err) {
        if (err) throw err
        r()
      })
    })
  })

test('2 writers', async function (t) {
  t.plan(4)

  const file = `${tmp}/test/fixture/exclusive.txt`
  await ensureFile(file)

  const a = new RAF(file, { lock: true })
  const b = new RAF(file, { lock: true })

  a.open(function (err) {
    t.absent(err, 'a granted lock')

    b.open(function (err) {
      t.ok(err, 'b denied lock')

      a.close(() => t.pass('a closed'))
      b.close(() => t.pass('b closed'))
    })
  })
})

test('2 readers', async function (t) {
  t.plan(4)

  const file = `${tmp}/test/fixture/shared.txt`
  await ensureFile(file)

  const a = new RAF(file, { lock: true, writable: false })
  const b = new RAF(file, { lock: true, writable: false })

  a.open(function (err) {
    t.absent(err, 'a granted lock')

    b.open(function (err) {
      t.absent(err, 'b granted lock')

      a.close(() => t.pass('a closed'))
      b.close(() => t.pass('b closed'))
    })
  })
})

test('2 readers + 1 writer', async function (t) {
  t.plan(6)

  const file = `${tmp}/test/fixture/shared.txt`
  await ensureFile(file)

  const a = new RAF(file, { lock: true, writable: false })
  const b = new RAF(file, { lock: true, writable: false })
  const c = new RAF(file, { lock: true })

  a.open(function (err) {
    t.absent(err, 'a granted lock')

    b.open(function (err) {
      t.absent(err, 'b granted lock')

      c.open(function (err) {
        t.ok(err, 'c denied lock')

        a.close(() => t.pass('a closed'))
        b.close(() => t.pass('b closed'))
        c.close(() => t.pass('c closed'))
      })
    })
  })
})

test('1 writer + 1 reader', async function (t) {
  t.plan(4)

  const file = `${tmp}/test/fixture/exclusive.txt`
  await ensureFile(file)

  const a = new RAF(file, { lock: true })
  const b = new RAF(file, { lock: true, writable: false })

  a.open(function (err) {
    t.absent(err, 'a granted lock')

    b.open(function (err) {
      t.ok(err, 'b denied lock')

      a.close(() => t.pass('a closed'))
      b.close(() => t.pass('b closed'))
    })
  })
})
