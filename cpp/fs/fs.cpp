#include "fs.hpp"

#include "uv.hpp"
#include "hlp_macros.h"

#include <fcntl.h>
#include <unistd.h>

#define FS_MODE 0666
#define FS_MODE_MKDIR 0777

using namespace facebook;
using namespace holepunch::fs;

jsi::Value FSBinding::get(jsi::Runtime &rt, const jsi::PropNameID &name) {
  auto propertyName = name.utf8(rt);

  JSI_HOSTOBJECT_METHOD("setup", 0, {
    _uvBinding = rt.global().getProperty(rt, HLP_UV_GLOBAL).asObject(rt).asHostObject<holepunch::uv::UVBinding>(rt);
    return JSI_UNDEFINED;
  });

  // constants

  JSI_HOSTOBJECT_NUMBER("O_RDWR", UV_FS_O_RDWR);
  JSI_HOSTOBJECT_NUMBER("O_RDONLY", UV_FS_O_RDONLY);
  JSI_HOSTOBJECT_NUMBER("O_WRONLY", UV_FS_O_WRONLY);
  JSI_HOSTOBJECT_NUMBER("O_CREAT", UV_FS_O_CREAT);
  JSI_HOSTOBJECT_NUMBER("O_TRUNC", UV_FS_O_TRUNC);
  JSI_HOSTOBJECT_NUMBER("O_APPEND", UV_FS_O_APPEND);

  // methods

  JSI_HOSTOBJECT_METHOD("fs_open", 4, {
    auto filename = JSI_ARG_STRING(0);
    auto flags = JSI_ARG_UINT32(1);
    auto mode = JSI_ARG_UINT32(2);

    bool hasCallback = argc > 3;

    auto req = new FSRequest;
    req->binding = this;
    req->rt = &rt;
    if (hasCallback) {
      req->callback = std::make_unique<jsi::Function>(JSI_ARG_FUNCTION(3));
    }

    auto on_open = [](uv_fs_t* uv_req) {
      auto req = static_cast<FSRequest*>(uv_req->data);

      jsi::Runtime& rt = *req->rt;
      if (req->binding->_callInvoker->isValid()) {
        if (uv_req->result < 0) {
          JSI_MAKE_UV_ERROR(res, static_cast<int>(uv_req->result), req->binding->_uvBinding);
          req->callback->call(rt, res_err);
        } else {
          req->callback->call(rt, JSI_UNDEFINED, JSI_NUMBER(uv_req->result));
          req->binding->_uvBinding->fds.insert(static_cast<uv_file>(uv_req->result));
        }
      }

      uv_fs_req_cleanup(&req->uv_req);
      delete req;
    };
    int res = uv_fs_open(_uvBinding->uv_loop(),
               &req->uv_req,
               filename.data(),
               flags,
               mode,
               hasCallback ? on_open : nullptr);
    if (hasCallback) {
      return JSI_UNDEFINED;
    }

    if (res >= 0) {
      delete req;
      return jsi::Value(res);
    }

    JSI_MAKE_UV_ERROR(exc, res, _uvBinding);
    JSI_CALL_THROWER(exc_err);

    delete req;

    return JSI_UNDEFINED;
  });

  JSI_HOSTOBJECT_METHOD("fs_close", 2, {
    auto fd = JSI_ARG_UINT32(0);

    bool hasCallback = argc > 1;

    auto req = new FSRequest;
    req->binding = this;
    req->rt = &rt;
    if (hasCallback) {
      req->callback = std::make_unique<jsi::Function>(JSI_ARG_FUNCTION(1));
    }

    uv_os_fd_t handle = uv_get_osfhandle(fd);

    auto on_close = [](uv_fs_t* uv_req) {
      auto req = static_cast<FSRequest*>(uv_req->data);

      jsi::Runtime& rt = *req->rt;
      if (req->binding->_callInvoker->isValid()) {
        if (uv_req->result < 0) {
          JSI_MAKE_UV_ERROR(res, static_cast<int>(uv_req->result), req->binding->_uvBinding);
          req->callback->call(rt, res_err);
        } else {
          req->callback->call(rt, JSI_UNDEFINED);
        }
      }

      uv_fs_req_cleanup(&req->uv_req);
      delete req;
    };
    req->binding->_uvBinding->fds.erase(fd);
    int res = uv_fs_close(_uvBinding->uv_loop(), &req->uv_req, handle, hasCallback ? on_close : nullptr);

    if (hasCallback || res == 0) {
      if (!hasCallback) {
        delete req;
      }
      return JSI_UNDEFINED;
    }

    JSI_MAKE_UV_ERROR(exc, res, _uvBinding);
    JSI_CALL_THROWER(exc_err);

    delete req;

    return JSI_UNDEFINED;
  });

  JSI_HOSTOBJECT_METHOD("fs_read", 6, {
    auto fd = JSI_ARG_UINT32(0);

    JSI_ARGV_TYPEDARRAY(buffer, 1);

    auto offset = JSI_ARG_UINT32(2);
    auto length = JSI_ARG_UINT32(3);
    auto position = JSI_ARG_UINT32(4);

    bool hasCallback = argc > 5;

    auto req = new FSRequest;
    req->binding = this;
    req->rt = &rt;
    if (hasCallback) {
      req->callback = std::make_unique<jsi::Function>(JSI_ARG_FUNCTION(5));
    }
    req->buffer.base = reinterpret_cast<char*>(buffer_data) + offset;
    req->buffer.len = length;

    uv_os_fd_t handle = uv_get_osfhandle(fd);

    auto on_read = [](uv_fs_t* uv_req) {
      auto req = static_cast<FSRequest*>(uv_req->data);

      jsi::Runtime& rt = *req->rt;
      if (req->binding->_callInvoker->isValid()) {
        if (uv_req->result < 0) {
          JSI_MAKE_UV_ERROR(res, static_cast<int>(uv_req->result), req->binding->_uvBinding);
          req->callback->call(rt, res_err);
        } else {
          req->callback->call(rt, JSI_UNDEFINED, JSI_NUMBER(uv_req->result));
        }
      }

      uv_fs_req_cleanup(&req->uv_req);
      delete req;
    };

    int res = uv_fs_read(_uvBinding->uv_loop(),
               &req->uv_req,
               handle,
               &req->buffer,
               1,  // nbuffs
               position,
               hasCallback ? on_read : nullptr);
    if (hasCallback) {
      return JSI_UNDEFINED;
    }

    if (res >= 0) {
      delete req;
      return jsi::Value(res);
    }

    JSI_MAKE_UV_ERROR(exc, res, _uvBinding);
    JSI_CALL_THROWER(exc_err);

    delete req;

    return JSI_UNDEFINED;
  });

  JSI_HOSTOBJECT_METHOD("fs_write", 6, {
    auto fd = JSI_ARG_UINT32(0);

    JSI_ARGV_TYPEDARRAY(buffer, 1);

    auto offset = JSI_ARG_UINT32(2);
    auto length = JSI_ARG_UINT32(3);
    auto position = JSI_ARG_UINT32(4);

    bool hasCallback = argc > 5;

    auto req = new FSRequest;
    req->binding = this;
    req->rt = &rt;
    if (hasCallback) {
      req->callback = std::make_unique<jsi::Function>(JSI_ARG_FUNCTION(5));
    }
    req->buffer.base = reinterpret_cast<char*>(buffer_data) + offset;
    req->buffer.len = length;

    uv_os_fd_t handle = uv_get_osfhandle(fd);

    auto on_write = [](uv_fs_t* uv_req) {
      auto req = static_cast<FSRequest*>(uv_req->data);

      jsi::Runtime& rt = *req->rt;
      if (req->binding->_callInvoker->isValid()) {
        if (uv_req->result < 0) {
          JSI_MAKE_UV_ERROR(res, static_cast<int>(uv_req->result), req->binding->_uvBinding);
          req->callback->call(rt, res_err);
        } else {
          req->callback->call(rt, JSI_UNDEFINED, JSI_NUMBER(uv_req->result));
        }
      }

      uv_fs_req_cleanup(&req->uv_req);
      delete req;
    };
    int res = uv_fs_write(_uvBinding->uv_loop(),
                &req->uv_req,
                handle,
                &req->buffer,
                1,  // nbuffs
                position,
                hasCallback ? on_write : nullptr);
    if (hasCallback) {
      return JSI_UNDEFINED;
    }

    if (res >= 0) {
      delete req;
      return jsi::Value(res);
    }

    JSI_MAKE_UV_ERROR(exc, res, _uvBinding);
    JSI_CALL_THROWER(exc_err);

    delete req;

    return JSI_UNDEFINED;
  });

  JSI_HOSTOBJECT_METHOD("fs_mkdir", 3, {
    auto filename = JSI_ARG_STRING(0);
    auto mode = JSI_ARG_UINT32(1);

    bool hasCallback = argc > 2;

    auto req = new FSRequest;
    req->binding = this;
    req->rt = &rt;
    if (hasCallback) {
      req->callback = std::make_unique<jsi::Function>(JSI_ARG_FUNCTION(2));
    }

    auto on_mkdir = [](uv_fs_t* uv_req) {
      auto req = static_cast<FSRequest*>(uv_req->data);

      jsi::Runtime& rt = *req->rt;
      if (req->binding->_callInvoker->isValid()) {
        if (uv_req->result < 0) {
          JSI_MAKE_UV_ERROR(res, static_cast<int>(uv_req->result), req->binding->_uvBinding);
          req->callback->call(rt, res_err);
        } else {
          req->callback->call(rt, JSI_UNDEFINED);
        }
      }
      uv_fs_req_cleanup(&req->uv_req);
      delete req;
    };
    int res = uv_fs_mkdir(_uvBinding->uv_loop(),
                          &req->uv_req,
                          filename.data(),
                          mode,
                          hasCallback ? on_mkdir : nullptr);
    if (hasCallback) {
      return JSI_UNDEFINED;
    }

    if (res == 0) {
      delete req;
      return JSI_UNDEFINED;
    }

    JSI_MAKE_UV_ERROR(exc, res, _uvBinding);
    JSI_CALL_THROWER(exc_err);

    delete req;

    return JSI_UNDEFINED;
  });

  JSI_HOSTOBJECT_METHOD("fs_rmdir", 2, {
    auto filename = JSI_ARG_STRING(0);

    bool hasCallback = argc > 1;

    auto req = new FSRequest;
    req->binding = this;
    req->rt = &rt;
    if (hasCallback) {
      req->callback = std::make_unique<jsi::Function>(JSI_ARG_FUNCTION(1));
    }

    auto on_rmdir = [](uv_fs_t* uv_req) {
      auto req = static_cast<FSRequest*>(uv_req->data);

      jsi::Runtime& rt = *req->rt;
      if (req->binding->_callInvoker->isValid()) {
        if (uv_req->result < 0) {
          JSI_MAKE_UV_ERROR(res, static_cast<int>(uv_req->result), req->binding->_uvBinding);
          req->callback->call(rt, res_err);
        } else {
          req->callback->call(rt, JSI_UNDEFINED);
        }
      }

      uv_fs_req_cleanup(&req->uv_req);
      delete req;
    };
    int res = uv_fs_rmdir(_uvBinding->uv_loop(),
                &req->uv_req,
                filename.data(),
                hasCallback ? on_rmdir : nullptr);
    if (hasCallback || res == 0) {
      if (!hasCallback) {
        delete req;
      }
      return JSI_UNDEFINED;
    }

    JSI_MAKE_UV_ERROR(exc, res, _uvBinding);
    JSI_CALL_THROWER(exc_err);

    delete req;

    return JSI_UNDEFINED;
  });

  JSI_HOSTOBJECT_METHOD("fs_ftruncate", 3, {
    auto fd = JSI_ARG_UINT32(0);
    auto offset = JSI_ARG_UINT32(1);

    bool hasCallback = argc > 2;

    auto req = new FSRequest;
    req->binding = this;
    req->rt = &rt;
    if (hasCallback) {
      req->callback = std::make_unique<jsi::Function>(JSI_ARG_FUNCTION(2));
    }

    uv_os_fd_t handle = uv_get_osfhandle(fd);

    auto on_ftruncate = [](uv_fs_t* uv_req) {
      auto req = static_cast<FSRequest*>(uv_req->data);

      jsi::Runtime& rt = *req->rt;
      if (req->binding->_callInvoker->isValid()) {
        if (uv_req->result < 0) {
          JSI_MAKE_UV_ERROR(res, static_cast<int>(uv_req->result), req->binding->_uvBinding);
          req->callback->call(rt, res_err);
        } else {
          req->callback->call(rt, JSI_UNDEFINED);
        }
      }

      uv_fs_req_cleanup(&req->uv_req);
      delete req;
    };
    int res = uv_fs_ftruncate(_uvBinding->uv_loop(),
                    &req->uv_req,
                    handle,
                    offset,
                    hasCallback ? on_ftruncate : nullptr);
    if (hasCallback || res == 0) {
      if (!hasCallback) {
        delete req;
      }
      return JSI_UNDEFINED;
    }

    JSI_MAKE_UV_ERROR(exc, res, _uvBinding);
    JSI_CALL_THROWER(exc_err);

    delete req;

    return JSI_UNDEFINED;
  });

  JSI_HOSTOBJECT_METHOD("fs_stat", 2, {
    auto filename = JSI_ARG_STRING(0);

    bool hasCallback = argc > 1;

    auto req = new FSRequest;
    req->binding = this;
    req->rt = &rt;
    if (hasCallback) {
      req->callback = std::make_unique<jsi::Function>(JSI_ARG_FUNCTION(1));
    }

    auto on_stat = [](uv_fs_t* uv_req) {
      auto req = static_cast<FSRequest*>(uv_req->data);

      jsi::Runtime& rt = *req->rt;
      if (req->binding->_callInvoker->isValid()) {
        if (uv_req->result < 0) {
          JSI_MAKE_UV_ERROR(res, static_cast<int>(uv_req->result), req->binding->_uvBinding);
          req->callback->call(rt, res_err);
        } else {
          jsi::Object res = jsi::Object(rt);
          res.setProperty(rt, "size", JSI_NUMBER(uv_req->statbuf.st_size));
          res.setProperty(rt, "directory", JSI_BOOL((uv_req->statbuf.st_mode & S_IFMT) == S_IFDIR));
          req->callback->call(rt, JSI_UNDEFINED, res);
        }
      }

      uv_fs_req_cleanup(&req->uv_req);
      delete req;
    };

    int res = uv_fs_stat(_uvBinding->uv_loop(),
                         &req->uv_req,
                         filename.data(),
                         hasCallback ? on_stat : nullptr);
    if (hasCallback || res == 0) {
      jsi::Object obj = jsi::Object(rt);
      obj.setProperty(rt, "size", JSI_NUMBER(req->uv_req.statbuf.st_size));
      obj.setProperty(rt, "directory", JSI_BOOL((req->uv_req.statbuf.st_mode & S_IFMT) == S_IFDIR));

      if (!hasCallback) {
        delete req;
      }

      return jsi::Value(rt, obj);
    }

    JSI_MAKE_UV_ERROR(exc, res, _uvBinding);
    JSI_CALL_THROWER(exc_err);

    delete req;

    return JSI_UNDEFINED;
  });

  JSI_HOSTOBJECT_METHOD("fs_fstat", 2, {
    int fd = JSI_ARG_UINT32(0);

    bool hasCallback = argc > 1;

    auto req = new FSRequest;
    req->binding = this;
    req->rt = &rt;
    if (hasCallback) {
      req->callback = std::make_unique<jsi::Function>(JSI_ARG_FUNCTION(1));
    }

    uv_os_fd_t handle = uv_get_osfhandle(fd);

    auto on_fstat = [](uv_fs_t* uv_req) {
      auto req = static_cast<FSRequest*>(uv_req->data);

      jsi::Runtime& rt = *req->rt;
      if (req->binding->_callInvoker->isValid()) {
        if (uv_req->result < 0) {
          JSI_MAKE_UV_ERROR(res, static_cast<int>(uv_req->result), req->binding->_uvBinding);
          req->callback->call(rt, res_err);
        } else {
          jsi::Object res = jsi::Object(rt);
          res.setProperty(rt, "size", JSI_NUMBER(uv_req->statbuf.st_size));
          res.setProperty(rt, "directory", JSI_BOOL((uv_req->statbuf.st_mode & S_IFMT) == S_IFDIR));
          req->callback->call(rt, JSI_UNDEFINED, res);
        }
      }

      uv_fs_req_cleanup(&req->uv_req);
      delete req;
    };

    int res = uv_fs_fstat(_uvBinding->uv_loop(),
                &req->uv_req,
                handle,
                hasCallback ? on_fstat : nullptr);
    if (hasCallback || res == 0) {
      jsi::Object obj = jsi::Object(rt);
      obj.setProperty(rt, "size", JSI_NUMBER(req->uv_req.statbuf.st_size));
      obj.setProperty(rt, "directory", JSI_BOOL((req->uv_req.statbuf.st_mode & S_IFMT) == S_IFDIR));

      if (!hasCallback) {
        delete req;
      }
      return jsi::Value(rt, obj);
    }

    JSI_MAKE_UV_ERROR(exc, res, _uvBinding);
    JSI_CALL_THROWER(exc_err);

    delete req;

    return JSI_UNDEFINED;
  });

  JSI_HOSTOBJECT_METHOD("fs_unlink", 2, {
    auto filename = JSI_ARG_STRING(0);

    bool hasCallback = argc > 1;

    auto req = new FSRequest;
    req->binding = this;
    req->rt = &rt;
    if (hasCallback) {
      req->callback = std::make_unique<jsi::Function>(JSI_ARG_FUNCTION(1));
    }

    auto on_unlink = [](uv_fs_t* uv_req) {
      auto req = static_cast<FSRequest*>(uv_req->data);

      jsi::Runtime& rt = *req->rt;
      if (req->binding->_callInvoker->isValid()) {
        if (uv_req->result < 0) {
          JSI_MAKE_UV_ERROR(res, static_cast<int>(uv_req->result), req->binding->_uvBinding);
          req->callback->call(rt, res_err);
        } else {
          req->callback->call(rt, JSI_UNDEFINED);
        }
      }

      uv_fs_req_cleanup(&req->uv_req);
      delete req;
    };
    int res = uv_fs_unlink(_uvBinding->uv_loop(),
                 &req->uv_req,
                 filename.data(),
                 hasCallback ? on_unlink : nullptr);
    if (hasCallback || res == 0) {
      if (!hasCallback) {
        delete req;
      }
      return JSI_UNDEFINED;
    }

    JSI_MAKE_UV_ERROR(exc, res, _uvBinding);
    JSI_CALL_THROWER(exc_err);

    delete req;

    return JSI_UNDEFINED;
  });

  JSI_HOSTOBJECT_METHOD("fs_rename", 3, {
    auto oldPath = JSI_ARG_STRING(0);
    auto newPath = JSI_ARG_STRING(1);

    bool hasCallback = argc > 2;

    auto req = new FSRequest;
    req->binding = this;
    req->rt = &rt;
    if (hasCallback) {
      req->callback = std::make_unique<jsi::Function>(JSI_ARG_FUNCTION(2));
    }

    auto on_rename = [](uv_fs_t* uv_req) {
      auto req = static_cast<FSRequest*>(uv_req->data);

      jsi::Runtime& rt = *req->rt;
      if (req->binding->_callInvoker->isValid()) {
        if (uv_req->result < 0) {
          JSI_MAKE_UV_ERROR(res, static_cast<int>(uv_req->result), req->binding->_uvBinding);
          req->callback->call(rt, res_err);
        } else {
          req->callback->call(rt, JSI_UNDEFINED);
        }
      }

      uv_fs_req_cleanup(&req->uv_req);
      delete req;
    };
    int res = uv_fs_rename(_uvBinding->uv_loop(),
                           &req->uv_req,
                           oldPath.data(),
                           newPath.data(),
                           hasCallback ? on_rename : nullptr);
    if (hasCallback || res == 0) {
      if (!hasCallback) {
        delete req;
      }
      return JSI_UNDEFINED;
    }

    JSI_MAKE_UV_ERROR(exc, res, _uvBinding);
    JSI_CALL_THROWER(exc_err);

    delete req;

    return JSI_UNDEFINED;
  });

  // extension methods

  JSI_HOSTOBJECT_METHOD("fs_try_lock", 4, {
    auto fd = JSI_ARG_INT(0);
    auto offset = JSI_ARG_UINT32(1);
    auto length = JSI_ARG_UINT32(2);
    auto exclusive = JSI_ARG_UINT32(3);

    uv_os_fd_t handle = uv_get_osfhandle(fd);
    fs_ext_lock_type_t type = exclusive ? FS_EXT_WRLOCK : FS_EXT_RDLOCK;

    int errCode = 0;
#if defined(__APPLE__)
    // https://github.com/hypercore-skunkworks/fs-native-extensions/blob/dc4c4e0b8cc140e5f9a29ebaa24394aebea55b17/src/mac.c
    if (offset != 0 || length != 0) errCode = UV_EINVAL;
    else {
      errCode = flock(handle,  (type == FS_EXT_WRLOCK ? LOCK_EX : LOCK_SH) | LOCK_NB);
    }
#elif defined(__ANDROID_API__)
    // https://github.com/hypercore-skunkworks/fs-native-extensions/blob/dc4c4e0b8cc140e5f9a29ebaa24394aebea55b17/src/linux.c
    flock data;
    data.l_start = offset;
    data.l_len = length;
    data.l_pid = 0;
    data.l_type = type == FS_EXT_WRLOCK ? F_WRLCK : F_RDLCK;
    data.l_whence = SEEK_SET;
    errCode = fcntl(handle, F_OFD_SETLK, &data);
#endif

    if (errCode == -1) {
      errCode = uv_translate_sys_error(errno);
    }

    if (errCode == UV_EACCES || errCode == UV_EAGAIN || errCode == UV_EBUSY) {
      errCode = UV_EAGAIN;
    }

    if (errCode == 0) {
      return JSI_UNDEFINED;
    }

    JSI_MAKE_UV_ERROR(ret, errCode, _uvBinding);
    return jsi::Value(rt, ret_err);
  });

  JSI_HOSTOBJECT_METHOD("fs_trim", 3, {
    auto fd = JSI_ARG_INT(0);
    auto offset = JSI_ARG_UINT32(1);
    auto length = JSI_ARG_UINT32(2);

    uv_os_fd_t handle = uv_get_osfhandle(fd);

    int errCode = 0;
#if defined(__APPLE__)
    // https://github.com/holepunchto/fs-native-extensions/blob/6b337247872d493985a2087d5c013fd984e70e6f/src/mac.c
    struct stat st;

    errCode = fstat(handle, &st);

    if (errCode == -1) {
      errCode = uv_translate_sys_error(errno);
      JSI_MAKE_UV_ERROR(ret, errCode, _uvBinding);
      return jsi::Value(rt, ret_err);
    }

    size_t align = st.st_blksize;
    size_t aligned_offset = (offset + (align - 1)) & -align;

    if (aligned_offset != offset) {
      size_t b = aligned_offset - offset;
      size_t zero_length = length < b ? length : b;

      void *buf = calloc(1, zero_length);

      ssize_t res = pwrite(fd, buf, zero_length, offset);

      free(buf);

      if (res == -1) {
        errCode = uv_translate_sys_error(errno);
        JSI_MAKE_UV_ERROR(ret, errCode, _uvBinding);
        return jsi::Value(rt, ret_err);
      }

      offset += zero_length;
      length -= zero_length;
    }

    ssize_t aligned_length = length & -align;

    if (aligned_length >= align) {
      struct fpunchhole data = fpunchhole({
                                              .fp_flags = 0,
                                              .reserved = 0,
                                              .fp_offset = offset,
                                              .fp_length = aligned_length,
                                          });

      int res = fcntl(fd, F_PUNCHHOLE, &data);

      if (res == -1) {
        errCode = uv_translate_sys_error(errno);
        JSI_MAKE_UV_ERROR(ret, errCode, _uvBinding);
        return jsi::Value(rt, ret_err);
      }

      offset += aligned_length;
      length -= aligned_length;
    }

    if (length > 0) {
      void *buf = calloc(1, length);

      ssize_t res = pwrite(fd, buf, length, offset);

      free(buf);

      if (res == -1) {
        errCode = uv_translate_sys_error(errno);
        JSI_MAKE_UV_ERROR(ret, errCode, _uvBinding);
        return jsi::Value(rt, ret_err);
      }
    }
#elif defined(__ANDROID_API__)
    // https://github.com/holepunchto/fs-native-extensions/blob/6b337247872d493985a2087d5c013fd984e70e6f/src/linux.c
    errCode = fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, offset, length);
#endif

    if (errCode == -1) {
      errCode = uv_translate_sys_error(errno);
    }

    if (errCode == 0) {
      return JSI_UNDEFINED;
    }

    JSI_MAKE_UV_ERROR(ret, errCode, _uvBinding);
    return jsi::Value(rt, ret_err);
  });

  return JSI_UNDEFINED;
}

void holepunch::fs::install(jsi::Runtime &rt, std::shared_ptr<CallInvokerGetter> callInvoker) {
  auto obj = std::make_shared<FSBinding>(callInvoker);
  jsi::Object hostObject = jsi::Object::createFromHostObject(rt, obj);
  rt.global().setProperty(rt, "__hlp_fs", std::move(hostObject));
}
