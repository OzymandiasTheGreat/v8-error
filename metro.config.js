/**
 * Metro configuration for React Native
 * https://github.com/facebook/react-native
 *
 * @format
 */

module.exports = {
  transformer: {
    minifierPath: require.resolve('metro-minify-esbuild'),
    minifierConfig: {},
    getTransformOptions: async () => ({
      transform: {
        experimentalImportSupport: false,
        inlineRequires: true
      }
    })
  },
  resolver: {
    resolveRequest(context, moduleName, platform) {
      if (moduleName === 'random-access-file') {
        return {
          filePath: require.resolve('random-access-file/index.js'),
          type: 'sourceFile'
        }
      } else if (moduleName === 'b4a') {
        return {
          filePath: require.resolve('./node_modules/b4a/browser.js'),
          type: 'sourceFile'
        }
      } else if (moduleName === 'events') {
        return {
          filePath: require.resolve('./src/binding/events'),
          type: 'sourceFile'
        }
      } else if (moduleName === 'os') {
        return {
          filePath: require.resolve('./src/binding/os.js'),
          type: 'sourceFile'
        }
      } else if (moduleName === 'fs') {
        return {
          filePath: require.resolve('./src/binding/fs/index.js'),
          type: 'sourceFile'
        }
      } else if (moduleName === 'fs/promises') {
        return {
          filePath: require.resolve('./src/binding/fs/promises.js'),
          type: 'sourceFile'
        }
      } else if (moduleName === 'fs/constants' || moduleName === 'constants') {
        return {
          filePath: require.resolve('./src/binding/fs/constants.js'),
          type: 'sourceFile'
        }
      } else if (
        moduleName === 'fs-native-extensions' ||
        moduleName === 'fsctl'
      ) {
        return {
          filePath: require.resolve('./src/binding/fs/extension.js'),
          type: 'sourceFile'
        }
      }
      return context.resolveRequest(context, moduleName, platform)
    }
  }
}
