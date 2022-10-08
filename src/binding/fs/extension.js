import { getFSBindings } from '../native'

const binding = getFSBindings()

export function tryLock(fd, offset = 0, length = 0, opts = {}) {
  // https://github.com/hypercore-skunkworks/fs-native-extensions/blob/main/index.js
  if (typeof offset === 'object') {
    opts = offset
    offset = 0
  }

  if (typeof length === 'object') {
    opts = length
    length = 0
  }

  if (typeof opts !== 'object') {
    opts = {}
  }

  const err = binding.fs_try_lock(fd, offset, length, opts.shared ? 0 : 1)
  if (err) {
    if (err.code === 'EAGAIN') return false
    throw err
  }

  return true
}

export async function sparse() {}

export async function trim(fd, offset = 0, length = 0) {
  const err = binding.fs_trim(fd, offset, length)
  if (err) {
    throw err
  }
}

export default { tryLock, sparse, trim }
