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

// firebase-arduino is an Arduino client for Firebase.
// It is currently limited to the ESP8266 board family.

#ifndef firebase_h
#define firebase_h

//#include "WString.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <memory>

#include "FirebaseError.h"
#include "FirebaseHttpClient.h"
#include "FirebaseObject.h"

// Firebase REST API client.
class Firebaseunused {
 public:
  Firebaseunused(const String&& host, const String&& auth = "");

  const String& auth() const;

  // Fetch json encoded `value` at `path`.
  void get(const String& path);

  // Set json encoded `value` at `path`.
  void set(const String& path, const String& json);

  // Add new json encoded `value` to list at `path`.
  void push(const String& path, const String& json);

  // Delete value at `path`.
  void remove(const String& path);

  // Start a stream of events that affect value at `path`.
  void stream(const String& path);

 protected:
  // Used for testing.
  Firebaseunused() {}

 private:
  std::shared_ptr<FirebaseHttpClient> http_;
  String host_;
  String auth_;
};

class FirebaseCall {
 public:
  FirebaseCall(const std::shared_ptr<FirebaseHttpClient> http = NULL) { http_ = http; }
  virtual ~FirebaseCall();

  const std::shared_ptr<FirebaseError> error() const {
    return error_;
  }

  boolean analyzeError(const String& method, int status, const String& path_with_auth);

  const String& response() const {
    return response_;
  }

  //  const void end();

  const JsonObject& json();
  int refreshLogin(const String& host, const String& path,
                   const String& user, const String& password,
                   String& refreshToken, String& auth);

  int refreshToken(const String& host, const String& path, const String& key,
                   String& refreshToken, String& auth);

 protected:
  std::shared_ptr<FirebaseHttpClient> http_;
  std::shared_ptr<FirebaseError> error_;
  String response_;
  std::shared_ptr<StaticJsonBuffer<FIREBASE_JSONBUFFER_SIZE>> buffer_;
};

class FirebaseRequest : public FirebaseCall {
 public:
  FirebaseRequest(const std::shared_ptr<FirebaseHttpClient> http = NULL) : FirebaseCall(http) {}
  virtual ~FirebaseRequest() {}
  int sendRequest(const String& host, const String& auth,
                  const String& method, const String& path, const String& data = "");
};

class FirebaseStream : public FirebaseCall {
 public:
  FirebaseStream(const std::shared_ptr<FirebaseHttpClient> http = NULL) : FirebaseCall(http) {}
  virtual ~FirebaseStream() {}

  bool startStreaming(const String& host, const String& auth, const String& path);
};

#endif  // firebase_h
