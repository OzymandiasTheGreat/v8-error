const shim = require('util-shim')

module.exports = {
  ...shim,
  getSystemErrorMap() {
    return __hlp_uv_error_map
  }
}
