/* Copyright (c) 2013, OmniTI Computer Consulting, Inc. All rights reserved. */

#ifndef IFE_WRAPPER_H_
#define IFE_WRAPPER_H_

#define DECLARE_NAPI_METHOD(name, func) \
  { name, 0, func, 0, 0, 0, napi_default, 0 }

#include <node_api.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <arpa/inet.h>

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>

class IFE {
  public:
    static napi_value init (napi_env env, napi_value exports);
    static void       Destructor(napi_env, void *nativeObject, void *finalize_hint);

  private:
    explicit IFE();
    ~IFE();

    napi_env env_;
    napi_ref wrapper_;

    static napi_ref   constructor;

    static napi_value New (napi_env env, napi_callback_info info);
    static napi_value list (napi_env env, napi_callback_info info);
    static napi_value up (napi_env env, napi_callback_info info);
    static napi_value down (napi_env env, napi_callback_info info);
    static napi_value gratarp (napi_env env, napi_callback_info info);
    static napi_value arpcache (napi_env env, napi_callback_info info);
};

#endif // !IFE_WRAPPER_H_
