#include "SDL_gpu_OpenGL_1.h"


#if defined(SDL_GPU_DISABLE_OPENGL) || defined(SDL_GPU_DISABLE_OPENGL_1)

// Dummy implementations
GPU_Renderer* GPU_CreateRenderer_OpenGL_1(GPU_RendererID request) {return NULL;}
void GPU_FreeRenderer_OpenGL_1(GPU_Renderer* renderer) {}

#else

// Most of the code pulled in from here...
#define SDL_GPU_USE_OPENGL
#define SDL_GPU_USE_GL_TIER3
#define SDL_GPU_GL_TIER 3
#define SDL_GPU_GLSL_VERSION 110
#define SDL_GPU_GL_MAJOR_VERSION 1
#define SDL_GPU_NO_VAO
#include "../GL_common/SDL_gpu_GL_common.inl"
#include "../GL_common/SDL_gpuShapes_GL_common.inl"


GPU_Renderer* GPU_CreateRenderer_OpenGL_1(GPU_RendererID request)
{
    GPU_Renderer* renderer = (GPU_Renderer*)malloc(sizeof(GPU_Renderer));
    if(renderer == NULL)
        return NULL;

    memset(renderer, 0, sizeof(GPU_Renderer));

    renderer->id = request;
    renderer->id.id = GPU_RENDERER_OPENGL_1;
    renderer->shader_language = GPU_LANGUAGE_GLSL;
    renderer->shader_version = SDL_GPU_GLSL_VERSION;
    
    renderer->current_context_target = NULL;

    SET_COMMON_FUNCTIONS(renderer);

    return renderer;
}

void GPU_FreeRenderer_OpenGL_1(GPU_Renderer* renderer)
{
    if(renderer == NULL)
        return;

    free(renderer);
}

#endif
