#pragma once

#include <GL/glew.h>
#include <cstdio>
#include <string>

/**
 * ShaderUtils - Shared shader loading and compilation utilities.
 * Eliminates duplication between main.cpp and DecorRenderer.cpp.
 */

inline std::string loadShaderFile(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "ShaderUtils: failed to open %s\n", path);
        return "";
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::string src;
    src.resize(static_cast<size_t>(len));
    fread(src.data(), 1, static_cast<size_t>(len), f);
    fclose(f);
    return src;
}

inline GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        fprintf(stderr, "Shader compile error: %s\n", log);
        return 0;
    }
    return shader;
}

inline GLuint createShaderProgram(const char* vertPath, const char* fragPath) {
    std::string vertSrc = loadShaderFile(vertPath);
    std::string fragSrc = loadShaderFile(fragPath);
    if (vertSrc.empty() || fragSrc.empty()) {
        fprintf(stderr, "Failed to load shader files: %s / %s\n", vertPath, fragPath);
        return 0;
    }
    GLuint vert = compileShader(GL_VERTEX_SHADER, vertSrc.c_str());
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc.c_str());
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);
    GLint ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(prog, 512, nullptr, log);
        fprintf(stderr, "Program link error: %s\n", log);
        glDeleteProgram(prog);
        return 0;
    }
    glDeleteShader(vert);
    glDeleteShader(frag);
    return prog;
}
