require('./src/binding/polyfills')

import { AppRegistry, Platform } from 'react-native'
import { name as appName } from './app.json'

require('./test/all')

const TestApp = () => {
  return null
}

AppRegistry.registerComponent(appName, () => TestApp)
