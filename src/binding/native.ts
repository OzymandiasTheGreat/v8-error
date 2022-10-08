import { NativeModules } from 'react-native'

const { HolepunchBindings } = NativeModules

export const FS_BASE: string = HolepunchBindings.FS_BASE
export const FS_TMP_BASE: string = HolepunchBindings.FS_TMP_BASE

export const hideLaunchScreen: () => void = HolepunchBindings.hideLaunchScreen

__hlp_uv.setup()
__hlp_fs.setup()
__hlp_thrower = (err) => {
  throw err
}

export const getFSBindings = () => {
  return __hlp_fs
}
