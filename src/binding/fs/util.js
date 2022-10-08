import {
  O_RDONLY,
  O_RDWR,
  O_TRUNC,
  O_CREAT,
  O_WRONLY,
  O_APPEND
} from './constants'

export function parseFileMode(value, def) {
  value ??= def
  if (typeof value === 'string') {
    return parseInt(value, 8)
  }
  return value
}

export function stringToFlags(flags) {
  if (typeof flags === 'number') {
    return flags
  }

  if (flags == null) {
    return O_RDONLY
  }

  switch (flags) {
    case 'r':
      return O_RDONLY
    case 'r+':
      return O_RDWR

    case 'w':
      return O_TRUNC | O_CREAT | O_WRONLY

    case 'w+':
      return O_TRUNC | O_CREAT | O_RDWR

    case 'a':
      return O_APPEND | O_CREAT | O_WRONLY

    case 'a+':
      return O_APPEND | O_CREAT | O_RDWR
  }
}
