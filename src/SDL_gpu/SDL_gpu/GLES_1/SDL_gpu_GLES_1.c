#include "SDL_gpu_GLES_1.h"

#if defined(SDL_GPU_DISABLE_GLES) || defined(SDL_GPU_DISABLE_GLES_1)

// Dummy implementations
GPU_Renderer* GPU_CreateRenderer_GLES_1(GPU_RendererID request) {return NULL;}
void GPU_FreeRenderer_GLES_1(GPU_Renderer* renderer) {}

#else


// Most of the code pulled in from here...
#define SDL_GPU_USE_GLES
#define SDL_GPU_USE_GL_TIER2
#define SDL_GPU_GL_TIER 2
#define SDL_GPU_GLES_MAJOR_VERSION 1
#define SDL_GPU_DISABLE_SHADERS
#define SDL_GPU_APPLY_TRANSFORMS_TO_GL_STACK
#define SDL_GPU_NO_VAO
#include "../GL_common/SDL_gpu_GL_common.inl"
#include "../GL_common/SDL_gpuShapes_GL_common.inl"


GPU_Renderer* GPU_CreateRenderer_GLES_1(GPU_RendererID request)
{
    GPU_Renderer* renderer = (GPU_Renderer*)malloc(sizeof(GPU_Renderer));
    if(renderer == NULL)
        return NULL;

    memset(renderer, 0, sizeof(GPU_Renderer));

    renderer->id = request;
    renderer->id.id = GPU_RENDERER_GLES_1;
    renderer->shader_language = GPU_LANGUAGE_NONE;
    renderer->shader_version = 0;
    
    renderer->current_context_target = NULL;

    SET_COMMON_FUNCTIONS(renderer);

    return renderer;
}

void GPU_FreeRenderer_GLES_1(GPU_Renderer* renderer)
{
    if(renderer == NULL)
        return;

    free(renderer);
}

#endif
