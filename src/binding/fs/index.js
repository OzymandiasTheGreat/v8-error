import nodepath from 'path'
import b4a from 'b4a'
import { Readable } from 'stream'
import { getFSBindings } from '../native'
import { parseFileMode, stringToFlags } from './util'
import constants from './constants'

const binding = getFSBindings()
const defaultCallback = (err) => {
  if (err) {
    throw err
  }
}

// async api

export function open(path, flags, mode, callback) {
  if (arguments.length < 3) {
    callback = flags
    flags = 'r'
    mode = 0o666
  } else if (typeof mode === 'function') {
    callback = mode
    mode = 0o666
  } else {
    mode = parseFileMode(mode, 0o666)
  }
  const flagsNumber = stringToFlags(flags)
  return binding.fs_open(path, flagsNumber, mode, callback ?? defaultCallback)
}

export function close(fd, callback) {
  return binding.fs_close(fd, callback ?? defaultCallback)
}

export function fstat(fd, options, callback) {
  if (typeof options === 'function') {
    callback = options
  }
  return binding.fs_fstat(fd, callback ?? defaultCallback)
}

export function stat(path, options, callback) {
  if (typeof options === 'function') {
    callback = options
  }
  return binding.fs_stat(path, callback ?? defaultCallback)
}

export function read(fd, buffer, offsetOrOptions, length, position, callback) {
  let offset = offsetOrOptions
  let params = null
  if (arguments.length <= 4) {
    if (arguments.length === 4) {
      // This is fs.read(fd, buffer, options, callback)
      callback = length
      params = offsetOrOptions
    } else if (arguments.length === 3) {
      // This is fs.read(fd, bufferOrParams, callback)
      if (!b4a.isBuffer(buffer)) {
        // This is fs.read(fd, params, callback)
        params = buffer
        ;({ buffer = b4a.alloc(16384) } = params ?? {})
      }
      callback = offsetOrOptions
    } else {
      // This is fs.read(fd, callback)
      callback = buffer
      buffer = b4a.alloc(16384)
    }

    ;({
      offset = 0,
      length = buffer.byteLength - offset,
      position = null
    } = params ?? {})
  }

  if (offset == null) {
    offset = 0
  }

  length |= 0

  if (length === 0) {
    return setImmediate(function tick() {
      callback(null, 0, buffer)
    })
  }

  if (position == null) {
    position = -1
  }

  function wrapper(err, bytesRead) {
    // Retain a reference to buffer so that it can't be GC'ed too soon.
    ;(callback ?? defaultCallback)(err, bytesRead || 0, buffer)
  }

  return binding.fs_read(fd, buffer, offset, length, position, wrapper)
}

export function write(fd, buffer, offsetOrOptions, length, position, callback) {
  function wrapper(err, written) {
    // Retain a reference to buffer so that it can't be GC'ed too soon.
    ;(callback ?? defaultCallback)(err, written || 0, buffer)
  }

  let offset = offsetOrOptions
  callback = (callback || position || length || offset) ?? defaultCallback

  if (typeof offset === 'object') {
    ;({
      offset = 0,
      length = buffer.byteLength - offset,
      position = null
    } = offsetOrOptions ?? {})
  }

  if (offset == null || typeof offset === 'function') {
    offset = 0
  }
  if (typeof length !== 'number') length = buffer.byteLength - offset
  if (typeof position !== 'number') {
    position = null
  }
  return binding.fs_write(fd, buffer, offset, length, position, wrapper)
}

export function mkdir(path, options, callback) {
  let mode = 0o777
  let recursive = false
  if (typeof options === 'function') {
    callback = options
  } else if (typeof options === 'number' || typeof options === 'string') {
    mode = parseFileMode(options, 0o777)
  } else if (options) {
    if (options.recursive !== undefined) recursive = options.recursive
    if (options.mode !== undefined) mode = parseFileMode(options.mode, 0o777)
  }

  if (recursive) {
    try {
      const items = []
      let dirpath = path
      let keepLooping = true
      while (keepLooping) {
        let exists = false
        try {
          binding.fs_stat(dirpath)
          exists = true
          keepLooping = false
        } catch (err) {
          if (err.code !== 'ENOENT') {
            throw err
          }
        }
        if (!exists) {
          const components = dirpath.split('/')
          if (components.length === 0) {
            keepLooping = false
          } else {
            items.push(components[components.length - 1])
            dirpath = nodepath.dirname(dirpath)
          }
        }
      }
      let newDirPath = ''
      for (let i = 0, l = items.length; i < l; i += 1) {
        newDirPath += `/${items[l - 1 - i]}`
        binding.fs_mkdir(dirpath + newDirPath, mode)
      }
      return (callback ?? defaultCallback)(undefined)
    } catch (err) {
      return (callback ?? defaultCallback)(err)
    }
  }
  return binding.fs_mkdir(path, mode, callback ?? defaultCallback)
}

export function rmdir(path, options, callback) {
  if (typeof options === 'function') {
    callback = options
  }
  return binding.fs_rmdir(path, callback ?? defaultCallback)
}

export function ftruncate(fd, len = 0, callback) {
  if (typeof len === 'function') {
    callback = len
    len = 0
  }
  len = Math.max(0, len)
  return binding.fs_ftruncate(fd, len, callback ?? defaultCallback)
}

export function unlink(path, callback) {
  return binding.fs_unlink(path, callback ?? defaultCallback)
}

export function rename(oldPath, newPath, callback) {
  return binding.fs_rename(oldPath, newPath, callback ?? defaultCallback)
}

export function readFile(path, options, callback) {
  let flag = 'r'
  let encoding = 'buffer'
  callback = callback ?? defaultCallback
  if (typeof options === 'function') {
    callback = options
  }
  if (typeof options === 'string') {
    encoding = options
  }
  if (typeof options === 'object') {
    flag = options.flag ?? 'r'
    encoding = options.encoding ?? 'buffer'
  }
  if (typeof path === 'number') {
    return binding.fs_fstat(path, (err, stat) => {
      const buffer = b4a.alloc(stat.size)
      binding.fs_read(path, buffer, 0, stat.size, -1, (err) => {
        if (encoding === 'buffer') {
          callback(err, buffer)
        } else {
          callback(err, b4a.toString(buffer, encoding))
        }
      })
    })
  }
  return binding.fs_open(path, stringToFlags(flag), 0o666, (err, fd) => {
    binding.fs_fstat(fd, (err, stat) => {
      if (err) {
        return callback(err)
      }
      const buffer = b4a.alloc(stat.size)
      binding.fs_read(fd, buffer, 0, stat.size, -1, (errr) => {
        binding.fs_close(fd, (errc) => {
          if (encoding === 'buffer') {
            callback(errr || errc, buffer)
          } else {
            callback(errr || errc, b4a.toString(buffer, encoding))
          }
        })
      })
    })
  })
}

export function writeFile(path, data, options, callback) {
  callback = (callback || options) ?? defaultCallback
  let encoding = 'utf8'
  let mode = 0o666
  let flag = stringToFlags('w')
  if (typeof options === 'object') {
    encoding = options.encoding ?? encoding
    mode = parseFileMode(options.mode, 0o666)
    flag = options.flag ? stringToFlags(options.flag) : flag
  }
  const buffer =
    b4a.isBuffer(data) || encoding === 'buffer'
      ? b4a.from(data)
      : b4a.from(data, encoding)
  if (typeof path === 'number') {
    return binding.fs_write(path, buffer, 0, buffer.byteLength, -1, (err) =>
      callback(err)
    )
  }
  return binding.fs_open(path, flag, mode, (err, fd) => {
    if (err) {
      return callback(err)
    }
    binding.fs_write(fd, buffer, 0, buffer.byteLength, -1, (errw) => {
      binding.fs_close(fd, (errc) => callback(errw || errc))
    })
  })
}

export function createReadStream(path, options) {
  options ??= {}
  const flags = stringToFlags(options.flags)
  const encoding =
    (typeof options === 'string' ? options : options.encoding) ?? null
  const fd = options.fd ?? null
  const mode = parseFileMode(options.mode, 0o666)
  const autoClose = !!options.autoClose ?? true
  const emitClose = !!options.emitClose ?? true
  const start = options.start ?? 0
  const end = options.end ?? Infinity
  const highWaterMark = options.highWaterMark || 64 * 1024
  let position = start
  return Readable({
    encoding,
    emitClose,
    autoDestroy: autoClose,
    highWaterMark,
    read(size) {
      if (position + size >= end) {
        size = end - position
      }
      const buffer = b4a.alloc(size)
      if (!this.fd) {
        binding.fs_open(path, flags, mode, (err, fd) => {
          if (err) {
            return callback(err)
          }
          this.fd = fd
          this.path = path
          binding.fs_read(
            this.fd,
            buffer,
            0,
            buffer.byteLength,
            position,
            (err, bytesRead) => {
              if (err) {
                return this.destroy(err)
              }
              if (!bytesRead) {
                return this.push(null)
              }
              position += bytesRead
              this.push(buffer)
              if (position >= end) {
                this.push(null)
              }
            }
          )
        })
      } else {
        binding.fs_read(
          this.fd,
          buffer,
          0,
          buffer.byteLength,
          position,
          (err, bytesRead) => {
            if (err) {
              return this.destroy(err)
            }
            if (!bytesRead) {
              return this.push(null)
            }
            position += bytesRead
            this.push(buffer)
            if (position >= end) {
              this.push(null)
            }
          }
        )
      }
    },
    destroy(err, callback) {
      binding.fs_close(this.fd, (errc) => callback(err || errc))
    }
  })
}

// sync api

export function openSync(path, flags, mode) {
  const flagsNumber = stringToFlags(flags)
  mode = parseFileMode(mode, 0o666)
  return binding.fs_open(path, flagsNumber, mode)
}

export function closeSync(fd) {
  return binding.fs_close(fd)
}

export function fstatSync(fd, options) {
  return binding.fs_fstat(fd)
}

export function statSync(path, options) {
  return binding.fs_stat(path)
}

export function readSync(fd, buffer, offset, length, position) {
  if (arguments.length <= 3) {
    // Assume fs.readSync(fd, buffer, options)
    const options =
      offset ||
      {}(
        ({
          offset = 0,
          length = buffer.byteLength - offset,
          position = null
        } = options)
      )
  }

  if (offset == null) {
    offset = 0
  }

  length |= 0

  if (length === 0) {
    return 0
  }

  if (position == null) {
    position = -1
  }

  return binding.fs_read(fd, buffer, offset, length, position)
}

export function writeSync(fd, buffer, offsetOrOptions, length, position) {
  let offset = offsetOrOptions
  if (typeof offset === 'object') {
    ;({
      offset = 0,
      length = buffer.byteLength - offset,
      position = null
    } = offsetOrOptions ?? {})
  }
  if (position == null) position = -1
  if (offset == null) {
    offset = 0
  }
  if (typeof length !== 'number') {
    length = buffer.byteLength - offset
  }
  return binding.fs_write(fd, buffer, offset, length, position)
}

export function mkdirSync(path, options) {
  let mode = 0o777
  let recursive = false
  if (typeof options === 'number' || typeof options === 'string') {
    mode = parseFileMode(options, 0o777)
  } else if (options) {
    if (options.recursive !== undefined) recursive = options.recursive
    if (options.mode !== undefined) mode = parseFileMode(options.mode, 0o777)
  }

  if (recursive) {
    const items = []
    let dirpath = path
    let keepLooping = true
    while (keepLooping) {
      let exists = false
      try {
        binding.fs_stat(dirpath)
        exists = true
        keepLooping = false
      } catch (err) {
        if (err.code !== 'ENOENT') {
          throw err
        }
      }
      if (!exists) {
        const components = dirpath.split('/')
        if (components.length === 0) {
          keepLooping = false
        } else {
          items.push(components[components.length - 1])
          dirpath = nodepath.dirname(dirpath)
        }
      }
    }
    let newDirPath = ''
    for (let i = 0, l = items.length; i < l; i += 1) {
      newDirPath += `/${items[l - 1 - i]}`
      binding.fs_mkdir(dirpath + newDirPath, mode)
    }
    return `${dirpath}/${items[items.length - 1]}`
  }

  return binding.fs_mkdir(path, mode)
}

export function rmdirSync(path, options) {
  return binding.fs_rmdir(path)
}

export function ftruncateSync(fd, len = 0) {
  len = Math.max(0, len)
  return binding.fs_ftruncate(fd, len)
}

export function unlinkSync(path) {
  return binding.fs_unlink(path)
}

export function renameSync(oldPath, newPath) {
  return binding.fs_rename(oldPath, newPath)
}

export function readFileSync(path, options) {
  let flag = 'r'
  let encoding = 'buffer'
  if (typeof options === 'string') {
    encoding = options
  }
  if (typeof options === 'object') {
    flag = options.flag ?? 'r'
    encoding = options.encoding ?? 'buffer'
  }
  if (typeof path === 'number') {
    const stat = binding.fs_fstat(path)
    const buffer = b4a.alloc(stat.size)
    binding.fs_read(path, buffer, 0, stat.size, -1)
    if (encoding !== 'buffer') {
      return b4a.toString(buffer, encoding)
    }
    return buffer
  }
  const fd = binding.fs_open(path, stringToFlags(flag), 0o666)
  const stat = binding.fs_fstat(fd)
  const buffer = b4a.alloc(stat.size)
  binding.fs_read(fd, buffer, 0, stat.size, -1)
  if (encoding !== 'buffer') {
    return b4a.toString(buffer, encoding)
  }
  return buffer
}

export function writeFileSync(path, data, options) {
  let encoding = 'utf8'
  let mode = 0o666
  let flag = 'w'
  if (typeof options === 'object') {
    encoding = options.encoding ?? encoding
    mode = parseFileMode(options.mode, 0o666)
    flag = options.flag ?? flag
  }
  const buffer =
    b4a.isBuffer(data) || encoding === 'buffer'
      ? b4a.from(data)
      : b4a.from(data, encoding)
  if (typeof path === 'number') {
    binding.fs_write(path, buffer, 0, buffer.byteLength, -1)
    return
  }
  const fd = binding.fs_open(path, stringToFlags(flag), mode)
  binding.fs_write(fd, buffer, 0, buffer.byteLength, -1)
  binding.fs_close(fd)
}

export { constants }

export default {
  constants,

  open,
  close,
  fstat,
  stat,
  read,
  write,
  mkdir,
  rmdir,
  ftruncate,
  unlink,
  rename,
  readFile,
  writeFile,
  createReadStream,

  openSync,
  closeSync,
  fstatSync,
  statSync,
  readSync,
  writeSync,
  mkdirSync,
  rmdirSync,
  ftruncateSync,
  unlinkSync,
  renameSync,
  readFileSync,
  writeFileSync
}
