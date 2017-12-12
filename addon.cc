#include "IFEheader.h"

napi_value Init(napi_env env, napi_value exports) {
  return IFE::init(env, exports);
}

NAPI_MODULE(addon, Init)