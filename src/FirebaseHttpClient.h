#ifndef FIREBASE_HTTP_CLIENT_H
#define FIREBASE_HTTP_CLIENT_H

#ifdef DEBUG_ESP_PORT
#define FBDEBUGF(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#define FBDEBUGLN(...) DEBUG_ESP_PORT.println( __VA_ARGS__ )
#endif
#ifndef FBDEBUGF
#define FBDEBUGF(...)
#endif
#ifndef FBDEBUGLN
#define FBDEBUGLN(...)
#endif

#include <string>

// using std::shared_ptr;
#include <Arduino.h>
#include <memory>
#include "Stream.h"



struct HttpStatus {
  static const int TEMPORARY_REDIRECT = 307;
};

class FirebaseHttpClient {
 public:
  static std::shared_ptr<FirebaseHttpClient> create();//factory function 
  virtual ~FirebaseHttpClient(){}

  virtual void setReuseConnection(bool reuse) = 0;
  virtual bool begin(const String& url) = 0;
  virtual bool begin(const String& host, const String& path) = 0;
  virtual bool beginApi(const String& host, const String& path) = 0;

  virtual void end() = 0;

  virtual void addHeader(const String& name, const String& value) = 0;
  virtual void collectHeaders(const char* header_keys[],
                              const int header_key_count) = 0;
  virtual String header(const String& name) = 0;

  virtual int sendRequest(const String& method, const String& data) = 0;

  virtual String getString() = 0;

  virtual Stream* getStreamPtr() = 0;

  virtual String errorToString(int error_code) = 0;

  virtual bool connected() = 0;

  virtual void setTimeout(int timeout_ms)=0;
 protected:
  static const uint16_t kFirebasePort = 443;
  void setClock();
  void initClient();
};

// static const char kFirebaseTokenFingerprint[] PROGMEM = //"C6 7F D3 A1 50 01 EA 2E 0E B0 D8 1C E7 BB 99 20 56 10 AF 2C";
//       {0xC6, 0x7F, 0xD3, 0xA1, 0x50, 0x01, 0xEA, 0x2E, 0x0E, 0xB0, 0xD8, 0x1C, 0xE7, 0xBB, 0x99, 0x20, 0x56, 0x10, 0xAF, 0x2C};
// static const uint8_t kFirebaseFingerprint[20] =//"6F D0 9A 52 C0 E9 E4 CD A0 D3 02 A4 B7 A1 92 38 2D CA 2F 26";
//       {0x6F, 0xD0, 0x9A, 0x52, 0xC0, 0xE9, 0xE4, 0xCD, 0xA0, 0xD3, 0x02, 0xA4, 0xB7, 0xA1, 0x92, 0x38, 0x2D, 0xCA, 0x2F, 0x26};

// static const char kFirebaseTokenFingerprint[]  = //"48 27 7a 0c ae 14 14 d3 ee f3 e9 9f d6 d1 1a 80 a3 3c d7 2c";
//       //"C6 7F D3 A1 50 01 EA 2E 0E B0 D8 1C E7 BB 99 20 56 10 AF 2C";
//       {0xC6, 0x7F, 0xD3, 0xA1, 0x50, 0x01, 0xEA, 0x2E, 0x0E, 0xB0, 0xD8, 0x1C, 0xE7, 0xBB, 0x99, 0x20, 0x56, 0x10, 0xAF, 0x2C};
//       //{0x48, 0x27, 0x7a, 0x0c, 0xae, 0x14, 0x14, 0xd3, 0xee, 0xf3, 0xe9, 0x9f, 0xd6, 0xd1, 0x1a, 0x80, 0xa3, 0x3c, 0xd7, 0x2c};
static const uint8_t kFirebaseFingerprint[] = //"6F D0 9A 52 C0 E9 E4 CD A0 D3 02 A4 B7 A1 92 38 2D CA 2F 26";
      {};//0x6F, 0xD0, 0x9A, 0x52, 0xC0, 0xE9, 0xE4, 0xCD, 0xA0, 0xD3, 0x02, 0xA4, 0xB7, 0xA1, 0x92, 0x38, 0x2D, 0xCA, 0x2F, 0x26};


static const char GlobalSign_Root_CAR2[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDujCCAqKgAwIBAgILBAAAAAABD4Ym5g0wDQYJKoZIhvcNAQEFBQAwTDEgMB4G
A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjIxEzARBgNVBAoTCkdsb2JhbFNp
Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDYxMjE1MDgwMDAwWhcNMjExMjE1
MDgwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMjETMBEG
A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI
hvcNAQEBBQADggEPADCCAQoCggEBAKbPJA6+Lm8omUVCxKs+IVSbC9N/hHD6ErPL
v4dfxn+G07IwXNb9rfF73OX4YJYJkhD10FPe+3t+c4isUoh7SqbKSaZeqKeMWhG8
eoLrvozps6yWJQeXSpkqBy+0Hne/ig+1AnwblrjFuTosvNYSuetZfeLQBoZfXklq
tTleiDTsvHgMCJiEbKjNS7SgfQx5TfC4LcshytVsW33hoCmEofnTlEnLJGKRILzd
C9XZzPnqJworc5HGnRusyMvo4KD0L5CLTfuwNhv2GXqF4G3yYROIXJ/gkwpRl4pa
zq+r1feqCapgvdzZX99yqWATXgAByUr6P6TqBwMhAo6CygPCm48CAwEAAaOBnDCB
mTAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUm+IH
V2ccHsBqBt5ZtJot39wZhi4wNgYDVR0fBC8wLTAroCmgJ4YlaHR0cDovL2NybC5n
bG9iYWxzaWduLm5ldC9yb290LXIyLmNybDAfBgNVHSMEGDAWgBSb4gdXZxwewGoG
3lm0mi3f3BmGLjANBgkqhkiG9w0BAQUFAAOCAQEAmYFThxxol4aR7OBKuEQLq4Gs
J0/WwbgcQ3izDJr86iw8bmEbTUsp9Z8FHSbBuOmDAGJFtqkIk7mpM0sYmsL4h4hO
291xNBrBVNpGP+DTKqttVCL1OmLNIG+6KYnX3ZHu01yiPqFbQfXf5WRDLenVOavS
ot+3i9DAgBkcRcAtjOj4LaR0VknFBbVPFd5uRHg5h6h+u/N5GJG79G+dwfCMNYxd
AfvDbbnvRG15RjF+Cv6pgsH/76tuIMRQyV+dTZsXjAzlAcmgQWpzU/qlULRuJQ/7
TBj0/VLZjmmx6BEP3ojY+x1J96relc8geMJgEtslQIxq/H5COEBkEveegeGTLg==
-----END CERTIFICATE-----
)EOF";
#endif  // FIREBASE_HTTP_CLIENT_H
