#ifndef MODEM_JSON_UTIL_H
#define MODEM_JSON_UTIL_H

namespace firebase {
namespace modem {
namespace {
String ReplaceAll(String str, const String& from, const String& to) {
  size_t start_pos = 0;
  while((start_pos = str.find(from, start_pos)) != String::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
  }
  return str;
}
}

// TODO(edcoyne): We should use a json library to escape.
inline String EncodeForJson(String input) {
  input = ReplaceAll(input, "\\", "\\\\");
  input = ReplaceAll(input, "\"", "\\\"");
  return "\"" + input + "\"";
}

}  // modem
}  // firebase

#endif  // MODEM_JSON_UTIL_H
