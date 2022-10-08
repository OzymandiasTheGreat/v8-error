import { EventEmitter2 } from 'eventemitter2'
import fs from '.'
import constants from './constants'

class FileHandle extends EventEmitter2 {
  constructor(fd) {
    this._fd = fd
  }

  get fd() {
    return this._fd
  }

  async close() {
    return new Promise((resolve, reject) =>
      fs.close(this.fd, (err) => {
        this.emit('close')
        if (err) {
          return reject(err)
        }
        resolve()
      })
    )
  }

  createReadStream(options = {}) {
    return fs.createReadStream('', { ...options, fd: this.fd })
  }

  async read(...args) {
    return new Promise((resolve, reject) =>
      fs.read(this.fd, ...args, (err, bytesRead, buffer) => {
        if (err) {
          return reject(err)
        }
        resolve({ bytesRead, buffer })
      })
    )
  }

  async readFile(options = {}) {
    return new Promise((resolve, reject) =>
      fs.readFile(this.fd, options, (err, data) => {
        if (err) {
          return reject(err)
        }
        resolve(data)
      })
    )
  }

  async stat(options = {}) {
    return new Promise((resolve, reject) =>
      fs.fstat(this.fd, options, (err, stat) => {
        if (err) {
          return reject(err)
        }
        resolve(stat)
      })
    )
  }

  async truncate(len = 0) {
    return new Promise((resolve, reject) =>
      fs.ftruncate(this.fd, len, (err) => {
        if (err) {
          return reject(err)
        }
        resolve()
      })
    )
  }

  async write(...args) {
    return new Promise((resolve, reject) =>
      fs.write(this.fd, ...args, (err, bytesWritten, buffer) => {
        if (err) {
          return reject(err)
        }
        resolve({ bytesWritten, buffer })
      })
    )
  }

  async writeFile(data, options = {}) {
    return new Promise((resolve, reject) =>
      fs.writeFile(this.fd, data, options, (err) => {
        if (err) {
          return reject(err)
        }
        resolve()
      })
    )
  }
}

export async function mkdir(path, options) {
  return new Promise((resolve, reject) =>
    fs.mkdir(path, options, (err, firstDir) => {
      if (err) {
        return reject(err)
      }
      resolve(firstDir)
    })
  )
}

export async function open(path, flags, mode) {
  return new Promise((resolve, reject) =>
    fs.open(path, flags, mode, (err, fd) => {
      if (err) {
        return reject(err)
      }
      resolve(new FileHandle(fd))
    })
  )
}

export async function readFile(path, options) {
  if (path instanceof FileHandle) {
    return path.readFile(options)
  }
  return new Promise((resolve, reject) =>
    fs.readFile(path, options, (err, data) => {
      if (err) {
        return reject(err)
      }
      resolve(data)
    })
  )
}

export async function rename(oldPath, newPath) {
  return new Promise((resolve, reject) =>
    fs.rename(oldPath, newPath, (err) => {
      if (err) {
        return reject(err)
      }
      resolve()
    })
  )
}

export async function rmdir(path, options) {
  return new Promise((resolve, reject) =>
    fs.rmdir(path, options, (err) => {
      if (err) {
        return reject(err)
      }
      resolve()
    })
  )
}

export async function stat(path, options) {
  return new Promise((resolve, reject) =>
    fs.stat(path, options, (err, stat) => {
      if (err) {
        return reject(err)
      }
      resolve(stat)
    })
  )
}

export async function unlink(path) {
  return new Promise((resolve, reject) =>
    fs.unlink(path, (err) => {
      if (err) {
        return reject(err)
      }
      resolve()
    })
  )
}

export async function writeFile(file, data, options = {}) {
  if (file instanceof FileHandle) {
    return file.writeFile(data, options)
  }
  return new Promise((resolve, reject) =>
    fs.writeFile(file, data, options, (err) => {
      if (err) {
        return reject(err)
      }
      resolve()
    })
  )
}

export { constants }

export default {
  constants,

  mkdir,
  open,
  readFile,
  rename,
  rmdir,
  stat,
  unlink,
  writeFile
}
