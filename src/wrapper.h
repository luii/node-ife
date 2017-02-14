/* Copyright (c) 2013, OmniTI Computer Consulting, Inc. All rights reserved. */

#include <node.h>
#include <node_object_wrap.h>
#include <v8.h>
#include <nan.h>

#include <sys/types.h>
#include <sys/mman.h>

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef __APPLE__
#include <stdlib.h>
#include <malloc.h>
#endif

using namespace Nan;

class IFE : public Nan::ObjectWrap {

public:


  static NAN_MODULE_INIT(Initialize);

private:
  IFE();  // constructor
  ~IFE(); // destructor

  static Persistent<v8::Function> constructor;
  static NAN_METHOD(New);      // create a new instance of `ife`
  static NAN_METHOD(list);     // list all interfaces
  static NAN_METHOD(up);       // activate a interface
  static NAN_METHOD(down);     // deactivate a interface
  static NAN_METHOD(gratarp);  // ?
  static NAN_METHOD(arpcache); // get the ARP Cache
};
