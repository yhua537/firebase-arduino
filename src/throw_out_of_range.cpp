#include <Arduino.h>

//TODO(edcoyne): remove this when it is present upstream.
// Currently this is needed to use String on Arduino.
namespace std{
void __attribute__((weak)) __throw_out_of_range(const char* str) {
    panic();
}
}
