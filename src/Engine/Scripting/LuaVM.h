#pragma once

#include <sol/sol.hpp>
#include <functional>
#include <string>
#include <memory>

/**
 * LuaVM - Lua脚本虚拟机
 *
 * 封装sol2状态，提供游戏脚本的加载、执行和C++/Lua互调。
 */
class LuaVM {
public:
    LuaVM();
    ~LuaVM();

    // 初始化（打开常用库，绑定游戏API）
    bool init();

    // 加载并执行Lua文件
    bool loadFile(const char* path);

    // 执行Lua代码字符串
    bool doString(const char* code);

    // 获取sol::state引用（用于高级操作）
    sol::state& state() { return *lua; }

    // 调用Lua函数（返回值可选）
    template<typename... Args>
    auto call(const char* funcName, Args&&... args);

    // 设置全局变量
    template<typename T>
    void setGlobal(const char* name, T&& value);

    // 注册C++函数到Lua
    template<typename Func>
    void setFunction(const char* name, Func&& func);

    // 错误处理
    bool hasError() const { return hasErrorFlag; }
    const std::string& getLastError() const { return lastError; }

    // 清除错误标志，允许重新加载脚本
    void clearError() { hasErrorFlag = false; lastError.clear(); }

private:
    std::unique_ptr<sol::state> lua;
    bool hasErrorFlag;
    std::string lastError;

    // 绑定游戏API到Lua
    void bindGameAPI();

    // 错误处理回调
    void errorHandler(const std::string& msg);
};

// Template implementations

// Look up a previously-defined Lua function by name and call it.
// Note: the Lua state must own the function as a global or upvalue before this is called.
// To call a function loaded by loadFile(), the file must assign it to _G (e.g. `function foo() ... end`).
template<typename... Args>
auto LuaVM::call(const char* funcName, Args&&... args) {
    if (!lua || hasErrorFlag) {
        return sol::make_object(lua ? lua->lua_state() : nullptr, sol::nil);
    }

    // Properly look up the function instead of treating funcName as code to execute.
    sol::protected_function func = lua->get<sol::protected_function>(funcName);
    if (!func.valid()) {
        errorHandler("Function not found: " + std::string(funcName));
        return sol::make_object(lua->lua_state(), sol::nil);
    }

    sol::protected_function_result result = func(std::forward<Args>(args)...);
    if (!result.valid()) {
        sol::protected_function_result::handler_type err = result;
        errorHandler(err.what());
        return sol::make_object(lua->lua_state(), sol::nil);
    }

    return result;
}

template<typename T>
void LuaVM::setGlobal(const char* name, T&& value) {
    if (lua) {
        lua->set(name, std::forward<T>(value));
    }
}

template<typename Func>
void LuaVM::setFunction(const char* name, Func&& func) {
    if (lua) {
        lua->set_function(name, std::forward<Func>(func));
    }
}
