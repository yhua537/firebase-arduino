#ifndef firebase_error_h
#define firebase_error_h

// These error codes are used in addition to regular HTTP error codes.
// Same error space is shared between HTTP errors and these values.
enum FIREBASE_ERROR_CODES {
  HTTP_CONNECTION_LOST = -5,
  STREAM_NOT_INITIALIZED = -6
};

class FirebaseError {
 public:
  // Make it explicit that the empty constructor mean no error.
  static FirebaseError OK() {
    return FirebaseError();
  }

  FirebaseError() {}
  FirebaseError(int code, const String& message) : code_(code), message_(message) {
    //FBDEBUGF(("[FirebaseError] %d - %s - Free Heap: %u %lu\n"), code, message.c_str(), ESP.getFreeHeap(), micros ());
  }

  operator bool() const { return code_ != 0; }
  int code() const { return code_; }
  const String& message() const { return message_; }

 private:
  int code_ = 200;
  String message_ = "";
};

/*

/// HTTP client errors
#define HTTPC_ERROR_CONNECTION_REFUSED  (-1)
#define HTTPC_ERROR_SEND_HEADER_FAILED  (-2)
#define HTTPC_ERROR_SEND_PAYLOAD_FAILED (-3)
#define HTTPC_ERROR_NOT_CONNECTED       (-4)
#define HTTPC_ERROR_CONNECTION_LOST     (-5)
#define HTTPC_ERROR_NO_STREAM           (-6)
#define HTTPC_ERROR_NO_HTTP_SERVER      (-7)
#define HTTPC_ERROR_TOO_LESS_RAM        (-8)
#define HTTPC_ERROR_ENCODING            (-9)
#define HTTPC_ERROR_STREAM_WRITE        (-10)
#define HTTPC_ERROR_READ_TIMEOUT        (-11)

*/
#endif  //firebase_error_h
