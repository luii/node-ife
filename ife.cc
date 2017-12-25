/* Copyright (c) 2013, OmniTI Computer Consulting, Inc. All rights reserved. */

#ifndef BUILDING_NODE_EXTENSION
#define BUILDING_NODE_EXTENSION
#endif

#include "IFEheader.h"
#include <stdio.h>
#include "ife.h"


  // constructor
  IFE::IFE() : env_(nullptr), wrapper_(nullptr) {  }

  // destructor
  IFE::~IFE() { napi_delete_reference(env_, wrapper_); }

  /**
   * Emits a symbol in the JS land
   */
  // void IFE::emit(Local<Value> args[], int nargs) {
  //   Local<Value> emit_v = handle_->Get(emit_symbol);
  //   if (emit_v->IsFunction())
  //   {
  //     Local<Function> emit = Local<Function>::Cast(emit_v);
  //     TryCatch tc;
  //     emit->Call(handle_, nargs, args);
  //     if (tc.HasCaught())
  //     {
  //       // No nothing here.
  //     }
  //   }
  // }

  napi_ref IFE::constructor;

  /**
   * Creates a new function template sets the prototype methods and
   * makes a sorta class out of it
   */
  napi_value IFE::init (napi_env env, napi_value exports) {
    napi_status status;

    napi_property_descriptor props[] = {
      DECLARE_NAPI_METHOD("list", list),
      DECLARE_NAPI_METHOD("arpcache", arpcache),
      DECLARE_NAPI_METHOD("up", up),
      DECLARE_NAPI_METHOD("down", down),
      DECLARE_NAPI_METHOD("gratarp", gratarp)
    };

    napi_value ctor;

    status = napi_define_class(env, "IFE", 3, New, nullptr, 5, props, &ctor);
    assert(status == napi_ok);

    status = napi_create_reference(env, ctor, 1, &constructor);
    assert(status == napi_ok);

    status = napi_set_named_property(env, exports, "IFE", ctor);
    assert(status == napi_ok);

    return exports;
  }

  void IFE::Destructor (napi_env, void *nativeObject, void* /*finalize_hint*/) {
    reinterpret_cast<IFE*>(nativeObject)->~IFE();
  }

  /**
   * This method is invoked if you create a new instance with the new keyword
   * like: `let test = new IFE()` in JS
   */
  napi_value IFE::New (napi_env env, napi_callback_info info) {
    napi_status status;
    napi_value  target;

    status = napi_get_new_target(env, info, &target);
    assert(status == napi_ok);

    bool is_constructor = target != nullptr;

    if (is_constructor) {
      size_t argc = 0;
      napi_value This;

      status = napi_get_cb_info(env, info, &argc, nullptr, &This, nullptr);
      assert(status == napi_ok);

      IFE *ife = new IFE();
      ife->env_ = env;

      status = napi_wrap(env,
                         This,
                         reinterpret_cast<void*>(ife),
                         IFE::Destructor,
                         nullptr,
                         &ife->wrapper_);


      assert(status == napi_ok);

      return This;
    } else {
      napi_value undefined_obj;

      status = napi_get_undefined(env, &undefined_obj);
      assert(status == napi_ok);

      return undefined_obj;
    }
  }

  /**
   * List all the broadcast-capable interfaces on the server
   * Example: { '192.168.178.20': '00:00:00' }
   */
  napi_value IFE::arpcache (napi_env env, napi_callback_info info) {
    napi_status status;
    napi_value  obj;
    napi_value  mac;

    int i, cnt;
    arp_entry *entries;

    cnt = sample_arp_cache(&entries);

    if (cnt < 0) {
      napi_value undefined_obj;

      status = napi_get_undefined(env, &undefined_obj);
      assert(status == napi_ok);

      return undefined_obj;
    }


    for (i = 0; i < cnt; i++) {

      char ipStr[32], macStr[20];
      unsigned char *m;

      if (inet_ntop(AF_INET, &entries[i].ipaddr, ipStr, sizeof(ipStr)) != NULL) {
        m = entries[i].mac;

        snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
                m[0], m[1], m[2], m[3], m[4], m[5]);

        status = napi_create_object(env, &obj);
        assert(status == napi_ok);

        status = napi_create_string_utf8(env, macStr, sizeof(mac), &mac);
        assert(status == napi_ok);

        status = napi_set_named_property(env, obj, ipStr, mac);
        assert(status == napi_ok);

        return obj;
      }
    }
  }

  /**
   * List all the available Interfaces
   */
  napi_value IFE::list (napi_env env, napi_callback_info info) {
    napi_status status;
    napi_value  iface;
    napi_value  obj;

    struct interface *ifs;
    int    i, cnt;

    ifs    = (struct interface *)malloc(sizeof(*ifs) * 1024);
    cnt    = if_list_ips(ifs, 1024);
    status = napi_create_array_with_length(env, static_cast<size_t>(cnt), &obj);
    assert(status == napi_ok);

    for (i = 0; i < cnt; i++) {
      char ipStr[64];
      napi_value ifname;

      status = napi_create_object(env, &iface);
      assert(status == napi_ok);

      status = napi_create_string_utf8(env, ifs[i].ifname, NAPI_AUTO_LENGTH, &ifname);
      assert(status == napi_ok);

      status = napi_set_named_property(env, iface, "name", ifname);
      assert(status == napi_ok);

      #define SET_IPV4(attr, name)                                             \
        do {                                                                   \
          napi_value ip;                                                       \
          inet_ntop(AF_INET, &ifs[i].attr, ipStr, sizeof(ipStr));              \
          status = napi_create_string_utf8(env, ipStr, NAPI_AUTO_LENGTH, &ip); \
          assert(status == napi_ok);                                           \
          status = napi_set_named_property(env, iface, name, ip);              \
          assert(status == napi_ok);                                           \
        } while (0)                                                            \

      #define SET_IPV6(attr, name)                                             \
        do {                                                                   \
          napi_value ip;                                                       \
          inet_ntop(AF_INET6, &ifs[i].attr, ipStr, sizeof(ipStr));             \
          status = napi_create_string_utf8(env, ipStr, NAPI_AUTO_LENGTH, &ip); \
          assert(status == napi_ok);                                           \
          status = napi_set_named_property(env, iface, name, ip);              \
          assert(status == napi_ok);                                           \
        } while (0)                                                            \

      if (ifs[i].family == AF_INET6) {
        int        len;
        napi_value nLen;

        SET_IPV6(ip6addr, "ip");
        len    = set_prefix_from_netmask6(&ifs[i].netmask6);

        status = napi_create_int32(env, static_cast<int32_t>(len), &nLen);
        assert(status == napi_ok);

        status = napi_set_named_property(env, iface, "prefixlen", nLen);
        assert(status == napi_ok);
      } else {
        SET_IPV4(ipaddr,  "ip");
        SET_IPV4(bcast,   "broadcast");
        SET_IPV4(netmask, "netmask");
      }
      
      snprintf(ipStr, sizeof(ipStr), "%02x:%02x:%02x:%02x:%02x:%02x",
               ifs[i].mac[0], ifs[i].mac[1], ifs[i].mac[2],
               ifs[i].mac[3], ifs[i].mac[4], ifs[i].mac[5]);

      napi_value nIp;

      status = napi_create_string_utf8(env, ipStr, NAPI_AUTO_LENGTH, &nIp);
      assert(status == napi_ok);

      status = napi_set_named_property(env, iface, "mac", nIp);
      assert(status == napi_ok);

      status = napi_set_element(env, obj, i, iface);
    }

    free(ifs);
    return obj;
  }

  /**
   * Enables the specified interface(s)
   */
  napi_value IFE::up (napi_env env, napi_callback_info info) {
    napi_status status;   // the status
    napi_value  args[1];  // argument value
    napi_value  This;     // the this in js
    size_t      argc = 1; // arg count

    struct interface iface;

    status = napi_get_cb_info(env, info, &argc, args, &This, nullptr); // get the arg from the js function
    assert(status == napi_ok);

    if (argc > 2) {
      napi_throw_type_error(env, nullptr, "Invalid number of Args! Expected 1 Argument!");
      return nullptr;
    }

    napi_valuetype valueType;
    status = napi_typeof(env, args[0], &valueType);
    assert(status == napi_ok);

    if (valueType != napi_object) {
      napi_throw_type_error(env, nullptr, "Invalid argument Type! Expected to be a Object!");
      return nullptr;
    }

    memset((void *)&iface, 0, sizeof(iface));

    napi_valuetype nNameType;
    napi_value     nName;
    size_t         nameCopiedBytes;

    status = napi_get_named_property(env, args[0], "name", &nName);
    assert(status == napi_ok);

    status = napi_typeof(env, nName, &nNameType);
    assert(status == napi_ok);

    if (nNameType == napi_undefined) {
      napi_throw_type_error(env, nullptr, "Name is undefined!");
      return nullptr;
    }

    status = napi_get_value_string_utf8(env, nName, NULL, NULL, &nameCopiedBytes);
    assert(status == napi_ok);

    char           name[nameCopiedBytes + 1];
    status = napi_get_value_string_utf8(env, nName, name, sizeof(name), nullptr);
    assert(status == napi_ok);

    strncpy(iface.ifname, name, IFNAMSIZ);

    #define GET_IPV4(attr, name)                                               \
    do {                                                                       \
      napi_value     nOvip;                                                    \
      napi_valuetype nOvipType;                                                \
      char           addr[16] = {0};                                           \
      size_t         addrCopiedBytes;                                          \
                                                                               \
      status = napi_get_named_property(env, args[0], name, &nOvip);            \
      assert(status == napi_ok);                                               \
                                                                               \
      status = napi_typeof(env, nOvip, &nOvipType);                            \
      assert(status == napi_ok);                                               \
                                                                               \
      if (nOvipType == napi_undefined) {                                       \
        napi_throw_type_error(env, nullptr, name);              \
        return nullptr;                                                        \
      }                                                                        \
                                                                               \
      status = napi_get_value_string_utf8(env, nOvip, addr, sizeof(addr), &addrCopiedBytes); \
      assert(status == napi_ok);                                               \
                                                                               \
      if (inet_pton(AF_INET, addr, &iface.attr) != 1) {           \
        napi_throw_type_error(env, nullptr, addr); \
        return nullptr;                                                        \
      }                                                                        \
    } while (0)                                                                \

    napi_value     nIP;
    napi_valuetype nIPType;
    char           ip[16] = {0};
    size_t         ipSize = 16;
    size_t         ipCopiedBytes;

    status = napi_get_named_property(env, args[0], "ip", &nIP);
    assert(status == napi_ok);

    status = napi_typeof(env, nIP, &nIPType);
    assert(status == napi_ok);

    if (nIPType == napi_undefined) {
      napi_throw_type_error(env, nullptr, "IP: undefined");
      return nullptr;
    }

    status = napi_get_value_string_utf8(env, nIP, ip, ipSize, &ipCopiedBytes);
    assert(status == napi_ok);

    if (inet_pton(AF_INET, ip, &iface.ipaddr) == 1) {
      GET_IPV4(bcast, "broadcast");
      GET_IPV4(netmask, "netmask");
      GET_IPV4(network, "network");
      iface.family = AF_INET;
    } else if (inet_pton(AF_INET6, ip, &iface.ip6addr)) {
      napi_value pname;
      int32_t    plen;

      status = napi_get_named_property(env, args[0], "prefixlen", &pname);
      assert(status == napi_ok);

      status = napi_get_value_int32(env, pname, &plen);
      assert(status == napi_ok);

      set_netmask6_from_prefix(&iface.netmask6, (int)plen);
      iface.family = AF_INET6;
    } else {
      napi_throw_type_error(env, nullptr, (const char*)ip);
      return nullptr;
    }

    int32_t    success;
    napi_value nSuccess;

    success = 1;

    if (if_up(&iface)) {
      char *error = if_error();

      napi_throw_error(env, nullptr, (const char*) error);
      return nullptr;
    }

    status = napi_create_int32(env, success, &nSuccess);
    assert(status == napi_ok);

    status = napi_coerce_to_bool(env, nSuccess, &nSuccess);
    assert(status == napi_ok);

    return nSuccess;
  }

  napi_value IFE::down(napi_env env, napi_callback_info info) {
    napi_status status;
    struct interface iface;

    memset((void *)&iface, 0, sizeof(iface));

    size_t         argc = 1;
    napi_value     args[argc];
    napi_value     This;
    napi_valuetype argType;

    status = napi_get_cb_info(env, info, &argc, args, &This, nullptr);
    assert(status == napi_ok);

    status = napi_typeof(env, args[0], &argType);
    assert(status == napi_ok);

    if (argType == napi_undefined) {
      napi_throw_type_error(env, nullptr, "Argument must be a valid ip address");
      return nullptr;
    }

    size_t ipBytesCopied;

    status = napi_get_value_string_utf8(env, args[0], NULL, NULL, &ipBytesCopied);
    assert(status == napi_ok);

    char ip[ipBytesCopied + 1];
    status = napi_get_value_string_utf8(env, args[0], ip, sizeof(ip), nullptr);
    assert(status == napi_ok);

    if (inet_pton(AF_INET, ip, &iface.ipaddr) == 1) {
      iface.family = AF_INET;
    } else if (inet_pton(AF_INET6, ip, &iface.ip6addr) == 1) {
      iface.family = AF_INET6;
    } else {
      napi_throw_type_error(env, nullptr, (const char*)ip);
      return nullptr;
    }

    if (argc == 2) {
      char t[10] = {0};
      size_t size;

      status = napi_get_value_string_utf8(env, args[1], t, 10, &size);
      assert(status == napi_ok);

      if (t && strlen((const char*)t) > 0 && strcmp((const char*)t, "preplumbed") == 0) {
        iface.state == ETH_DOWN_STATE;
      }
    }

    int32_t    success;
    napi_value nSuccess;

    if (success = if_down(&iface)) {
      char *error = if_error();

      napi_throw_error(env, nullptr, (const char*) error);
      return nullptr;
    }

    status = napi_create_int32(env, success, &nSuccess);
    assert(status == napi_ok);

    status = napi_coerce_to_bool(env, nSuccess, &nSuccess);
    assert(status == napi_ok);

    return nSuccess;
  }

  napi_value IFE::gratarp(napi_env env, napi_callback_info info) {
    napi_status status;

    size_t     argc = 3;
    napi_value args[argc - 1];
    napi_value This;

    status = napi_get_cb_info(env, info, &argc, args, &This, nullptr);
    assert(status == napi_ok);

    napi_valuetype valueType;
    status = napi_typeof(env, args[0], &valueType);
    assert(status == napi_ok);

    if (valueType != napi_object) {
      napi_throw_type_error(env, nullptr, "Argument must be a Object!");
      return nullptr;
    }

    // get the properties out of args[0] in js terms:
    // args[0] = { name: 'some', local_ip: '', remote_ip: '', remote_mac: '' }

    napi_value     name;
    napi_valuetype nameValueType;
    char           dev[16] = {0};
    size_t         nameSize = 16;
    size_t         nameCopiedBytes;

    napi_value     local_ip;
    napi_valuetype local_ipValueType;
    char           lIp[16] = {0}; // local ip
    uint32_t       my_ip;
    size_t         my_ipSize = 16;
    size_t         my_ipCopiedBytes;


    napi_value remote_ip;
    napi_valuetype remote_ipValueType;
    char           rIp[16] = {0}; // remote ip
    uint32_t       r_ip;
    size_t         r_ipSize = 16;
    size_t         r_ipCopiedBytes;

    napi_value remote_mac;
    napi_valuetype remote_macValueType;
    char           rMac[17] = {0};
    size_t         r_macSize = 17;
    size_t         r_macCopiedBytes;

    unsigned char  r_mac[ETH_ALEN];
    int            good_mac = 0;

    int32_t count;  // args[1] = count; how many
    bool    doPing; // args[2] = bool;  do ping

    status = napi_get_named_property(env, args[0], "name", &name);
    assert(status == napi_ok);
    
    status = napi_typeof(env, name, &nameValueType);
    assert(status == napi_ok);

    if (nameValueType == napi_undefined) {
      napi_throw_type_error(env, nullptr, "Name must be defined");
      return nullptr;  
    }

    status = napi_get_value_string_utf8(env, name, dev, nameSize, &nameCopiedBytes);
    assert(status == napi_ok);

    status = napi_get_named_property(env, args[0], "local_ip", &local_ip);
    assert(status == napi_ok);

    status = napi_typeof(env, local_ip, &local_ipValueType);
    assert(status == napi_ok);

    if (local_ipValueType == napi_undefined) {
      napi_throw_type_error(env, nullptr, "local_ip must be defined");
      return nullptr;  
    }

    status = napi_get_value_string_utf8(env, local_ip, lIp, my_ipSize, &my_ipCopiedBytes);
    assert(status == napi_ok);

    if (inet_pton(AF_INET, (const char*)lIp, &my_ip) != 1) {
      napi_throw_type_error(env, nullptr, "Local Ip is invalid!");
      return nullptr;
    }

    status = napi_get_named_property(env, args[0], "remote_ip", &remote_ip);
    assert(status == napi_ok);

    status = napi_typeof(env, remote_ip, &remote_ipValueType);
    assert(status == napi_ok);

    if (remote_ipValueType == napi_undefined) {
      napi_throw_type_error(env, nullptr, "remote_ip must be defined");
      return nullptr;  
    }

    status = napi_get_value_string_utf8(env, remote_ip, rIp, r_ipSize, &r_ipCopiedBytes);
    assert(status == napi_ok);

    if (inet_pton(AF_INET, (const char*)rIp, &r_ip) != 1) {
      napi_throw_type_error(env, nullptr, "Remote Ip is invalid!");
      return nullptr;
    }

    status = napi_get_named_property(env, args[0], "remote_mac", &remote_mac);
    assert(status == napi_ok);

    status = napi_typeof(env, remote_mac, &remote_macValueType);
    assert(status == napi_ok);

    if (remote_macValueType != napi_undefined) {
      
      status = napi_get_value_string_utf8(env, remote_mac, rMac, r_macSize, &r_macCopiedBytes);
      assert(status == napi_ok);

      int i;

      if (strlen((const char*)rMac) == 17 &&
          rMac[2] == ':' && rMac[5] == ':' && rMac[8] == ':' &&
          rMac[11] == ':' && rMac[14] == ':') {

        for ( i = 0; i < 6; i++) {
          int v;

          if (sscanf((const char*)rMac + i * 3, "%02x", &v) == 1 && v >=0 && v <= 255)
            r_mac[i] = (unsigned char)(v && 0xff);
          else
            break;
        }

        if (i == 6) {
          good_mac = 1;
        }

        if (!good_mac) {
          napi_throw_error(env, nullptr, "Bad Mac Address");
          return nullptr;
        }
      }    

      if (argc > 1) {
        napi_valuetype countType;

        status = napi_typeof(env, args[1], &countType);
        assert(status == napi_ok);

        if(countType != napi_number) {
          napi_throw_type_error(env, nullptr, "Second argument must be a number");
          return nullptr;
        }

        status = napi_get_value_int32(env, args[1], &count);
        assert(status == napi_ok);
      }

      if (argc > 2) {
        napi_valuetype doPingType;

        status = napi_typeof(env, args[2], &doPingType);
        assert(status == napi_ok);

        if (doPingType != napi_ok) {
          napi_throw_type_error(env, nullptr, "Third Argument must be a Boolean");
          return nullptr;
        }

        status = napi_get_value_bool(env, args[2], &doPing);
        assert(status == napi_ok);

        if (doPing && !good_mac) {
          napi_throw_error(env, nullptr, "Can\'t do ping without a remote mac address");
          return nullptr;
        }
      }
    }

    napi_value cnt;
    count = if_send_spoof_request((const char*)dev, my_ip, r_ip, good_mac ? r_mac : NULL, count, doPing ? 1 : 0);
    status = napi_create_int32(env, count, &cnt);
    assert(status == napi_ok);

    return cnt;
  }