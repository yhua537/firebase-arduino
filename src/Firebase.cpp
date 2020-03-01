//
// Copyright 2015 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "Firebase.h"

using std::shared_ptr;
using std::unique_ptr;

#define FIREBASE_GRANT_TYPE "grant_type=refresh_token&refresh_token="
#define FIREBASE_LOGIN_JSON "{\"u\":\"%s\",\"p\":\"%s\"}%c"

namespace {
String makeFirebaseURL(const String& path, const String& auth) {
  String url;
  if (path[0] != '/') {
    url = "/";
  }
  url += path + ".json";
  if (auth.length() > 0) {
    url += "?auth=" + auth;
  }
  FBDEBUGLN("[makeFirebaseURL] url: " + url);
  return url;
}

String makeFirebaseTokenURL(const String& path, const String& key) {
  String url;
  if (path[0] != '/') {
    url = "/";
  }
  url += "/v1/token";
  if (key.length() > 0) {
    url += "?key=" + key;
  }
  return url;
}
// String makeFirebaseLoginURL(const String& path, const String& key) {
//   String url;
//   if (path[0] != '/') {
//     url = "/";
//   }
//   url += "/v3/relyingparty/verifyPassword";
//   if (key.length() > 0) {
//     url += "?key=" + key;
//   }
//   return url;
// }
}  // namespace

Firebaseunused::Firebaseunused(const String&& host, const String&& auth) {
  host_ = (host);
  auth_ = (auth);
  //http_.reset(FirebaseHttpClient::create());
  //http_->setReuseConnection(true);
}

// Firebase::~Firebase() {
//   if (http_.get() != NULL) {
//     http_->end();
//     delete http_;
//     http_.reset();
//   }
// }

const String& Firebaseunused::auth() const {
  return auth_;
}

boolean FirebaseCall::analyzeError(const String& method, int status, const String& path_with_auth) {
  if (status != 200 && status != HttpStatus::TEMPORARY_REDIRECT) {
    error_ = std::make_shared<FirebaseError>(status,
                                             String(method) + " " + path_with_auth +
                                                 ": " + http_->errorToString(status));
    return false;
  }
  error_ = std::make_shared<FirebaseError>();
  return true;
}

FirebaseCall::~FirebaseCall() {
  FBDEBUGF(("[~FirebaseCall] http_.use_count:%d\n"), http_.use_count());
  if (http_.get() != nullptr) {
    http_->setReuseConnection(false);
    //http_->setTimeout(1);
    http_->end();
    http_.reset();
  }
  FBDEBUGF(("[~FirebaseCall] buffer_.use_count:%d\n"), buffer_.use_count());
  if (buffer_.get() != nullptr) {
    buffer_.get()->clear();
    buffer_.reset();
  }
  FBDEBUGF(("[~FirebaseCall] error_.use_count:%d\n"), error_.use_count());
  if (error_.get() != nullptr) {
    error_.reset();
  }
  FBDEBUGF(("[~FirebaseCall] http_.use_count:%d buffer_.use_count:%d error_.use_count:%d\n"), http_.use_count(), buffer_.use_count(), error_.use_count());
}

const JsonObject& FirebaseCall::json() {
  //TODO(edcoyne): This is not efficient, we should do something smarter with
  //the buffers. kotl: Is this still valid?
  if (buffer_.get() == nullptr) {
    buffer_ = std::make_shared<StaticJsonBuffer<FIREBASE_JSONBUFFER_SIZE>>();
  }
  return buffer_.get()->parseObject(response().c_str());
}

int FirebaseCall::refreshLogin(const String& host, const String& path,
                               const String& user, const String& password,
                               String& refreshToken, String& auth) {
  if (auth.length() > 0) ESP.resetFreeContStack();
  FBDEBUGF(("[refreshLogin] start login to get token\n"));
  //String path_with_auth = makeFirebaseLoginURL(path, key);
  //http_->setTimeout(15000);
  if (http_->begin(host, path)) {
    http_->addHeader("Content-Type", "application/json");
    char jsonBuffer[40];
    sprintf(jsonBuffer, FIREBASE_LOGIN_JSON, user.c_str(), password.c_str(), '\0');
    int status = http_->sendRequest("POST", jsonBuffer);
    //yield();
    FBDEBUGF("[refreshLogin] waitting for response\n");
    yield();
    FBDEBUGF("[refreshLogin] got response\n");
    if (status == 200) {
      response_ = http_->getString();
      FBDEBUGF("%s", response_.c_str());
      int pos = response_.indexOf("refresh_token", 0) + 14;
      pos = response_.indexOf('"', pos) + 1;
      int posEnd = response_.indexOf('"', pos);
      FBDEBUGF("pos: %d, posEnd: %d\n", pos, posEnd);
      if (posEnd - pos > 100)
        refreshToken = response_.substring(pos, posEnd);
      FBDEBUGLN(refreshToken);

      //pos = response_.indexOf("id_token", 0)+9;
      pos = response_.indexOf("access_token", 0) + 13;
      pos = response_.indexOf('"', pos) + 1;
      posEnd = response_.indexOf('"', pos);
      FBDEBUGF("pos: %d, posEnd: %d\n", pos, posEnd);
      if (posEnd - pos > 100)
        auth = response_.substring(pos, posEnd);
      FBDEBUGLN(auth);
    }
    return status;
  } else {
    int status = http_->sendRequest("GET", "");
    analyzeError("refreshLogin TOKENGET", -1, path);
    http_->end();
    return -1;
  }
}

int FirebaseCall::refreshToken(const String& host, const String& path, const String& key,
                               String& refreshToken, String& auth) {
  if (auth.length() > 0) ESP.resetFreeContStack();
  FBDEBUGF(("[refreshToken] start refreshing token\n"));
  String path_with_auth = makeFirebaseTokenURL(path, key);
  //http_->setTimeout(15000);
  if (http_->begin(host, path_with_auth)) {
    http_->addHeader("Content-Type", "application/x-www-form-urlencoded");
    int status = http_->sendRequest("POST", FIREBASE_GRANT_TYPE + refreshToken);
    yield();
    response_ = http_->getString();
    yield();
    FBDEBUGLN(response_);
    if (status == 200) {
      response_ = http_->getString();
      int pos = response_.indexOf("refresh_token", 0) + 14;
      pos = response_.indexOf('"', pos) + 1;
      int posEnd = response_.indexOf('"', pos);
      if (posEnd - pos > 100)
        refreshToken = response_.substring(pos, posEnd);
      FBDEBUGLN(refreshToken);

      //pos = response_.indexOf("id_token", 0)+9;
      pos = response_.indexOf("access_token", 0) + 13;
      pos = response_.indexOf('"', pos) + 1;
      posEnd = response_.indexOf('"', pos);
      if (posEnd - pos > 100)
        auth = response_.substring(pos, posEnd);
      FBDEBUGLN(auth);
    }
    return status;
  } else {
    int status = http_->sendRequest("GET", "");
    analyzeError("refreshToken TOKENGET", -1, path_with_auth);
    http_->end();
    return -1;
  }
}

// FirebaseRequest
int FirebaseRequest::sendRequest(
    const String& host, const String& auth,
    const String& method, const String& path, const String& data) {
  ESP.resetFreeContStack();
  String path_with_auth = makeFirebaseURL(path, auth);

  if (auth.length() <= 0) {
    if (!analyzeError(method.c_str(), 401, path_with_auth)) {
      http_->setReuseConnection(false);
      http_->end();
    }
    return 401;
  }
  //http_->setTimeout(15000);
  http_->setReuseConnection(true);

  //FBDEBUGLN("/n/n/n/n-------------------start sending request begin:/n/n");
  if (http_->begin(host, path_with_auth)) {
    if (auth.length() > 0) ESP.resetFreeContStack();
    yield();
    int status = http_->sendRequest(method, data);

    //FBDEBUGLN("/n/n/n/n---------------------Firebase Error:"+(String)status);
    //yield();

    if (!analyzeError(method.c_str(), status, path_with_auth)) {
      http_->setReuseConnection(false);
      http_->end();
      return status;
    }
    response_ = http_->getString();
    return status;
  } else {
    //int status = http_->sendRequest("GET", "");
    analyzeError("STREAM", -1, path_with_auth);
    http_->setReuseConnection(false);
    http_->end();
    return -1;
  }
}

// FirebaseStream
bool FirebaseStream::startStreaming(const String& host, const String& auth, const String& path) {
  ESP.resetFreeContStack();
  String path_with_auth = makeFirebaseURL(path, auth);
  //http_->setTimeout(5000);
  http_->setReuseConnection(true);

  //FBDEBUGLN("/n/n/n/n-------------------start sending request begin:/n/n");
  if (http_->begin(host, path_with_auth)) {
    http_->addHeader("Accept", "text/event-stream");
    const char* headers[] = {"Location"};
    http_->collectHeaders(headers, 1);
    //yield();
    //FBDEBUGLN("/n/n/n/n-------------------start sending request :/n/n");
    
    if (auth.length() > 0) ESP.resetFreeContStack();
    int status = http_->sendRequest("GET", "");
    if (!analyzeError("STREAM", status, path_with_auth)) {
      // if(status != 200){
      //FBDEBUGLN("/n/n/n/n---------------------Firebase Error:"+(String)status);
      http_->end();
      return false;
    }

    while (status == HttpStatus::TEMPORARY_REDIRECT) {
      String location = http_->header("Location");
      http_->setReuseConnection(false);
      http_->end();
      http_->setReuseConnection(true);
      http_->begin(location);
      //yield();
      status = http_->sendRequest("GET", String());
    }
  } else {
    int status = http_->sendRequest("GET", "");
    analyzeError("STREAM", -1, path_with_auth);
    http_->end();
    return false;
  }
  return true;
}
