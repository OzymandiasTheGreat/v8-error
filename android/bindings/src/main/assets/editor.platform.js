window.keet_native_send = function (args) {
  var command = args[0]
  var arg = args[1]
  if (command === 'height') keet.height(arg)
  else if (command === 'path') keet.path(arg)
  else if (command === 'html') keet.html(arg)
  else if (command === 'ready') keet.ready()
}
window.keet_command_start()
