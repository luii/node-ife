/* Copyright (c) 2013, OmniTI Computer Consulting, Inc. All rights reserved. */

#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif
#include <v8.h>

#include <node.h>
#include <stdio.h>
#include "ife.h"

#include "wrapper.h"

using namespace Nan;

Persistent<v8::Function> IFE::constructor;

IFE::IFE() : Nan::ObjectWrap() {
}

IFE::~IFE() {
}

NAN_MODULE_INIT(IFE::Initialize) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(IFE::New);

  tpl->SetClassName(Nan::New<v8::String>("IFE").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  SetPrototypeMethod(tpl, "list", IFE::list);
  SetPrototypeMethod(tpl, "up", IFE::up);
  SetPrototypeMethod(tpl, "down", IFE::down);
  SetPrototypeMethod(tpl, "gratarp", IFE::gratarp);
  SetPrototypeMethod(tpl, "arpcache", IFE::arpcache);

  constructor.Reset(tpl->GetFunction());

  Set(target,
      Nan::New<v8::String>("IFE").ToLocalChecked(),
      tpl->GetFunction());
}


NAN_METHOD(IFE::New) {
  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Class constructors cannot be invoked without 'new'");
  }

  IFE *p = new IFE();
  p->Wrap(info.This());
  return info.GetReturnValue().Set(info.This());
}

NAN_METHOD(IFE::arpcache) {
  int i, cnt;
  arp_entry *entries;

  cnt = sample_arp_cache(&entries);
  if(cnt < 0) return info.GetReturnValue().SetUndefined();

  v8::Local<v8::Object> obj = Nan::New<v8::Object>();

  for(i = 0; i < cnt; i++) {
    char ipstr[32], mac[20];
    unsigned char *m;
    if(inet_ntop(AF_INET, &entries[i]._ipaddr, ipstr, sizeof(ipstr)) != NULL) {
      m = entries[i].mac;
      snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
              m[0], m[1], m[2], m[3], m[4], m[5]);

      Nan::Set(obj,
              Nan::To<v8::Value>(ipstr.c_str()),
              Nan::To<v8::Value>(mac));
    }
  }
  return info.GetReturnValue().Set(obj);
}

NAN_METHOD(IFE::list) {
  int cnt, i;
  struct interface *ifs;

  ifs = (struct interface *)malloc(sizeof(*ifs) * 1024);
  cnt = if_list_ips(ifs, 1024);
  v8::Handle<v8::Array> obj = Nan::New<v8::Array>(cnt);

  for(i=0; i<cnt; i++) {
    char ipstr[64];
    v8::Handle<v8::Object> iface = Nan::New<v8::Object>();
    Nan::Set(iface,
             Nan::New("name").ToLocalChecked(),
             Nan::New(ifs[i].ifname).ToLocalChecked());

#define SET_IPV4(attr, name) do { \
  inet_ntop(AF_INET, &ifs[i].attr, ipstr, sizeof(ipstr)); \
  Nan::Set(iface, \
           Nan::New(name).ToLocalChecked(), \
           Nan::New(ipstr).ToLocalChecked()); \
} while(0)

#define SET_IPV6(attr, name) do { \
  inet_ntop(AF_INET6, &ifs[i].attr, ipstr, sizeof(ipstr)); \
  Nan::Set(iface, \
           Nan::New(name).ToLocalChecked(), \
           Nan::New(ipstr).ToLocalChecked()); \
} while(0)
      if(ifs[i].family == AF_INET6) {
        int len;
        SET_IPV6(_ipaddr._ip6addr, "ip");
        len = set_prefix_from_netmask6(&ifs[i]._netmask._netmask6);
        Nan::Set(iface,
                 Nan::New("prefixlen").ToLocalChecked(),
                 Nan::New(len).ToLocalChecked());
      } else {
        SET_IPV4(_ipaddr._ip4addr, "ip");
        SET_IPV4(_bcast._bcast4, "broadcast");
        SET_IPV4(_netmask._netmask4, "netmask");
      }
      snprintf(ipstr, sizeof(ipstr), "%02x:%02x:%02x:%02x:%02x:%02x",
               ifs[i].mac[0], ifs[i].mac[1], ifs[i].mac[2],
               ifs[i].mac[3], ifs[i].mac[4], ifs[i].mac[5]);

      Nan::Set(iface, Nan::New<v8::Value>("mac"), Nan::New<v8::Value>(ipstr));
      Nan::Set(obj, Nan::New<v8::Value>(i), iface)
    }
    free(ifs);
    return info.GetReturnValue().Set(obj);
  }

  NAN_METHOD(IFE::up) {
    struct interface iface;

    v8::Handle<v8::Object> obj = info.Holder();
    IFE *ife = Nan::ObjectWrap::Unwrap<IFE>(obj);

    memset((void *)&iface, 0, sizeof(iface));
    v8::Local<v8::Object> o = info[0]->ToObject();
    v8::Local<v8::Value> vname = Nan::Get(o, Nan::New<v8::Value>("name"));
    if(vname->IsUndefined()) {
      Nan::ThrowTypeError("name: undefined");
      return info.GetReturnValue().Set(Nan::False());
    }
    v8::Local<v8::String> name = vname->ToString();
    Nan::Utf8String ifname(name);
    strncpy(iface.ifname, *(ifname), IFNAMSIZ);

#define GET_IPV4(attr, name) do { \
    v8::Local<v8::Value> ovip = Nan::Get(o, Nan::New<v8::Value>(name)); \
    if(ovip->IsUndefined()) { \
      Nan::ThrowTypeError("name: undefined"); \
      return info.GetReturnValue().Set(Nan::False()); \
    } \
    v8::Local<v8::String> addr = ovip->ToString(); \
    Nan::Utf8String val(addr); \
    if(inet_pton(AF_INET, *(val), &iface.attr) != 1) { \
      Nan::ThrowTypeError(Nan::New<v8::String>(*val)); \
      return info.GetReturnValue().Set(Nan::False()); \
    } \
} while(0)

    v8::Local<v8::Value> vip = Nan::Get(o, Nan::New<v8::Value>("ip"));
    if(vip->IsUndefined()) {
      Nan::ThrowTypeError("ip: undefined");
      return info.GetReturnValue().Set(Nan::False());
    }
    v8::Local<v8::String> ip = vip->ToString();
    Nan::Utf8String ipval(ip);
    if(inet_pton(AF_INET, *(ipval), &iface._ipaddr) == 1) {
      GET_IPV4(_bcast._bcast4, "broadcast");
      GET_IPV4(_netmask._netmask4, "netmask");
      GET_IPV4(_network._network4, "network");
      iface.family = AF_INET;
    }
    else if(inet_pton(AF_INET6, *(ipval), &iface._ipaddr) == 1) {
      v8::Local<v8::Value> pname = Nan::Get(o, Nan::New("prefixlen"));
      v8::Local<v8::Integer> plen = pname->ToInteger();
      set_netmask6_from_prefix(&iface._netmask._netmask6, plen->Value());
      iface.family = AF_INET6;
    }
    else {
      Nan::ThrowTypeError(Nan::New<v8::String>(*ipval));
      return info.GetReturnValue().Set(Nan::False());
    }
    if(if_up(&iface)) {
      v8::Local<v8::Value> vChr[2];
      vChr[0] = Nan::New<v8::String>("error");
      vChr[1] = Nan::New<v8::String>(if_error());
      return info.GetReturnValue().Set(Nan::False());
    }
    return info.GetReturnValue().Set(Nan::True());
  }

  NAN_METHOD(IFE::down) {
    struct interface iface;

    v8::Handle<v8::Object> obj = info.Holder();
    IFE *ife = Nan::ObjectWrap::Unwrap<IFE>(obj);
    memset((void *)&iface, 0, sizeof(iface));

    if(info[0]->IsUndefined()) {
      Nan::ThrowTypeError("argument undefined"); \
      return info.GetReturnValue().SetUndefined();
    }
    v8::Local<v8::String> ip = info[0]->ToString();
    Nan::Utf8String val(ip);
    if(inet_pton(AF_INET, *(val), &iface._ipaddr) == 1) {
      iface.family = AF_INET;
    }
    else if(inet_pton(AF_INET6, *(val), &iface._ipaddr._ip6addr) == 1) {
      iface.family = AF_INET6;
    }
    else {
      Nan::ThrowTypeError(Nan::New<v8::String>(*val));
      return info.GetReturnValue().SetUndefined();
    }
    if(info.Length() == 2) {
      Nan::Utf8String val(info[1]);
      if(*val && strlen(*val) > 0
        && strcmp(*val, "preplumbed")==0 ) {
          iface.state = ETH_DOWN_STATE;
      }
    }

    if(if_down(&iface)) {
      v8::Local<v8::Value> vChr[2];
      vChr[0] = Nan::New<v8::String>("error");
      vChr[1] = Nan::New<v8::String>(if_error());
      return info.GetReturnValue().Set(Nan::False());
    }
    return info.GetReturnValue().Set(Nan::True());
  }

  NAN_METHOD(IFE::gratarp) {
    const char *dev;
    uint32_t my_ip, r_ip;
    int count = 1, do_ping = 1;
    unsigned char r_mac[ETH_ALEN];
    int good_mac = 0;

    v8::Local<v8::Object> o = info[0]->ToObject();
    v8::Local<v8::Value> vname = Nan::Get(o, Nan::New<v8::Value>("name"));

    if(vname->IsUndefined()) {
      Nan::ThrowTypeError("name: undefined");
      return info.GetReturnValue().Set(Nan::False());
    }
    v8::Local<v8::String> name = vname->ToString();
    Nan::Utf8String ifname(name);
    dev = *ifname;

    v8::Local<v8::Value> vmyip = Nan::Get(o, Nan::New<v8::Value>("local_ip"));
    if(vmyip->IsUndefined()) {
      Nan::ThrowTypeError("local_ip: undefined");
      return info.GetReturnValue().Set(Nan::False());
    }
    v8::Local<v8::String> v8_myip = vmyip->ToString();
    Nan::Utf8String val_myip(v8_myip);
    if(inet_pton(AF_INET, *(val_myip), &my_ip) != 1) {
      Nan::ThrowTypeError(Nan::New<v8::String>(*val_myip));
      return info.GetReturnValue().Set(Nan::False());
    }

    v8::Local<v8::Value> vrip = Nan::Get(o, Nan::New<v8::Value>("remote_ip"));
    if(vrip->IsUndefined()) {
      Nan::ThrowTypeError("remote_ip: undefined");
      return info.GetReturnValue().Set(Nan::False());
    }
    v8::Local<v8::String> v8_rip = vrip->ToString();
    Nan::Utf8String val_rip(v8_rip);
    if(inet_pton(AF_INET, *(val_rip), &r_ip) != 1) {
      Nan::ThrowError(Nan::New<v8::String>(*val_rip));
      return info.GetReturnValue().Set(Nan::False());
    }

    v8::Local<v8::Value> vrmac = Nan::Get(o, Nan::New<v8::Value>("remote_mac"));
    if(!vrmac->IsUndefined()) {
      int i;
      v8::Local<v8::String> v8_mac = vrmac->ToString();
      Nan::Utf8String val_mac(v8_mac);
      if(strlen(*val_mac) == 17 &&
         (*(val_mac))[2] == ':' && (*(val_mac))[5] == ':' && (*(val_mac))[8] == ':' &&
         (*(val_mac))[11] == ':' && (*(val_mac))[14] == ':') {
        for(i=0;i<6;i++) {
          int v;
          if(sscanf((*(val_mac)) + i*3, "%02x", &v) == 1 && v >= 0 && v <= 255)
            r_mac[i] = (unsigned char) (v & 0xff);
          else
            break;
        }
        if(i == 6) good_mac = 1;
      }
      if(!good_mac) {
        Nan::ThrowError("bad mac address");
        return info.GetReturnValue().SetUndefined();
      }
    }

    if(info.Length() > 1) {
      if(!info[1]->IsNumber()) {
        Nan::ThrowTypeError("Second argument must be a number");
        return info.GetReturnValue().SetUndefined();
      }
      count = info[1]->NumberValue();
    }

    if(info.Length() > 2) {
      if(!info[2]->IsBoolean()) {
        Nan::ThrowTypeError("Third argument must be a boolean");
        return info.GetReturnValue().SetUndefined();
      }
      do_ping = info[2]->BooleanValue();
      if(do_ping && !good_mac) {
        Nan::ThrowError("Can't do ping without remote mac");
        return info.GetReturnValue().SetUndefined();
      }
    }

    count = if_send_spoof_request(dev, my_ip, r_ip, good_mac ? r_mac : NULL, count, do_ping);
    return info.GetReturnValue().Set(Nan::New<v8::Integer>(count));
  }