#include "config.h"

#include <cstdlib>
#include <cstdio>

#include <log.h>

static std::string trimWs(const std::string& s) {
    const char* ws = " \t\r\n";
    size_t a = s.find_first_not_of(ws);
    if (a == std::string::npos) return {};
    return s.substr(a, s.find_last_not_of(ws) - a + 1);
}

GotchaConfig readConfig(const std::string& moduleDir) {
    GotchaConfig cfg{ "com.example.test", 300000 };

    std::string path = moduleDir + "/config.prop";
    FILE* f = fopen(path.c_str(), "r");
    if (!f) {
        LOGI("config.prop not found at %s — using defaults", path.c_str());
        return cfg;
    }
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        std::string t = trimWs(std::string(line));
        if (t.empty() || t[0] == '#') continue;
        size_t eq = t.find('=');
        if (eq == std::string::npos) continue;
        std::string key = trimWs(t.substr(0, eq));
        std::string val = trimWs(t.substr(eq + 1));
        if (key == "target_package" && !val.empty()) {
            cfg.targetPackageName = val;
        } else if (key == "delay_us") {
            char* end = nullptr;
            uint64_t v = strtoull(val.c_str(), &end, 10);
            if (end != val.c_str() && *end == '\0')
                cfg.delayUs = v;
            else
                LOGW("config: invalid delay_us '%s', using default", val.c_str());
        } else {
            LOGW("config: unknown key '%s', ignoring", key.c_str());
        }
    }
    fclose(f);
    return cfg;
}
