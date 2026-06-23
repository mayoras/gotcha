#include <log.h>
#include "gadget.h"


__attribute__((constructor))
void onLoad() {
    LOGD("Gotcha gadget loaded.");
}