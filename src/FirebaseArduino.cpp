//
// Copyright 2016 Google Inc.
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

#include "FirebaseArduino.h"

// This is needed to compile String on esp8266.
template class std::basic_string<char>;

void FirebaseArduino::begin(const String&& host, const String&& auth,
                            const String&& tokenHost, const String&& tokenPath,
                            const String&& refreshToken, const String&& key,
                            const String&& loginHost, const String&& loginPath,
                            const String&& usr, const String&& password) {
  //FBDEBUGF(("begin called Free Heap: %u %lu\n") , ESP.getFreeHeap(), micros ());
  host_ = host;
  //FBDEBUGF(("host assigned Free Heap: %u %lu\n") , ESP.getFreeHeap(), micros ());
  auth_ = auth;
  //FBDEBUGF(("auth assigned Free Heap: %u %lu\n") , ESP.getFreeHeap(), micros ());
  tokenHost_ = tokenHost;
  //FBDEBUGF(("tokenHost assigned Free Heap: %u %lu\n") , ESP.getFreeHeap(), micros ());
  tokenPath_ = tokenPath;
  //FBDEBUGF(("tokenPath assigned Free Heap: %u %lu\n") , ESP.getFreeHeap(), micros ());
  refreshToken_ = refreshToken;
  //FBDEBUGF(("refreshToken assigned Free Heap: %u %lu\n") , ESP.getFreeHeap(), micros ());
  key_ = key;
  loginHost_ = loginHost;
  loginPath_ = loginPath;
  user_ = usr;
  pwd_ = password;
  //FBDEBUGF(("key assigned Free Heap: %u %lu\n") , ESP.getFreeHeap(), micros ());
  if (auth_.length() == 0 && usr.length() > 0) {
    FBDEBUGF(("[FirebaseArduino::begin] start to login first Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    int status = handleLoginError(false);
    FBDEBUGF(("[FirebaseArduino::begin] login Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  } else if (auth_.length() == 0 && refreshToken.length() < 2) {
    FBDEBUGF(("[FirebaseArduino::begin] start fix token first Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    int status = handleTokenError(false);
    FBDEBUGF(("[FirebaseArduino::begin] done fix token Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  }
}

void FirebaseArduino::initStream() {
  connectionEnd();
  if (stream_http_.get() == nullptr) {
    FBDEBUGF(("[initStream] called Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    stream_http_ = std::move(FirebaseHttpClient::create());
    FBDEBUGF(("[initStream] FirebaseHttpClient called Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    stream_http_->setReuseConnection(true);
    FBDEBUGF(("[initStream] setReuseConnection called Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  }
  if (stream_.get() == nullptr) {
    stream_ = std::make_shared<FirebaseStream>(stream_http_);
    FBDEBUGF(("[initStream] FirebaseStream Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  }

  FBDEBUGF(("[initStream] end of init stream_.use_count:%d stream_http_.use_count:%d error_.use_count:%d - expecting all 1 Free Heap: %u %lu\n"), stream_.use_count(), stream_http_.use_count(), error_.use_count(), ESP.getFreeHeap(), micros());
  FBDEBUGF(("[initStream] end of init req_.use_count:%d req_http_.use_count:%d error_.use_count:%d - expecting all 0 Free Heap: %u %lu\n"), req_.use_count(), req_http_.use_count(), error_.use_count(), ESP.getFreeHeap(), micros());
}

void FirebaseArduino::initRequest() {
  //connectionEnd();
  if (req_http_.get() == nullptr) {
    FBDEBUGF(("[initRequest] called Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    req_http_ = std::move(FirebaseHttpClient::create());
    FBDEBUGF(("[initRequest] FirebaseHttpClient called Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    req_http_->setReuseConnection(true);
    FBDEBUGF(("[initRequest] setReuseConnection called Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  }
  if (req_.get() == nullptr) {
    req_ = std::make_shared<FirebaseRequest>(req_http_);
    FBDEBUGF(("[initRequest] firebaseRequest Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  }

  FBDEBUGF(("[initRequest] end of init stream_.use_count:%d stream_http_.use_count:%d error_.use_count:%d - expecting all 1 Free Heap: %u %lu\n"), stream_.use_count(), stream_http_.use_count(), error_.use_count(), ESP.getFreeHeap(), micros());
  FBDEBUGF(("[initRequest] end of init req_.use_count:%d req_http_.use_count:%d error_.use_count:%d - expecting all 1 Free Heap: %u %lu\n"), req_.use_count(), req_http_.use_count(), error_.use_count(), ESP.getFreeHeap(), micros());
}

int FirebaseArduino::handleTokenError(bool isStream) {
  //get new token here
  int status = 200;
  if (!isStream) {
    initRequest();
    status = req_.get()->refreshToken(tokenHost_, tokenPath_, key_, refreshToken_, auth_);
  } else {
    initStream();
    status = stream_.get()->refreshToken(tokenHost_, tokenPath_, key_, refreshToken_, auth_);
  }
  if (status == 400) {
    FBDEBUGLN("\n Error unAuth 400");

    connectionEnd();
    status = handleLoginError(false);
    return status;
    //get new token here
  }
  connectionEnd();
  //FBDEBUGLN(refreshToken_);
  //FBDEBUGLN(auth_);
  return status;
}

int FirebaseArduino::handleLoginError(bool isStream) {
  //get new token here
  int status = 200;
  if (!isStream) {
    initRequest();
    status = req_.get()->refreshLogin(loginHost_, loginPath_, user_, pwd_, refreshToken_, auth_);
  } else {
    initStream();
    status = stream_.get()->refreshLogin(loginHost_, loginPath_, user_, pwd_, refreshToken_, auth_);
  }
  connectionEnd();
  //FBDEBUGLN(refreshToken_);
  //FBDEBUGLN(auth_);
  return status;
}

String FirebaseArduino::pushInt(const String& path, int value) {
  return push(path, value);
}

String FirebaseArduino::pushFloat(const String& path, float value) {
  return push(path, value);
}

String FirebaseArduino::pushBool(const String& path, bool value) {
  return push(path, value);
}

String FirebaseArduino::pushString(const String& path, const String& value) {
  JsonVariant json(value.c_str());
  return push(path, json);
}

String FirebaseArduino::push(const String& path, const JsonVariant& value) {
  int size = value.measureLength() + 1;
  char* buf = new char[size];
  value.printTo(buf, size);
  initRequest();
  int status = req_.get()->sendRequest(host_, auth_, "POST", path.c_str(), buf);
  error_ = req_.get()->error();
  if (error_->code() == 401) {
    FBDEBUGLN("\n Error unAuth 401");

    connectionEnd();
    status = handleTokenError(false);
    return "";
    //get new token here
  } else if (error_->code() == 400) {
    FBDEBUGLN("\n Error unAuth 400");

    connectionEnd();
    status = handleLoginError(false);
    return "";
    //get new token here
  } else if (error_->code() != 200) {
    connectionEnd();
    return "";
  } else {
    const char* name = req_.get()->json()["name"].as<const char*>();
    return name;
  }
  delete buf;
  return "";
}

void FirebaseArduino::setInt(const String& path, int value) {
  set(path, value);
}

void FirebaseArduino::setFloat(const String& path, float value) {
  set(path, value);
}

void FirebaseArduino::setBool(const String& path, bool value) {
  set(path, value);
}

void FirebaseArduino::setString(const String& path, const String& value) {
  JsonVariant json(value.c_str());
  set(path, json);
}

bool FirebaseArduino::set(const String& path, const JsonVariant& value) {
  int status;
  FBDEBUGF(("[FirebaseArduino::set] entering Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  // req_.get()->refreshToken(tokenHost_, tokenPath_, key_, refreshToken_);
  // if(auth_ == "EMPTY") {
  //   FBDEBUGF(("[FirebaseArduino::set] fix token first Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros ());
  //   status = handleTokenError(false);
  // }
  FBDEBUGF(("[FirebaseArduino::set] initRequest Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  initRequest();
  FBDEBUGF(("[FirebaseArduino::set] initRequest Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  int size = value.measureLength() + 1;
  char* buf = new char[size];
  value.printTo(buf, size);
  FBDEBUGF(("[FirebaseArduino::set] new buf Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  status = req_.get()->sendRequest(host_, auth_, "PUT", path.c_str(), buf);
  FBDEBUGF(("[FirebaseArduino::set] sendRequest put %s Free Heap: %u %lu\n"), buf, ESP.getFreeHeap(), micros());
  error_ = req_.get()->error();
  FBDEBUGF(("[FirebaseArduino::set] req_.get()->error() Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  delete buf;
  FBDEBUGF(("[FirebaseArduino::set] free buf Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  if (error_->code() == 401) {
    FBDEBUGF(("[FirebaseArduino::set] unAuthd 401 Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    connectionEnd();
    FBDEBUGF(("[FirebaseArduino::set] reqEnd() for 401 Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());

    status = handleTokenError(false);
    // if(status == 200){
    //   //get new token here
    //   initRequest();
    //   buf= new char[size];
    //   value.printTo(buf, size);
    //   FBDEBUGF(("[FirebaseArduino::set] retry initRequest Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros ());
    //   status = req_.get()->sendRequest(host_, auth_, "PUT", path.c_str(), buf);
    //   FBDEBUGF(("[FirebaseArduino::set] retry sendRequest put Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros ());
    //   error_ = req_.get()->error();
    //   FBDEBUGF(("[FirebaseArduino::set] retry req_.get()->error() Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros ());
    //   delete buf;
    //   FBDEBUGF(("[FirebaseArduino::set] free buf Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros ());
    //   if(error_->code() != 200){
    //     FBDEBUGF(("[FirebaseArduino::set] retry other error %d Free Heap: %u %lu\n"), error_->code(), ESP.getFreeHeap(), micros ());
    //     connectionEnd();
    //     FBDEBUGF(("[FirebaseArduino::set] retry reqEnd() for other error %d Free Heap: %u %lu\n"),  error_->code(), ESP.getFreeHeap(), micros ());
    //   }
    // }

    //   //if(status == 200){return set(path, value);}
    return false;
  } else if (error_->code() == 400) {
    FBDEBUGLN("\n Error unAuth 400");

    connectionEnd();
    status = handleLoginError(false);
    return false;
    //get new token here
  } else if (error_->code() != 200) {
    FBDEBUGF(("[FirebaseArduino::set] other error %d Free Heap: %u %lu\n"), error_->code(), ESP.getFreeHeap(), micros());
    //connectionEnd();
    //FBDEBUGF(("[FirebaseArduino::set] reqEnd() for other error %d Free Heap: %u %lu\n"),  error_->code(), ESP.getFreeHeap(), micros ());

    return false;
  }
  // delete buf;
  FBDEBUGF(("[FirebaseArduino::set] exiting Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  return true;
}

bool FirebaseArduino::patch(const String& path, const JsonVariant& value) {
  int status;
  FBDEBUGF(("[FirebaseArduino::patch] entering Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  // req_.get()->refreshToken(tokenHost_, tokenPath_, key_, refreshToken_);
  // if(auth_ == "EMPTY") {
  //   FBDEBUGF(("[FirebaseArduino::set] fix token first Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros ());
  //   status = handleTokenError(false);
  // }
  FBDEBUGF(("[FirebaseArduino::patch] initRequest Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  initRequest();
  FBDEBUGF(("[FirebaseArduino::patch] initRequest Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  int size = value.measureLength() + 1;
  char* buf = new char[size];
  value.printTo(buf, size);
  FBDEBUGF(("[FirebaseArduino::patch] new buf %s Free Heap: %u %lu\n"), buf, ESP.getFreeHeap(), micros());
  status = req_.get()->sendRequest(host_, auth_, "PATCH", path.c_str(), buf);
  FBDEBUGF(("[FirebaseArduino::patch] sendRequest PATCH Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  error_ = req_.get()->error();
  FBDEBUGF(("[FirebaseArduino::patch] req_.get()->error() Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  delete buf;
  FBDEBUGF(("[FirebaseArduino::patch] free buf Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  if (error_->code() == 401) {
    FBDEBUGF(("[FirebaseArduino::patch] unAuthd 401 Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    connectionEnd();
    FBDEBUGF(("[FirebaseArduino::patch] reqEnd() for 401 Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());

    status = handleTokenError(false);
    // if(status == 200){
    //   //get new token here
    //   initRequest();
    //   buf= new char[size];
    //   value.printTo(buf, size);
    //   FBDEBUGF(("[FirebaseArduino::set] retry initRequest Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros ());
    //   status = req_.get()->sendRequest(host_, auth_, "PUT", path.c_str(), buf);
    //   FBDEBUGF(("[FirebaseArduino::set] retry sendRequest put Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros ());
    //   error_ = req_.get()->error();
    //   FBDEBUGF(("[FirebaseArduino::set] retry req_.get()->error() Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros ());
    //   delete buf;
    //   FBDEBUGF(("[FirebaseArduino::set] free buf Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros ());
    //   if(error_->code() != 200){
    //     FBDEBUGF(("[FirebaseArduino::set] retry other error %d Free Heap: %u %lu\n"), error_->code(), ESP.getFreeHeap(), micros ());
    //     connectionEnd();
    //     FBDEBUGF(("[FirebaseArduino::set] retry reqEnd() for other error %d Free Heap: %u %lu\n"),  error_->code(), ESP.getFreeHeap(), micros ());
    //   }
    // }

    //   //if(status == 200){return set(path, value);}
    return false;
  } else if (error_->code() == 400) {
    FBDEBUGLN("\n Error unAuth 400");

    connectionEnd();
    status = handleLoginError(false);
    return false;
    //get new token here
  } else if (error_->code() != 200) {
    FBDEBUGF(("[FirebaseArduino::patch] other error %d Free Heap: %u %lu\n"), error_->code(), ESP.getFreeHeap(), micros());
    //connectionEnd();
    //FBDEBUGF(("[FirebaseArduino::patch] reqEnd() for other error %d Free Heap: %u %lu\n"),  error_->code(), ESP.getFreeHeap(), micros ());

    return false;
  }
  // delete buf;
  FBDEBUGF(("[FirebaseArduino::set] exiting Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  return true;
}

void FirebaseArduino::getRequest(const String& path) {
  initRequest();
  req_.get()->sendRequest(host_, auth_, "GET", path.c_str());
  error_ = req_.get()->error();
  if (error_->code() == 401) {
    //    FBDEBUGLN("\n Error  unAuth 401");
    //    //reqEnd();
    //    int status = handleTokenError(false);
    //    //get new token here
    //    //initRequest();
    //    //int status = req_.get()->refreshToken(tokenHost_, tokenPath_, key_, refreshToken_, auth_);
    //    //if(status == 200){return getRequest(path);}
    //  } else if(error_->code() != 200){
    //    reqEnd();
    connectionEnd();
    handleTokenError(false);
  } else if (error_->code() == 400) {
    FBDEBUGLN("\n Error unAuth 400");

    connectionEnd();
    handleLoginError(false);
    //get new token here
  }
  connectionEnd();
}

FirebaseObject FirebaseArduino::get(const String& path) {
  getRequest(path);
  if (failed()) {
    return FirebaseObject{""};
  }
  return FirebaseObject(req_.get()->response().c_str());
}

int FirebaseArduino::getInt(const String& path) {
  getRequest(path);
  if (failed()) {
    return 0;
  }
  FirebaseObject obj = FirebaseObject(req_.get()->response().c_str());
  int i = obj.getInt();
  obj.clear();
  return i;
}

float FirebaseArduino::getFloat(const String& path) {
  getRequest(path);
  if (failed()) {
    return 0.0f;
  }
  FirebaseObject obj = FirebaseObject(req_.get()->response().c_str());
  float f = obj.getFloat();
  obj.clear();
  return f;
}

String FirebaseArduino::getString(const String& path) {
  getRequest(path);
  if (failed()) {
    return "";
  }
  FirebaseObject obj = FirebaseObject(req_.get()->response().c_str());
  String s = obj.getString();
  obj.clear();
  return s;
}

bool FirebaseArduino::getBool(const String& path) {
  getRequest(path);
  if (failed()) {
    return "";
  }
  FirebaseObject obj = FirebaseObject(req_.get()->response().c_str());
  bool b = obj.getBool();
  obj.clear();
  return b;
  // return req_.get()->response().toInt()==1;
}
void FirebaseArduino::remove(const String& path) {
  initRequest();
  req_.get()->sendRequest(host_, auth_, "DELETE", path.c_str());
  error_ = req_.get()->error();
  if (error_->code() == 401) {
    connectionEnd();
    handleTokenError(true);
  } else if (error_->code() == 400) {
    FBDEBUGLN("\n Error unAuth 400");

    connectionEnd();
    handleLoginError(false);
    //get new token here
  }
}

bool FirebaseArduino::stream(const String& path) {
  initStream();
  FBDEBUGF(("[stream] startStreaming Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  stream_.get()->startStreaming(host_, auth_, path.c_str());
  FBDEBUGF(("[stream] startStreaming check error Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  error_ = stream_.get()->error();
  FBDEBUGF(("[stream] startStreaming error %d ready Free Heap: %u %lu\n"), error_->code(), ESP.getFreeHeap(), micros());
  if (error_->code() == 401) {
    FBDEBUGF(("[stream]unAuth Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    //    FBDEBUGLN("\n Error  unAuth 401");
    connectionEnd();
    //    //get new token here
    //initRequest();
    //int status = req_.get()->refreshToken(tokenHost_, tokenPath_, key_, refreshToken_, auth_);
    //    delay(3000);
    handleTokenError(true);
    //    //get new token here
    //    //initRequest();
    //    //int status = req_.get()->refreshToken(tokenHost_, tokenPath_, key_, refreshToken_, auth_);
    //    //if(status == 200){return stream(path);}

    return false;
  } else if (error_->code() == 400) {
    FBDEBUGLN("\n Error unAuth 400");

    connectionEnd();
    handleLoginError(false);
    return false;
    //get new token here
  } else if (error_->code() != 200) {
    FBDEBUGF(("[stream] startStreaming failed Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    connectionEnd();
    FBDEBUGF(("[stream] startStreaming streamEnd Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    return false;
  }
  FBDEBUGF(("[stream] startStreaming ready Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  return true;
}

bool FirebaseArduino::available() {
  if (stream_http_.get() == nullptr) {
    error_ = std::make_shared<FirebaseError>(FIREBASE_ERROR_CODES::STREAM_NOT_INITIALIZED, "HTTP stream is not initialized");
    return false;
  }
  if (!stream_http_.get()->connected()) {
    error_ = std::make_shared<FirebaseError>(FIREBASE_ERROR_CODES::HTTP_CONNECTION_LOST, "Connection Lost");
    return false;
  }
  auto client = stream_http_.get()->getStreamPtr();
  return (client == nullptr) ? false : client->available();
}

// void FirebaseArduino::streamEnd() {
//   connectionEnd();
// }

// void FirebaseArduino::reqEnd(){
//   connectionEnd();
// }

void FirebaseArduino::connectionEnd() {
  //ending stream if any
  FBDEBUGF(("[connectionEnd-streamEnd] Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  if (stream_.get() != nullptr) {
    FBDEBUGF(("[connectionEnd-streamEnd] stream_.use_count:%d Free Heap: %u %lu\n"), stream_.use_count(), ESP.getFreeHeap(), micros());
    FBDEBUGF(("[connectionEnd-streamEnd] stream_http_.use_count:%d Free Heap: %u %lu\n"), stream_http_.use_count(), ESP.getFreeHeap(), micros());
    stream_.reset();
    FBDEBUGF(("[connectionEnd-streamEnd] stream_.reset Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  }
  if (stream_http_.get() != nullptr) {
    FBDEBUGF(("[connectionEnd-streamEnd] stream_.use_count:%d Free Heap: %u %lu\n"), stream_.use_count(), ESP.getFreeHeap(), micros());
    FBDEBUGF(("[connectionEnd-streamEnd] stream_http_.use_count:%d Free Heap: %u %lu\n"), stream_http_.use_count(), ESP.getFreeHeap(), micros());
    stream_http_->setReuseConnection(false);
    FBDEBUGF(("[connectionEnd-streamEnd] setReuseConnection Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    //stream_http_->setTimeout(1);
    FBDEBUGF(("[connectionEnd-streamEnd] setTimeout Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    stream_http_.reset();
    FBDEBUGF(("[connectionEnd-streamEnd] stream_http_.reset Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  }
  if (error_.get() != nullptr) {
    FBDEBUGF(("[connectionEnd-streamEnd] error_.use_count:%d Free Heap: %u %lu\n"), error_.use_count(), ESP.getFreeHeap(), micros());
    error_.reset();
  }
  FBDEBUGF(("[connectionEnd-streamEnd] ending stream_.use_count:%d stream_http_.use_count:%d error_.use_count:%d - expecting all 0 Free Heap: %u %lu\n"), stream_.use_count(), stream_http_.use_count(), error_.use_count(), ESP.getFreeHeap(), micros());
  //ending request if any
  FBDEBUGF(("[connectionEnd-reqEnd] Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  if (req_.get() != nullptr) {
    FBDEBUGF(("[connectionEnd-reqEnd] req_.use_count:%d Free Heap: %u %lu\n"), req_.use_count(), ESP.getFreeHeap(), micros());
    FBDEBUGF(("[connectionEnd-reqEnd] req_http_.use_count:%d Free Heap: %u %lu\n"), req_http_.use_count(), ESP.getFreeHeap(), micros());
    req_.reset();
    FBDEBUGF(("[connectionEnd-reqEnd] req_.reset Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  }
  if (req_http_.get() != nullptr) {
    FBDEBUGF(("[connectionEnd-reqEnd] req_.use_count:%d - expecting 0 Free Heap: %u %lu\n"), req_.use_count(), ESP.getFreeHeap(), micros());
    FBDEBUGF(("[connectionEnd-reqEnd] req_http_.use_count:%d - expecting 1 Free Heap: %u %lu\n"), req_http_.use_count(), ESP.getFreeHeap(), micros());
    req_http_->setReuseConnection(false);
    FBDEBUGF(("[connectionEnd-reqEnd] setReuseConnection Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    //req_http_->setTimeout(1);
    FBDEBUGF(("[connectionEnd-reqEnd] setTimeout Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
    req_http_.reset();
    FBDEBUGF(("[connectionEnd-reqEnd] req_http_.reset Free Heap: %u %lu\n"), ESP.getFreeHeap(), micros());
  }
  if (error_.get() != nullptr) {
    FBDEBUGF(("[connectionEnd-reqEnd] error_.use_count:%d Free Heap: %u %lu\n"), error_.use_count(), ESP.getFreeHeap(), micros());
    error_.reset();
  }
  FBDEBUGF(("[connectionEnd-reqEnd] ending req_.use_count:%d req_http_.use_count:%d error_.use_count:%d - expecting all 0 Free Heap: %u %lu\n"), req_.use_count(), req_http_.use_count(), error_.use_count(), ESP.getFreeHeap(), micros());
}

FirebaseObject FirebaseArduino::readEvent() {
  if (stream_http_.get() == nullptr) {
    return FirebaseObject("");
  }
  auto client = stream_http_.get()->getStreamPtr();
  if (client == nullptr) {
    return FirebaseObject("");
  }
  //String type = client->readStringUntil('\n').substring(7);;
  //String event = client->readStringUntil('\n').substring(6);
  String type = client->readStringUntil('\n').substring(7);
  FBDEBUGF("event: %s ", type.c_str());

  String event = client->readStringUntil('\n').substring(6);
  FBDEBUGF("data: %s\n", event.c_str());

  client->readStringUntil('\n');  // consume separator
  FirebaseObject obj = FirebaseObject(const_cast<char*>(event.c_str()));

  // required to have a copy of the string but use a char[] format which is
  // the only supported format for JsonObject#set (it does not like the String of the test env)
  //char *cstr = new char[type.length() + 1];
  //strncpy(cstr, type.c_str(), type.length() + 1);
  //obj.getJsonVariant().as<JsonObject&>().set("type", cstr);
  obj.getJsonVariant().as<JsonObject&>().set("type", type);
  //delete[] cstr;
  return obj;
}

void FirebaseArduino::readEventString(String& eventType, String& data) {
  if (stream_http_.get() == nullptr) {
    return;
  }
  auto client = stream_http_.get()->getStreamPtr();
  if (client == nullptr) {
    return;
  }
  //String type = client->readStringUntil('\n').substring(7);;
  //String event = client->readStringUntil('\n').substring(6);
  eventType = client->readStringUntil('\n').substring(7);
  FBDEBUGF("\nevent: [%s] ", eventType.c_str());

  data = client->readStringUntil('\n').substring(6);
  FBDEBUGF("data: [%s]", data.c_str());

  client->readStringUntil('\n');  // consume separator
  return;
}

bool FirebaseArduino::success() {
  return error_->code() == 200;
}

bool FirebaseArduino::failed() {
  return error_->code() != 200;
}

const String& FirebaseArduino::error() {
  return error_->message().c_str();
}

FirebaseArduino Firebase;
