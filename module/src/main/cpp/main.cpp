/* Copyright 2022-2023 John "topjohnwu" Wu
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <android/log.h>
#include <string>
#include <thread>
#include <vector>
#include <filesystem>
#include <regex>

#include <log.h>
#include "zygisk.hpp"

#define BUFFER_SIZE 1024

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

static std::string recvString(int fd) {
    size_t len;
    read(fd, &len, sizeof(len));
    std::vector<char> buffer(len);
    read(fd, buffer.data(), len);
    return buffer.data();
}

static oid sendString(int fd, const std::string &str) {
    size_t len = str.size() + 1;
    write(fd, &len, sizeof(len));
    write(fd, str.c_str(), len);
}

namespace fs = std::filesystem;
static std::string findMatchingFile(const fs::path& directory, const std::regex& pattern) {
    for (const auto& entry : fs::directory_iterator(directory)) {
        const auto& path = entry.path();
        const auto& filename = path.filename().string();

        if (std::regex_search(filename, pattern)) {
            return filename;
        }
    }
    return ""; // Return an empty string if no match is found
}

static std::string getPathFromFd(int fd) {
    char buf[PATH_MAX];
    std::string fdPath = "/proc/self/fd/" + std::to_string(fd);
    ssize_t len = readlink(fdPath.c_str(), buf, sizeof(buf) - 1);
    close(fd);
    if (len != -1) {
        buf[len] = '\0';
        return {buf};
    } else {
        // Handle error
        return "";
    }
}

static void copyFile(const char *src, const char *dst) {
    FILE *fsrc, *fdst;

    fsrc = fopen(src, "rb");
    if (fsrc == nullptr) {
        LOGE("Failed to open source file: %s", src);
        goto finally;
    }

    fdst = fopen(dst, "wb");
    if (fdst == nullptr) {
        LOGE("Failed to open destination file: %s", dst);
        goto finally;
    }

    // 1 KiB write
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, fsrc)) > 0) {
        if (fwrite(buffer, 1, bytes_read, fdst) != bytes_read) {
            LOGE("Failed to write to destination file: %s", dst);
            goto finally;
        }
    }

    finally:
    {
        if (fsrc != nullptr)
            fclose(fsrc);
        if (fdst != nullptr)
            fclose(fdst);
    }
}

static void injection_fn(const char *packageName, const char *library_path, uint64_t delay) {
    LOGD("Gotcha library injection thread started for %s, library name: %s, usleep: %lu",
         packageName,
         library_path,
         delay);

    // sleep for delay to ensure app is started and library is in place
    usleep(delay);
}

class EntryPoint : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        if (!args || !args->nice_name) {
            LOGE("Skip unknown process");
            return;
        }

        // Use JNI to fetch our process name
        const char *process_name = env->GetStringUTFChars(args->nice_name, nullptr);

        // get the module file path
        std::string module_path = getPathFromFd(api->getModuleDir());
        LOGD("module_path=[%s]\n", module_path.c_str());
        sendString(api->connectCompanion(), module_path);

        int fd = api->connectCompanion();

        std::string targetPackageName = recvString(fd);

        if (strcmp(process_name, targetPackageName.c_str()) == 0) {
            LOGD("Enable gotcha for process=[%s]\n", process_name);
            _enableGotcha = true;
            write(fd, &_enableGotcha, sizeof(_enableGotcha));

            _targetPackageName = strdup(targetPackageName.c_str());

            uint64_t delay;
            read(fd, &delay, sizeof(delay));
            LOGD("delay=[%lu]\n", delay);
            _delay = delay;

            std::string gotchaLibraryPath = recvString(fd);
            LOGD("gotchaLibraryPath=[%s]\n", gotchaLibraryPath.c_str());
            _gotchaLibraryPath = strdup(gotchaLibraryPath.c_str());

            close(fd);
        } else {
            // no target app, just close and return
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
            close(fd);
        }

        env->ReleaseStringUTFChars(args->nice_name, process_name);
    }

    void postAppSpecialize(const AppSpecializeArgs *args) override {
        if (_enableGotcha) {
            std::thread t(injection_fn, _targetPackageName, _gotchaLibraryPath, _delay);
            t.detach();
        }
    }

private:
    Api *api{};
    JNIEnv *env{};
    bool _enableGotcha = false;
    char *_targetPackageName{};
    char *_gotchaLibraryPath{};
    uint64_t _delay{};
};

static void companion_handler(int i) {
    std::string moduleDir = recvString(i);

    // TODO: make this configurable with webui
    std::string targetPackageName = "com.example.test";
    uint64_t delay = 300000;

    // send target package name
    sendString(i, targetPackageName);

    // check if gotcha is enabled for the target package
    bool enableGotcha;
    read(i, &enableGotcha, sizeof(enableGotcha));
    if (!enableGotcha)
        return;

    // send delay
    write(i, &delay, sizeof(delay));

#if defined(__arm__)
    std::regex gotchaLibraryPattern(".*gotcha.*arm\\.so$");
#elif defined(__aarch64__)
    std::regex gotchaLibraryPattern(".*gotcha.*arm64\\.so$");
#elif defined(__i386__)
    std::regex gotchaLibraryPattern(".*gotcha.*x86\\.so$");
#elif defined(__x86_64__)
    std::regex gotchaLibraryPattern(".*gotcha.*x86_64\\.so$");
#else
    std::regex gotchaLibraryPattern(".*gotcha.*arm64\\.so$");
#endif
    std::string gotchaLibraryName = findMatchingFile(moduleDir, gotchaLibraryPattern);
    if (gotchaLibraryName.empty()) {
        LOGE("No gotcha library found in module dir");
        return;
    }
    sendString(i, gotchaLibraryName);
    std::string gotchaLibraryPath = moduleDir + "/" + gotchaLibraryName;

    // copy gotcha library to target process
    std::string src = gotchaLibraryPath;
    std::string dst = "/data/data/" + targetPackageName + "/" + gotchaLibraryName;
    LOGD("Copying gotcha library from %s to %s", src.c_str(), dst.c_str());
    copyFile(src.c_str(), dst.c_str());
}

// Register our module class and the companion handler function
REGISTER_ZYGISK_MODULE(EntryPoint)
REGISTER_ZYGISK_COMPANION(companion_handler)
