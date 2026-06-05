#include "LuaVM.h"
#include <cstdio>

// Exception handler with correct signature for sol2
static int luaExceptionHandler(lua_State* L, sol::optional<const std::exception&> maybe_ex, sol::string_view description) {
    if (maybe_ex) {
        fprintf(stderr, "[Lua Error] %s\n", maybe_ex.value().what());
    } else {
        fprintf(stderr, "[Lua Error] %.*s\n", static_cast<int>(description.size()), description.data());
    }
    return 0;
}

LuaVM::LuaVM() : hasErrorFlag(false) {}

LuaVM::~LuaVM() = default;

bool LuaVM::init() {
    lua = std::make_unique<sol::state>();
    lua->open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::string);

    // 绑定C++ API到Lua
    bindGameAPI();

    // 设置错误处理
    lua->set_exception_handler(luaExceptionHandler);

    return true;
}

void LuaVM::bindGameAPI() {
    if (!lua) return;

    auto& s = *lua;

    // 数学工具函数
    s.set_function("dist", [](float x1, float y1, float x2, float y2) {
        float dx = x2 - x1;
        float dy = y2 - y1;
        return sqrtf(dx * dx + dy * dy);
    });

    s.set_function("normalize", [&s](float x, float y) {
        float len = sqrtf(x * x + y * y);
        if (len < 0.0001f) {
            auto tbl = s.create_table();
            tbl.set("x", 1.0f);
            tbl.set("y", 0.0f);
            return tbl;
        }
        auto tbl = s.create_table();
        tbl.set("x", x / len);
        tbl.set("y", y / len);
        return tbl;
    });

    s.set_function("lerp", [](float a, float b, float t) {
        return a + (b - a) * t;
    });

    // 调试输出
    s.set_function("printLog", [](const char* msg) {
        printf("[Lua] %s\n", msg);
    });
}

bool LuaVM::loadFile(const char* path) {
    if (!lua) return false;

    // Clear error flag on new load attempt
    hasErrorFlag = false;

    sol::protected_function_result result = lua->safe_script_file(path);
    if (!result.valid()) {
        sol::error err = result;
        errorHandler(err.what());
        return false;
    }

    return true;
}

bool LuaVM::doString(const char* code) {
    if (!lua) return false;

    // Clear error flag on new execution attempt
    hasErrorFlag = false;

    sol::protected_function_result result = lua->safe_script(code);
    if (!result.valid()) {
        sol::error err = result;
        errorHandler(err.what());
        return false;
    }

    return true;
}

void LuaVM::errorHandler(const std::string& msg) {
    hasErrorFlag = true;
    lastError = msg;
    fprintf(stderr, "[Lua Error] %s\n", msg.c_str());
}
