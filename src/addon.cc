#include <nan.h>
#include "IFEheader.h"

void InitAll(v8::Local<v8::Object> exports) {
  IFE::Initialize(exports);
}

NODE_MODULE(IFEBinding, InitAll)