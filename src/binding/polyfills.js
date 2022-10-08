// https://github.com/floatdrop/capture-stack-trace
Error.captureStackTrace = function captureStackTrace(error) {
  var container = new Error()

  Object.defineProperty(error, 'stack', {
    configurable: true,
    get: function getStack() {
      var stack = container.stack

      Object.defineProperty(this, 'stack', {
        value: stack
      })

      return stack
    }
  })
}

// https://github.com/then/promise/blob/f7e26e9d982c61ad599a26b6e39e5aa27a24ea2d/src/es6-extensions.js#L101
Promise.allSettled = function (iterable) {
  return Promise.all(
    Array.from(iterable).map(function (item) {
      function onFulfill(value) {
        return { status: 'fulfilled', value: value }
      }
      function onReject(reason) {
        return { status: 'rejected', reason: reason }
      }
      if (item && (typeof item === 'object' || typeof item === 'function')) {
        if (item instanceof Promise && item.then === Promise.prototype.then) {
          return item.then(onFulfill, onReject)
        }
        var then = item.then
        if (typeof then === 'function') {
          return new Promise(then.bind(item)).then(onFulfill, onReject)
        }
      }
      return onFulfill(item)
    })
  )
}

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
if (typeof Symbol === undefined || !Symbol.asyncIterator) {
  Symbol.asyncIterator = Symbol.for('Symbol.asyncIterator')
}

global.process.pid = Math.floor(Math.random() * 100000000)
global.process.nextTick = setImmediate
global.Buffer = require('b4a')
