import { getFSBindings } from '../native'

const binding = getFSBindings()

export const O_RDWR = binding.O_RDWR
export const O_RDONLY = binding.O_RDONLY
export const O_WRONLY = binding.O_WRONLY
export const O_CREAT = binding.O_CREAT
export const O_TRUNC = binding.O_TRUNC
export const O_APPEND = binding.O_APPEND

export default {
  O_APPEND,
  O_CREAT,
  O_RDONLY,
  O_RDWR,
  O_TRUNC,
  O_WRONLY
}
