
#include "FirebaseHttpClient.h"

#include <stdio.h>

// The ordering of these includes matters greatly.
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

// Detect whether stable version of HTTP library is installed instead of
// master branch and patch in missing status and methods.
#ifndef HTTP_CODE_TEMPORARY_REDIRECT
#define HTTP_CODE_TEMPORARY_REDIRECT 307
#define USE_ESP_ARDUINO_CORE_2_0_0
#endif

// Firebase now returns `Connection: close` after REST streaming redirection.
//
// Override the built-in ESP8266HTTPClient to *not* close the
// connection if forceReuse it set to `true`.
class ForceReuseHTTPClient : public HTTPClient {
 public:
  void end() {
    if (_forceReuse) {
      _canReuse = true;
    }
    FBDEBUGF(("ForceReuseHTTPClient end, called http.end Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    HTTPClient::end();
  }
  void forceReuse(bool forceReuse) {
    _forceReuse = forceReuse;
  }

 protected:
  bool _forceReuse = false;
};

class FirebaseHttpClientEsp8266 : public FirebaseHttpClient {
 public:
  FirebaseHttpClientEsp8266() {}

  void setReuseConnection(bool reuse) override {
    http_.setReuse(reuse);
    http_.forceReuse(reuse);
  }

  void initClient() {
    //    std::unique_ptr<BearSSL::WiFiClientSecure>client;
    //    BearSSL::X509List cert;
    //    if (client.get() == nullptr) {
    //      client.reset(new BearSSL::WiFiClientSecure);
    //      client->setBufferSizes(2048, 2048);
    //      cert.append(GlobalSign_Root_CAR2);
    //      client->setTrustAnchors(&cert);
    //    }
  }

  void setClock() {
    configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

    FBDEBUGLN("Waiting for NTP time sync: ");
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
      delay(500);
      FBDEBUGLN(".");
      now = time(nullptr);
    }
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    FBDEBUGLN("Current time: ");
    FBDEBUGLN(asctime(&timeinfo));
    yield();
  }

  bool begin(const String& url) override {
    FBDEBUGF(("[begin]begin called with free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    if (!http_.begin(url.c_str(), kFirebaseFingerprint)) {
      //end();
      FBDEBUGF(("[begin]begin fail called with free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
      FBDEBUGLN("[begin]conenct with fingerprint failed");
      //initClient();

      //std::unique_ptr<BearSSL::WiFiClientSecure>client;
      //BearSSL::X509List cert;
      ////client.reset(new BearSSL::WiFiClientSecure);
      //client->setBufferSizes(2048, 2048);
      ////cert.append(GlobalSign_Root_CAR2);
      //client->setTrustAnchors(&cert);
      //setClock();
      //return http_.begin(*client, url);
      return false;
    }

    FBDEBUGF(("[begin]begin success called with free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    return true;
  }

  bool begin(const String& host, const String& path) override {
    FBDEBUGF(("[begin]connection begin with free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    //std::unique_ptr<BearSSL::WiFiClientSecure> client;
    FBDEBUGF(("[begin]client declear with free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    //http_.begin(client, host, kFirebasePort, path);
    client = (std::make_shared<BearSSL::WiFiClientSecure>());
    FBDEBUGF(("[begin]client reset(new BearSSL::WiFiClientSecure) free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    //auto client = std::make_unique<BearSSL::WiFiClientSecure>();
    client->setBufferSizes(512, 512);
    FBDEBUGF(("[begin]client setBufferSizes(1024, 1024)d free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    //client->setFingerprint(kFirebaseFingerprint);
    FBDEBUGF(("[begin]client setFingerprint(kFirebaseFingerprint) free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
cert = std::make_shared<BearSSL::X509List>();
    FBDEBUGF(("[begin]init cert(GlobalSign_Root_CAR2) free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
cert->append(GlobalSign_Root_CAR2);
    FBDEBUGF(("[begin]append cert(GlobalSign_Root_CAR2) free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
client->setTrustAnchors(cert.get());

client->setCiphersLessSecure();
    // client->setInsecure();
    FBDEBUGF(("[begin]client client->setTrustAnchors(&cert) free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    setClock();
    FBDEBUGF(("[begin]client setClock free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    FBDEBUGF(("[begin]calling begin with host: [%s] path:[%s] free Heap: %u %lu\n"), host.c_str(), path.c_str(), ESP.getFreeHeap(), micros());
    // // if(!client->connect(host, kFirebasePort)){
    // //   FBDEBUGF(("[begin]connection failed with free Heap: %u %lu\n") , ESP.getFreeHeap(), micros ());
    // //   http_.end();
    // //   FBDEBUGF(("[begin]http_ end with free Heap: %u %lu\n") , ESP.getFreeHeap(), micros ());
    // //   client->stop(1);
    // //   FBDEBUGF(("[begin]client stop with free Heap: %u %lu\n") , ESP.getFreeHeap(), micros ());
    // //   client.reset();
    // //   FBDEBUGF(("[begin]client reset with free Heap: %u %lu\n") , ESP.getFreeHeap(), micros ());
    // //   return false;
    // // }
    if (!http_.begin(*client, host.c_str(), kFirebasePort, path.c_str())) {
      FBDEBUGF(("begin fail called with free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
      //   //end();
      //   FBDEBUGLN("conenct with fingerprint failed");
      //   FBDEBUGLN("begin host: "+ host);
      //   FBDEBUGLN("begin path: "+ path);
      //   //initClient();

      //   //std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
      //   //BearSSL::X509List cert(GlobalSign_Root_CAR2);
      //   ////client.reset(new BearSSL::WiFiClientSecure);
      //   //client->setBufferSizes(2048, 2048);
      //   ////cert.append(GlobalSign_Root_CAR2);
      //   //client->setTrustAnchors(&cert);
      //   //setClock();
      // return http_.begin(*client, host, kFirebasePort, path, true);
      return false;
    }
    // else {
    // //   if(http_.connect()){
    // //     FBDEBUGF(("begin connect called with free Heap: %u %lu\n") , ESP.getFreeHeap(), micros ());
    // //   } else {
    // //     FBDEBUGF(("begin connect failed with free Heap: %u %lu\n") , ESP.getFreeHeap(), micros ());
    // //   }
    // }
    FBDEBUGF(("[begin]begin success called with free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    // return true;
    return true;
  }

  bool beginApi(const String& host, const String& path) override {
    // if(http_.begin(host.c_str(), kFirebasePort, path.c_str(), kFirebaseTokenFingerprint)){

    //   FBDEBUGF(("[beginApi]begin fail called with free Heap: %u %lu\n") , ESP.getFreeHeap(), micros ());
    //   //end();
    //   FBDEBUGLN("[beginApi]conenct with fingerprint failed");
    //   //initClient();

    //   //std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    //   //BearSSL::X509List cert(GlobalSign_Root_CAR2);
    //   // client.reset(new BearSSL::WiFiClientSecure);
    //   // client->setBufferSizes(1024, 1024);
    //   ////cert.append(GlobalSign_Root_CAR2);
    //   //client->setTrustAnchors(&cert);
    //   //setClock();
    //   //return http_.begin(*client, host, kFirebasePort, path, true);
    //   return false;
    // }
    FBDEBUGF(("[beginApi]begin success called with free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    return true;
  }

  void end() override {
    //http_.end();
    if (client.get() != nullptr) {
      http_.end();
      FBDEBUGF(("[end] client.use_count:%d Free Heap: %u %lu\n"), client.use_count());
      client->stop(1);
      client.reset();
    }
    if (cert.get() != nullptr) {
      FBDEBUGF(("[end] cert.use_count:%d Free Heap: %u %lu\n"), cert.use_count());
      cert.reset();
    }
  }

  void addHeader(const String& name, const String& value) override {
    http_.addHeader(name.c_str(), value.c_str());
  }

  void collectHeaders(const char* header_keys[], const int count) override {
    http_.collectHeaders(header_keys, count);
  }

  String header(const String& name) override {
    return http_.header(name.c_str()).c_str();
  }

  int sendRequest(const String& method, const String& data) override {
    //yield();

    FBDEBUGF(("[sendRequest] method:%s data:%s free Heap: %u %lu\n"), method.c_str(), data.c_str(), ESP.getFreeHeap(), micros());

    return http_.sendRequest(method.c_str(), (uint8_t*)data.c_str(), data.length());
  }

  String getString() override {
    return http_.getString().c_str();
  }

  Stream* getStreamPtr() override {
    return http_.getStreamPtr();
  }

  String errorToString(int error_code) override {
    return HTTPClient::errorToString(error_code).c_str();
  }

  bool connected() override {
    return http_.connected();
  }

  void setTimeout(int timeout_ms) {
    http_.setTimeout(timeout_ms);
  }

 private:
  std::shared_ptr<BearSSL::WiFiClientSecure> client;
  //BearSSL::X509List cert;//(GlobalSign_Root_CAR2);
  std::shared_ptr<BearSSL::X509List> cert;
  ForceReuseHTTPClient http_;
};

std::shared_ptr<FirebaseHttpClient> FirebaseHttpClient::create() {
  FBDEBUGF(("[FirebaseHttpClient::create()] creating a new HTTPClient ---------- free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  return static_cast<std::shared_ptr<FirebaseHttpClient>>(std::make_shared<FirebaseHttpClientEsp8266>());
}
