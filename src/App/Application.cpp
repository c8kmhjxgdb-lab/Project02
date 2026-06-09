#include "App/Application.h"

#include "App/GameBootstrap.h"
#include "App/GameLoop.h"
#include "Game/GameState.h"

#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <cstdio>

namespace {

constexpr int kInitialWindowWidth = 800;
constexpr int kInitialWindowHeight = 600;

void destroyPlatform(SDL_Window* window, SDL_GLContext glCtx) {
    if (glCtx) {
        SDL_GL_DeleteContext(glCtx);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}

bool configureOpenGLAttributes() {
    return SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) == 0 &&
           SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3) == 0 &&
           SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) == 0 &&
           SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) == 0 &&
           SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24) == 0;
}

void configureInitialRenderState() {
    glViewport(0, 0, kInitialWindowWidth, kInitialWindowHeight);
    glClearColor(0.96f, 0.94f, 0.85f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

}  // namespace

int Application::run() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_StopTextInput();

    if (!configureOpenGLAttributes()) {
        std::fprintf(stderr, "SDL_GL_SetAttribute failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Starchild 2D - Stage 4: Emotion & Princess System",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        kInitialWindowWidth,
        kInitialWindowHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GLContext glCtx = SDL_GL_CreateContext(window);
    if (!glCtx) {
        std::fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        destroyPlatform(window, nullptr);
        return 1;
    }

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::fprintf(stderr, "glewInit failed\n");
        destroyPlatform(window, glCtx);
        return 1;
    }

    configureInitialRenderState();

    GameState gs = {};
    if (!GameBootstrap::initialize(gs, window)) {
        GameBootstrap::shutdown(gs);
        destroyPlatform(window, glCtx);
        return 1;
    }

    GameLoop::run(window, gs);

    int score = gs.score;
    int enemiesKilled = gs.enemiesKilled;

    GameBootstrap::shutdown(gs);
    destroyPlatform(window, glCtx);

    std::printf("Starchild 2D: Stage 3 prototype closed cleanly. Score: %d, Kills: %d\n",
                score, enemiesKilled);
    return 0;
}
