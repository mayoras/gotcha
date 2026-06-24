//
// Created by evaluator on 24/06/2026.
//

#ifndef GOTCHA_CONFIG_H
#define GOTCHA_CONFIG_H

#include <cstdint>
#include <string>

struct GotchaConfig {
    std::string targetPackageName;
    uint64_t    delayUs;
};

// Reads <moduleDir>/config.prop. Missing file or invalid values fall back to
// defaults (com.example.test, 300000us).
GotchaConfig readConfig(const std::string& moduleDir);

#endif //GOTCHA_CONFIG_H
