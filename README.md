# Gotcha: A Zygisk module for function hooking

Gotcha is a Zygisk module that provides a simple way to hook functions in Android applications. Consists of injecting a shared library into the target process that dynamically modifies the GOT entries of selected target libraries, allowing you to intercept and modify function calls.
