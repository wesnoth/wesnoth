#ifndef _SDL_GPU_H__
#define _SDL_GPU_H__

#include "SDL.h"
#include <stdio.h>
#include <stdarg.h>


#ifdef __cplusplus
extern "C" {
#endif

// Compile-time versions
#define SDL_GPU_VERSION_MAJOR 0
#define SDL_GPU_VERSION_MINOR 9
#define SDL_GPU_VERSION_PATCH 0

/* Auto-detect if we're using the SDL2 API by the headers available. */
#if SDL_VERSION_ATLEAST(2,0,0)
    #define SDL_GPU_USE_SDL2
#endif

typedef struct GPU_Renderer GPU_Renderer;
typedef struct GPU_Target GPU_Target;

/*! A struct representing a rectangular area with floating point precision.
 * \see GPU_MakeRect() 
 */
typedef struct GPU_Rect
{
    float x, y;
    float w, h;
} GPU_Rect;

#define GPU_RENDERER_ORDER_MAX 10

typedef Uint32 GPU_RendererEnum;
static const GPU_RendererEnum GPU_RENDERER_UNKNOWN = 0x0;  // invalid value
static const GPU_RendererEnum GPU_RENDERER_OPENGL_1_BASE = 0x1;
static const GPU_RendererEnum GPU_RENDERER_OPENGL_1 = 0x2;
static const GPU_RendererEnum GPU_RENDERER_OPENGL_2 = 0x4;
static const GPU_RendererEnum GPU_RENDERER_OPENGL_3 = 0x8;
static const GPU_RendererEnum GPU_RENDERER_OPENGL_4 = 0x10;
static const GPU_RendererEnum GPU_RENDERER_GLES_1 = 0x100;
static const GPU_RendererEnum GPU_RENDERER_GLES_2 = 0x200;
static const GPU_RendererEnum GPU_RENDERER_GLES_3 = 0x400;
static const GPU_RendererEnum GPU_RENDERER_D3D9 = 0x10000;
static const GPU_RendererEnum GPU_RENDERER_D3D10 = 0x20000;
static const GPU_RendererEnum GPU_RENDERER_D3D11 = 0x40000;

/*! Renderer ID object for identifying a specific renderer.
 * \see GPU_MakeRendererID()
 * \see GPU_InitRendererByID()
 */
typedef struct GPU_RendererID
{
    GPU_RendererEnum id;
    int major_version;
    int minor_version;
    
    int index;
} GPU_RendererID;


/*! Blend component functions
 * \see GPU_SetBlendFunction()
 * Values chosen for direct OpenGL compatibility.
 */
typedef enum {
    GPU_FUNC_ZERO = 0,
    GPU_FUNC_ONE = 1,
    GPU_FUNC_SRC_COLOR = 0x0300,
    GPU_FUNC_DST_COLOR = 0x0306,
    GPU_FUNC_ONE_MINUS_SRC = 0x0301,
    GPU_FUNC_ONE_MINUS_DST = 0x0307,
    GPU_FUNC_SRC_ALPHA = 0x0302,
    GPU_FUNC_DST_ALPHA = 0x0304,
    GPU_FUNC_ONE_MINUS_SRC_ALPHA = 0x0303,
    GPU_FUNC_ONE_MINUS_DST_ALPHA = 0x0305
} GPU_BlendFuncEnum;

/*! Blend component equations
 * \see GPU_SetBlendEquation()
 * Values chosen for direct OpenGL compatibility.
 */
typedef enum {
    GPU_EQ_ADD = 0x8006,
    GPU_EQ_SUBTRACT = 0x800A,
    GPU_EQ_REVERSE_SUBTRACT = 0x800B
} GPU_BlendEqEnum;

/*! Blend mode storage struct */
typedef struct GPU_BlendMode
{
    GPU_BlendFuncEnum source_color;
    GPU_BlendFuncEnum dest_color;
    GPU_BlendFuncEnum source_alpha;
    GPU_BlendFuncEnum dest_alpha;
    
    GPU_BlendEqEnum color_equation;
    GPU_BlendEqEnum alpha_equation;
} GPU_BlendMode;

/*! Blend mode presets 
 * \see GPU_SetBlendMode()
 * \see GPU_GetBlendModeFromPreset()
 */
typedef enum {
    GPU_BLEND_NORMAL = 0,
    GPU_BLEND_PREMULTIPLIED_ALPHA = 1,
    GPU_BLEND_MULTIPLY = 2,
    GPU_BLEND_ADD = 3,
    GPU_BLEND_SUBTRACT = 4,
    GPU_BLEND_MOD_ALPHA = 5,
    GPU_BLEND_SET_ALPHA = 6,
    GPU_BLEND_SET = 7,
    GPU_BLEND_NORMAL_KEEP_ALPHA = 8,
    GPU_BLEND_NORMAL_ADD_ALPHA = 9
} GPU_BlendPresetEnum;

/*! Image filtering options.  These affect the quality/interpolation of colors when images are scaled. 
 * \see GPU_SetImageFilter()
 */
typedef enum {
    GPU_FILTER_NEAREST = 0,
    GPU_FILTER_LINEAR = 1,
    GPU_FILTER_LINEAR_MIPMAP = 2
} GPU_FilterEnum;

/*! Snap modes.  Blitting with these modes will align the sprite with the target's pixel grid.
 * \see GPU_SetSnapMode()
 * \see GPU_GetSnapMode()
 */
typedef enum {
    GPU_SNAP_NONE = 0,
    GPU_SNAP_POSITION = 1,
    GPU_SNAP_DIMENSIONS = 2,
    GPU_SNAP_POSITION_AND_DIMENSIONS = 3
} GPU_SnapEnum;


/*! Image wrapping options.  These affect how images handle src_rect coordinates beyond their dimensions when blitted.
 * \see GPU_SetWrapMode()
 */
typedef enum {
    GPU_WRAP_NONE = 0,
    GPU_WRAP_REPEAT = 1,
    GPU_WRAP_MIRRORED = 2
} GPU_WrapEnum;

/*! Image format enum
 * \see GPU_CreateImage()
 */
typedef enum {
    GPU_FORMAT_LUMINANCE = 1,
    GPU_FORMAT_LUMINANCE_ALPHA = 2,
    GPU_FORMAT_RGB = 3,
    GPU_FORMAT_RGBA = 4,
    GPU_FORMAT_ALPHA = 5,
    GPU_FORMAT_RG = 6,
    GPU_FORMAT_YCbCr422 = 7,
    GPU_FORMAT_YCbCr420P = 8
} GPU_FormatEnum;



/*! Image object for containing pixel/texture data.
 * A GPU_Image can be created with GPU_CreateImage(), GPU_LoadImage(), GPU_CopyImage(), or GPU_CopyImageFromSurface().
 * Free the memory with GPU_FreeImage() when you're done.
 * \see GPU_CreateImage()
 * \see GPU_LoadImage()
 * \see GPU_CopyImage()
 * \see GPU_CopyImageFromSurface()
 * \see GPU_Target
 */
typedef struct GPU_Image
{
	struct GPU_Renderer* renderer;
	GPU_Target* target;
	Uint16 w, h;
	GPU_FormatEnum format;
	int num_layers;
	int bytes_per_pixel;
	Uint32 texture_w, texture_h;  // Underlying texture dimensions
	Uint8 has_mipmaps;
	
	SDL_Color color;
	Uint8 use_blending;
	GPU_BlendMode blend_mode;
	GPU_FilterEnum filter_mode;
	GPU_SnapEnum snap_mode;
	GPU_WrapEnum wrap_mode_x;
	GPU_WrapEnum wrap_mode_y;
	
	void* data;
	int refcount;
	Uint8 is_alias;
} GPU_Image;


/*! Camera object that determines viewing transform.
 * \see GPU_SetCamera() 
 * \see GPU_GetDefaultCamera() 
 * \see GPU_GetCamera()
 */
typedef struct GPU_Camera
{
	float x, y, z;
	float angle;
	float zoom;
} GPU_Camera;


/*! Container for the built-in shader attribute and uniform locations (indices).
 * \see GPU_LoadShaderBlock()
 * \see GPU_SetShaderBlock()
 */
typedef struct GPU_ShaderBlock
{
    // Attributes
    int position_loc;
    int texcoord_loc;
    int color_loc;
    // Uniforms
    int modelViewProjection_loc;
} GPU_ShaderBlock;




#ifndef GPU_MATRIX_STACK_MAX
#define GPU_MATRIX_STACK_MAX 5
#endif

/*! Matrix stack data structure for replacing the old OpenGL matrix stack.  */
typedef struct GPU_MatrixStack
{
    unsigned int size;
    float matrix[GPU_MATRIX_STACK_MAX][16];
} GPU_MatrixStack;


/*! Rendering context data.  Only GPU_Targets which represent windows will store this. */
typedef struct GPU_Context
{
    /*! SDL_GLContext */
    void* context;
    Uint8 failed;
    
    /*! SDL window ID */
	Uint32 windowID;
	
	/*! Actual window dimensions */
	int window_w;
	int window_h;
	
	/*! Window dimensions for restoring windowed mode after GPU_ToggleFullscreen(1). */
	int stored_window_w;
	int stored_window_h;
	
	/*! Internal state */
	Uint32 current_shader_program;
	Uint32 default_textured_shader_program;
	Uint32 default_untextured_shader_program;
	
	Uint8 shapes_use_blending;
	GPU_BlendMode shapes_blend_mode;
	float line_thickness;
	Uint8 use_texturing;
	
    int matrix_mode;
    GPU_MatrixStack projection_matrix;
    GPU_MatrixStack modelview_matrix;
	
	void* data;
} GPU_Context;


/*! Render target object for use as a blitting destination.
 * A GPU_Target can be created from a GPU_Image with GPU_LoadTarget().
 * A GPU_Target can also represent a separate window with GPU_CreateTargetFromWindow().  In that case, 'context' is allocated and filled in.
 * Note: You must have passed the SDL_WINDOW_OPENGL flag to SDL_CreateWindow() for OpenGL renderers to work with new windows.
 * Free the memory with GPU_FreeTarget() when you're done.
 * \see GPU_LoadTarget()
 * \see GPU_CreateTargetFromWindow()
 * \see GPU_FreeTarget()
 */
struct GPU_Target
{
	struct GPU_Renderer* renderer;
	GPU_Image* image;
	void* data;
	Uint16 w, h;
	Uint8 use_clip_rect;
	GPU_Rect clip_rect;
	Uint8 use_color;
	SDL_Color color;
	
	GPU_Rect viewport;
	
	/*! Perspective and object viewing transforms. */
	GPU_Camera camera;
	
	/*! Renderer context data.  NULL if the target does not represent a window or rendering context. */
	GPU_Context* context;
	int refcount;
	Uint8 is_alias;
};

/*! Important GPU features which may not be supported depending on a device's extension support.  Can be OR'd together.
 * \see GPU_IsFeatureEnabled()
 * \see GPU_SetPreInitFlags()
 * \see GPU_GetPreInitFlags()
 */
typedef Uint32 GPU_FeatureEnum;
static const GPU_FeatureEnum GPU_FEATURE_NON_POWER_OF_TWO = 0x1;
static const GPU_FeatureEnum GPU_FEATURE_RENDER_TARGETS = 0x2;
static const GPU_FeatureEnum GPU_FEATURE_BLEND_EQUATIONS = 0x4;
static const GPU_FeatureEnum GPU_FEATURE_BLEND_FUNC_SEPARATE = 0x8;
static const GPU_FeatureEnum GPU_FEATURE_BLEND_EQUATIONS_SEPARATE = 0x10;
static const GPU_FeatureEnum GPU_FEATURE_GL_BGR = 0x20;
static const GPU_FeatureEnum GPU_FEATURE_GL_BGRA = 0x40;
static const GPU_FeatureEnum GPU_FEATURE_GL_ABGR = 0x80;
static const GPU_FeatureEnum GPU_FEATURE_VERTEX_SHADER = 0x100;
static const GPU_FeatureEnum GPU_FEATURE_FRAGMENT_SHADER = 0x200;
static const GPU_FeatureEnum GPU_FEATURE_PIXEL_SHADER = 0x200;
static const GPU_FeatureEnum GPU_FEATURE_GEOMETRY_SHADER = 0x400;
static const GPU_FeatureEnum GPU_FEATURE_WRAP_REPEAT_MIRRORED = 0x800;

/*! Combined feature flags */
#define GPU_FEATURE_ALL_BASE GPU_FEATURE_RENDER_TARGETS
#define GPU_FEATURE_ALL_BLEND_PRESETS (GPU_FEATURE_BLEND_EQUATIONS | GPU_FEATURE_BLEND_FUNC_SEPARATE)
#define GPU_FEATURE_ALL_GL_FORMATS (GPU_FEATURE_GL_BGR | GPU_FEATURE_GL_BGRA | GPU_FEATURE_GL_ABGR)
#define GPU_FEATURE_BASIC_SHADERS (GPU_FEATURE_FRAGMENT_SHADER | GPU_FEATURE_PIXEL_SHADER)
#define GPU_FEATURE_ALL_SHADERS (GPU_FEATURE_FRAGMENT_SHADER | GPU_FEATURE_PIXEL_SHADER | GPU_FEATURE_GEOMETRY_SHADER)

/*! For separating combined feature flags from init flags. */
#define GPU_FEATURE_MASK 0x00FFFF
#define GPU_INIT_MASK 0xFF0000

typedef Uint32 GPU_WindowFlagEnum;

/*! Initialization flags for changing default init parameters.  Can be bitwise OR'ed together with GPU_FeatureEnums.
 * Default (0) is to use late swap vsync and double buffering.
 * \see GPU_SetPreInitFlags()
 * \see GPU_GetPreInitFlags()
 */
typedef Uint32 GPU_InitFlagEnum;
static const GPU_InitFlagEnum GPU_INIT_ENABLE_VSYNC = 0x10000;
static const GPU_InitFlagEnum GPU_INIT_DISABLE_VSYNC = 0x20000;
static const GPU_InitFlagEnum GPU_INIT_DISABLE_DOUBLE_BUFFER = 0x40000;

#define GPU_DEFAULT_INIT_FLAGS 0


static const Uint32 GPU_NONE = 0x0;

/*! Bit flags for the blit batch functions.
 * \see GPU_BlitBatch()
 * \see GPU_BlitBatchSeparate()
 */
typedef Uint32 GPU_BlitFlagEnum;
static const GPU_BlitFlagEnum GPU_PASSTHROUGH_VERTICES = 0x1;
static const GPU_BlitFlagEnum GPU_PASSTHROUGH_TEXCOORDS = 0x2;
static const GPU_BlitFlagEnum GPU_PASSTHROUGH_COLORS = 0x4;
static const GPU_BlitFlagEnum GPU_USE_DEFAULT_POSITIONS = 0x8;
static const GPU_BlitFlagEnum GPU_USE_DEFAULT_SRC_RECTS = 0x10;
static const GPU_BlitFlagEnum GPU_USE_DEFAULT_COLORS = 0x20;

#define GPU_PASSTHROUGH_ALL (GPU_PASSTHROUGH_VERTICES | GPU_PASSTHROUGH_TEXCOORDS | GPU_PASSTHROUGH_COLORS)

/*! Type enumeration for GPU_AttributeFormat specifications. */
typedef Uint32 GPU_TypeEnum;
// Use OpenGL's values for simpler translation
static const GPU_TypeEnum GPU_TYPE_BYTE = 0x1400;
static const GPU_TypeEnum GPU_TYPE_UNSIGNED_BYTE = 0x1401;
static const GPU_TypeEnum GPU_TYPE_SHORT = 0x1402;
static const GPU_TypeEnum GPU_TYPE_UNSIGNED_SHORT = 0x1403;
static const GPU_TypeEnum GPU_TYPE_INT = 0x1404;
static const GPU_TypeEnum GPU_TYPE_UNSIGNED_INT = 0x1405;
static const GPU_TypeEnum GPU_TYPE_FLOAT = 0x1406;
static const GPU_TypeEnum GPU_TYPE_DOUBLE = 0x140A;






/*! Shader type enum.
 * \see GPU_LoadShader()
 * \see GPU_CompileShader()
 * \see GPU_CompileShader_RW()
 */
typedef enum {
    GPU_VERTEX_SHADER = 0,
    GPU_FRAGMENT_SHADER = 1,
    GPU_PIXEL_SHADER = 1,
    GPU_GEOMETRY_SHADER = 2
} GPU_ShaderEnum;



/*! Type enumeration for the shader language used by the renderer. */
typedef enum {
    GPU_LANGUAGE_NONE = 0,
    GPU_LANGUAGE_ARB_ASSEMBLY = 1,
    GPU_LANGUAGE_GLSL = 2,
    GPU_LANGUAGE_GLSLES = 3,
    GPU_LANGUAGE_HLSL = 4,
    GPU_LANGUAGE_CG = 5
} GPU_ShaderLanguageEnum;

typedef struct GPU_AttributeFormat
{
    Uint8 is_per_sprite;  // Per-sprite values are expanded to 4 vertices
    int num_elems_per_value;
    GPU_TypeEnum type;  // GPU_TYPE_FLOAT, GPU_TYPE_INT, GPU_TYPE_UNSIGNED_INT, etc.
    Uint8 normalize;
    int stride_bytes;  // Number of bytes between two vertex specifications
    int offset_bytes;  // Number of bytes to skip at the beginning of 'values'
} GPU_AttributeFormat;

typedef struct GPU_Attribute
{
    int location;
    void* values;  // Expect 4 values for each sprite
    GPU_AttributeFormat format;
} GPU_Attribute;

typedef struct GPU_AttributeSource
{
    Uint8 enabled;
    int num_values;
    void* next_value;
    // Automatic storage format
    int per_vertex_storage_stride_bytes;
    int per_vertex_storage_offset_bytes;
    int per_vertex_storage_size;  // Over 0 means that the per-vertex storage has been automatically allocated
    void* per_vertex_storage;  // Could point to the attribute's values or to allocated storage
    GPU_Attribute attribute;
} GPU_AttributeSource;


/*! Type enumeration for error codes.
 * \see GPU_PushErrorCode()
 * \see GPU_PopErrorCode()
 */
typedef enum {
    GPU_ERROR_NONE = 0,
    GPU_ERROR_BACKEND_ERROR = 1,
    GPU_ERROR_DATA_ERROR = 2,
    GPU_ERROR_USER_ERROR = 3,
    GPU_ERROR_UNSUPPORTED_FUNCTION = 4,
    GPU_ERROR_NULL_ARGUMENT = 5,
    GPU_ERROR_FILE_NOT_FOUND = 6
} GPU_ErrorEnum;


typedef struct GPU_ErrorObject
{
    char* function;
    GPU_ErrorEnum error;
    char* details;
} GPU_ErrorObject;


/*! Type enumeration for debug levels.
 * \see GPU_SetDebugLevel()
 * \see GPU_GetDebugLevel()
 */
typedef enum {
    GPU_DEBUG_LEVEL_0 = 0,
    GPU_DEBUG_LEVEL_1 = 1,
    GPU_DEBUG_LEVEL_2 = 2,
    GPU_DEBUG_LEVEL_3 = 3,
    GPU_DEBUG_LEVEL_MAX = 3
} GPU_DebugLevelEnum;




/*! Renderer object which specializes the API to a particular backend. */
struct GPU_Renderer
{
	/*! Struct identifier of the renderer. */
	GPU_RendererID id;
	GPU_RendererID requested_id;
	GPU_WindowFlagEnum SDL_init_flags;
	GPU_InitFlagEnum GPU_init_flags;
	
	GPU_ShaderLanguageEnum shader_language;
	int shader_version;
    GPU_FeatureEnum enabled_features;
	
	/*! Current display target */
	GPU_Target* current_context_target;
	
	
	/*! \see GPU_Init()
	 *  \see GPU_InitRenderer()
	 *  \see GPU_InitRendererByID()
	 */
	GPU_Target* (*Init)(GPU_Renderer* renderer, GPU_RendererID renderer_request, Uint16 w, Uint16 h, GPU_WindowFlagEnum SDL_flags);
	
	/*! \see GPU_IsFeatureEnabled() */
	Uint8 (*IsFeatureEnabled)(GPU_Renderer* renderer, GPU_FeatureEnum feature);
	
    /*! \see GPU_CreateTargetFromWindow
     * The extra parameter is used internally to reuse/reinit a target. */
    GPU_Target* (*CreateTargetFromWindow)(GPU_Renderer* renderer, Uint32 windowID, GPU_Target* target);
    
    /*! \see GPU_CreateAliasTarget() */
    GPU_Target* (*CreateAliasTarget)(GPU_Renderer* renderer, GPU_Target* target);

    /*! \see GPU_MakeCurrent */
    void (*MakeCurrent)(GPU_Renderer* renderer, GPU_Target* target, Uint32 windowID);
	
	/*! Sets up this renderer to act as the current renderer.  Called automatically by GPU_SetCurrentRenderer(). */
	void (*SetAsCurrent)(GPU_Renderer* renderer);
	
	/*! \see GPU_SetWindowResolution() */
	Uint8 (*SetWindowResolution)(GPU_Renderer* renderer, Uint16 w, Uint16 h);
	
	/*! \see GPU_SetVirtualResolution() */
	void (*SetVirtualResolution)(GPU_Renderer* renderer, GPU_Target* target, Uint16 w, Uint16 h);
	
	/*! \see GPU_UnsetVirtualResolution() */
	void (*UnsetVirtualResolution)(GPU_Renderer* renderer, GPU_Target* target);
	
	/*! Clean up the renderer state. */
	void (*Quit)(GPU_Renderer* renderer);
	
	/*! \see GPU_ToggleFullscreen() */
	Uint8 (*ToggleFullscreen)(GPU_Renderer* renderer, Uint8 use_desktop_resolution);

	/*! \see GPU_SetCamera() */
	GPU_Camera (*SetCamera)(GPU_Renderer* renderer, GPU_Target* target, GPU_Camera* cam);
	
    /*! \see GPU_CreateImage() */
	GPU_Image* (*CreateImage)(GPU_Renderer* renderer, Uint16 w, Uint16 h, GPU_FormatEnum format);
	
	/*! \see GPU_LoadImage() */
	GPU_Image* (*LoadImage)(GPU_Renderer* renderer, const char* filename);
	
    /*! \see GPU_CreateAliasImage() */
	GPU_Image* (*CreateAliasImage)(GPU_Renderer* renderer, GPU_Image* image);
	
	/*! \see GPU_SaveImage() */
	Uint8 (*SaveImage)(GPU_Renderer* renderer, GPU_Image* image, const char* filename);
	
	/*! \see GPU_CopyImage() */
	GPU_Image* (*CopyImage)(GPU_Renderer* renderer, GPU_Image* image);
	
	/*! \see GPU_UpdateImage */
	void (*UpdateImage)(GPU_Renderer* renderer, GPU_Image* image, SDL_Surface* surface, const GPU_Rect* surface_rect);
	
	/*! \see GPU_UpdateSubImage */
	void (*UpdateSubImage)(GPU_Renderer* renderer, GPU_Image* image, const GPU_Rect* image_rect, SDL_Surface* surface, const GPU_Rect* surface_rect);
	
	/*! \see GPU_UpdateImageBytes */
	void (*UpdateImageBytes)(GPU_Renderer* renderer, GPU_Image* image, const GPU_Rect* image_rect, const unsigned char* bytes, int bytes_per_row);
	
	/*! \see GPU_CopyImageFromSurface() */
	GPU_Image* (*CopyImageFromSurface)(GPU_Renderer* renderer, SDL_Surface* surface);
	
	/*! \see GPU_CopyImageFromTarget() */
	GPU_Image* (*CopyImageFromTarget)(GPU_Renderer* renderer, GPU_Target* target);
	
	/*! \see GPU_CopySurfaceFromTarget() */
	SDL_Surface* (*CopySurfaceFromTarget)(GPU_Renderer* renderer, GPU_Target* target);
	
	/*! \see GPU_CopySurfaceFromImage() */
	SDL_Surface* (*CopySurfaceFromImage)(GPU_Renderer* renderer, GPU_Image* image);
	
	/*! \see GPU_FreeImage() */
	void (*FreeImage)(GPU_Renderer* renderer, GPU_Image* image);
	
	/*! \see GPU_LoadTarget() */
	GPU_Target* (*LoadTarget)(GPU_Renderer* renderer, GPU_Image* image);
	
	/*! \see GPU_FreeTarget() */
	void (*FreeTarget)(GPU_Renderer* renderer, GPU_Target* target);

	/*! \see GPU_Blit() */
	void (*Blit)(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y);
	
	/*! \see GPU_BlitRotate() */
	void (*BlitRotate)(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float degrees);
	
	/*! \see GPU_BlitScale() */
	void (*BlitScale)(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float scaleX, float scaleY);
	
	/*! \see GPU_BlitTransform */
	void (*BlitTransform)(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float degrees, float scaleX, float scaleY);
	
	/*! \see GPU_BlitTransformX() */
	void (*BlitTransformX)(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float pivot_x, float pivot_y, float degrees, float scaleX, float scaleY);
	
	/*! \see GPU_BlitTransformMatrix() */
	void (*BlitTransformMatrix)(GPU_Renderer* renderer, GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float* matrix3x3);
	
	/*! \see GPU_BlitBatch() */
	void (*BlitBatch)(GPU_Renderer* renderer, GPU_Image* image, GPU_Target* target, unsigned int num_sprites, float* values, GPU_BlitFlagEnum flags);
	
	/*! \see GPU_TriangleBatch() */
	void (*TriangleBatch)(GPU_Renderer* renderer, GPU_Image* image, GPU_Target* target, unsigned short num_vertices, float* values, unsigned int num_indices, unsigned short* indices, GPU_BlitFlagEnum flags);
	
	/*! \see GPU_GenerateMipmaps() */
	void (*GenerateMipmaps)(GPU_Renderer* renderer, GPU_Image* image);

	/*! \see GPU_SetClip() */
	GPU_Rect (*SetClip)(GPU_Renderer* renderer, GPU_Target* target, Sint16 x, Sint16 y, Uint16 w, Uint16 h);

	/*! \see GPU_UnsetClip() */
	void (*UnsetClip)(GPU_Renderer* renderer, GPU_Target* target);
	
	/*! \see GPU_GetPixel() */
	SDL_Color (*GetPixel)(GPU_Renderer* renderer, GPU_Target* target, Sint16 x, Sint16 y);
	
	/*! \see GPU_SetImageFilter() */
	void (*SetImageFilter)(GPU_Renderer* renderer, GPU_Image* image, GPU_FilterEnum filter);
	
	/*! \see GPU_SetWrapMode() */
	void (*SetWrapMode)(GPU_Renderer* renderer, GPU_Image* image, GPU_WrapEnum wrap_mode_x, GPU_WrapEnum wrap_mode_y);

	/*! \see GPU_Clear() */
	void (*Clear)(GPU_Renderer* renderer, GPU_Target* target);
	/*! \see GPU_ClearRGBA() */
	void (*ClearRGBA)(GPU_Renderer* renderer, GPU_Target* target, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
	/*! \see GPU_FlushBlitBuffer() */
	void (*FlushBlitBuffer)(GPU_Renderer* renderer);
	/*! \see GPU_Flip() */
	void (*Flip)(GPU_Renderer* renderer, GPU_Target* target);
	
	
    /*! \see GPU_CompileShader_RW() */
	Uint32 (*CompileShader_RW)(GPU_Renderer* renderer, GPU_ShaderEnum shader_type, SDL_RWops* shader_source);
	
    /*! \see GPU_CompileShader() */
    Uint32 (*CompileShader)(GPU_Renderer* renderer, GPU_ShaderEnum shader_type, const char* shader_source);

    /*! \see GPU_LinkShaderProgram() */
    Uint32 (*LinkShaderProgram)(GPU_Renderer* renderer, Uint32 program_object);

    /*! \see GPU_LinkShaders() */
    Uint32 (*LinkShaders)(GPU_Renderer* renderer, Uint32 shader_object1, Uint32 shader_object2);

    /*! \see GPU_FreeShader() */
    void (*FreeShader)(GPU_Renderer* renderer, Uint32 shader_object);

    /*! \see GPU_FreeShaderProgram() */
    void (*FreeShaderProgram)(GPU_Renderer* renderer, Uint32 program_object);

    /*! \see GPU_AttachShader() */
    void (*AttachShader)(GPU_Renderer* renderer, Uint32 program_object, Uint32 shader_object);

    /*! \see GPU_DetachShader() */
    void (*DetachShader)(GPU_Renderer* renderer, Uint32 program_object, Uint32 shader_object);
    
    /*! \see GPU_IsDefaultShaderProgram() */
    Uint8 (*IsDefaultShaderProgram)(GPU_Renderer* renderer, Uint32 program_object);

    /*! \see GPU_ActivateShaderProgram() */
    void (*ActivateShaderProgram)(GPU_Renderer* renderer, Uint32 program_object, GPU_ShaderBlock* block);

    /*! \see GPU_DeactivateShaderProgram() */
    void (*DeactivateShaderProgram)(GPU_Renderer* renderer);

    /*! \see GPU_GetShaderMessage() */
    const char* (*GetShaderMessage)(GPU_Renderer* renderer);

    /*! \see GPU_GetAttribLocation() */
    int (*GetAttributeLocation)(GPU_Renderer* renderer, Uint32 program_object, const char* attrib_name);

    /*! \see GPU_GetUniformLocation() */
    int (*GetUniformLocation)(GPU_Renderer* renderer, Uint32 program_object, const char* uniform_name);
    
    /*! \see GPU_LoadShaderBlock() */
    GPU_ShaderBlock (*LoadShaderBlock)(GPU_Renderer* renderer, Uint32 program_object, const char* position_name, const char* texcoord_name, const char* color_name, const char* modelViewMatrix_name);
    
    /*! \see GPU_SetShaderBlock() */
    void (*SetShaderBlock)(GPU_Renderer* renderer, GPU_ShaderBlock block);
    
    /*! \see GPU_SetShaderImage() */
    void (*SetShaderImage)(GPU_Renderer* renderer, GPU_Image* image, int location, int image_unit);
    
    /*! \see GPU_GetUniformiv() */
    void (*GetUniformiv)(GPU_Renderer* renderer, Uint32 program_object, int location, int* values);

    /*! \see GPU_SetUniformi() */
    void (*SetUniformi)(GPU_Renderer* renderer, int location, int value);

    /*! \see GPU_SetUniformiv() */
    void (*SetUniformiv)(GPU_Renderer* renderer, int location, int num_elements_per_value, int num_values, int* values);

    /*! \see GPU_GetUniformuiv() */
    void (*GetUniformuiv)(GPU_Renderer* renderer, Uint32 program_object, int location, unsigned int* values);

    /*! \see GPU_SetUniformui() */
    void (*SetUniformui)(GPU_Renderer* renderer, int location, unsigned int value);

    /*! \see GPU_SetUniformuiv() */
    void (*SetUniformuiv)(GPU_Renderer* renderer, int location, int num_elements_per_value, int num_values, unsigned int* values);

    /*! \see GPU_GetUniformfv() */
    void (*GetUniformfv)(GPU_Renderer* renderer, Uint32 program_object, int location, float* values);

    /*! \see GPU_SetUniformf() */
    void (*SetUniformf)(GPU_Renderer* renderer, int location, float value);

    /*! \see GPU_SetUniformfv() */
    void (*SetUniformfv)(GPU_Renderer* renderer, int location, int num_elements_per_value, int num_values, float* values);

    /*! \see GPU_SetUniformMatrixfv() */
    void (*SetUniformMatrixfv)(GPU_Renderer* renderer, int location, int num_matrices, int num_rows, int num_columns, Uint8 transpose, float* values);
    
    /*! \see GPU_SetAttributef() */
    void (*SetAttributef)(GPU_Renderer* renderer, int location, float value);
    
    /*! \see GPU_SetAttributei() */
    void (*SetAttributei)(GPU_Renderer* renderer, int location, int value);
    
    /*! \see GPU_SetAttributeui() */
    void (*SetAttributeui)(GPU_Renderer* renderer, int location, unsigned int value);
    
    /*! \see GPU_SetAttributefv() */
    void (*SetAttributefv)(GPU_Renderer* renderer, int location, int num_elements, float* value);
    
    /*! \see GPU_SetAttributeiv() */
    void (*SetAttributeiv)(GPU_Renderer* renderer, int location, int num_elements, int* value);
    
    /*! \see GPU_SetAttributeuiv() */
    void (*SetAttributeuiv)(GPU_Renderer* renderer, int location, int num_elements, unsigned int* value);
    
    /*! \see GPU_SetAttributeSource() */
    void (*SetAttributeSource)(GPU_Renderer* renderer, int num_values, GPU_Attribute source);
    
    
    // Shapes
    
    /*! \see GPU_SetLineThickness() */
	float (*SetLineThickness)(GPU_Renderer* renderer, float thickness);
	
    /*! \see GPU_GetLineThickness() */
	float (*GetLineThickness)(GPU_Renderer* renderer);
	
    /*! \see GPU_Pixel() */
	void (*Pixel)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, SDL_Color color);

    /*! \see GPU_Line() */
	void (*Line)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

    /*! \see GPU_Arc() */
	void (*Arc)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, float start_angle, float end_angle, SDL_Color color);
	
    /*! \see GPU_ArcFilled() */
	void (*ArcFilled)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, float start_angle, float end_angle, SDL_Color color);

    /*! \see GPU_Circle() */
	void (*Circle)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color);

    /*! \see GPU_CircleFilled() */
	void (*CircleFilled)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float radius, SDL_Color color);

    /*! \see GPU_Sector() */
    void (*Sector)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float inner_radius, float outer_radius, float start_angle, float end_angle, SDL_Color color);

    /*! \see GPU_SectorFilled() */
    void (*SectorFilled)(GPU_Renderer* renderer, GPU_Target* target, float x, float y, float inner_radius, float outer_radius, float start_angle, float end_angle, SDL_Color color);
    
    /*! \see GPU_Tri() */
	void (*Tri)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color);

    /*! \see GPU_TriFilled() */
	void (*TriFilled)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color);

    /*! \see GPU_Rectangle() */
	void (*Rectangle)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

    /*! \see GPU_RectangleFilled() */
	void (*RectangleFilled)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

    /*! \see GPU_RectangleRound() */
	void (*RectangleRound)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color);

    /*! \see GPU_RectangleRoundFilled() */
	void (*RectangleRoundFilled)(GPU_Renderer* renderer, GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color);

    /*! \see GPU_Polygon() */
	void (*Polygon)(GPU_Renderer* renderer, GPU_Target* target, unsigned int num_vertices, float* vertices, SDL_Color color);

    /*! \see GPU_PolygonFilled() */
	void (*PolygonFilled)(GPU_Renderer* renderer, GPU_Target* target, unsigned int num_vertices, float* vertices, SDL_Color color);
};






// Setup calls

// Visual C does not support static inline
#ifdef _MSC_VER
static SDL_version GPU_GetCompiledVersion(void)
#else
static inline SDL_version GPU_GetCompiledVersion(void)
#endif
{
    SDL_version v = {SDL_GPU_VERSION_MAJOR, SDL_GPU_VERSION_MINOR, SDL_GPU_VERSION_PATCH};
    return v;
}

SDL_version GPU_GetLinkedVersion(void);

/*! The window corresponding to 'windowID' will be used to create the rendering context instead of creating a new window. */
void GPU_SetInitWindow(Uint32 windowID);

/*! Returns the window ID that has been set via GPU_SetInitWindow(). */
Uint32 GPU_GetInitWindow(void);

/*! Set special flags to use for initialization. Set these before calling GPU_Init().
 * \param GPU_flags An OR'ed combination of GPU_InitFlagEnum flags and GPU_FeatureEnum flags.  GPU_FeatureEnum flags will force GPU_Init() to create a renderer that supports all of the given flags or else fail.  Default flags (0) enable late swap vsync and double buffering. */
void GPU_SetPreInitFlags(GPU_InitFlagEnum GPU_flags);

/*! Returns the current special flags to use for initialization. */
GPU_InitFlagEnum GPU_GetPreInitFlags(void);

/*! Gets the default initialization renderer IDs for the current platform copied into the 'order' array and the number of renderer IDs into 'order_size'.  Pass NULL for 'order' to just get the size of the renderer order array.  Will return at most GPU_RENDERER_ORDER_MAX renderers. */
void GPU_GetDefaultRendererOrder(int* order_size, GPU_RendererID* order);

/*! Gets the current renderer ID order for initialization copied into the 'order' array and the number of renderer IDs into 'order_size'.  Pass NULL for 'order' to just get the size of the renderer order array. */
void GPU_GetRendererOrder(int* order_size, GPU_RendererID* order);

/*! Sets the renderer ID order to use for initialization.  If 'order' is NULL, it will restore the default order. */
void GPU_SetRendererOrder(int order_size, GPU_RendererID* order);

/*! Initializes SDL and SDL_gpu.  Creates a window and goes through the renderer order to create a renderer context.
 * \see GPU_SetRendererOrder()
 */
GPU_Target* GPU_Init(Uint16 w, Uint16 h, GPU_WindowFlagEnum SDL_flags);

/*! Initializes SDL and SDL_gpu.  Creates a window and the requested renderer context. */
GPU_Target* GPU_InitRenderer(GPU_RendererEnum renderer_enum, Uint16 w, Uint16 h, GPU_WindowFlagEnum SDL_flags);

/*! Initializes SDL and SDL_gpu.  Creates a window and the requested renderer context.
 * By requesting a renderer via ID, you can specify the major and minor versions of an individual renderer backend.
 * \see GPU_MakeRendererID
 */
GPU_Target* GPU_InitRendererByID(GPU_RendererID renderer_request, Uint16 w, Uint16 h, GPU_WindowFlagEnum SDL_flags);

/*! Checks for important GPU features which may not be supported depending on a device's extension support.  Feature flags (GPU_FEATURE_*) can be bitwise OR'd together. 
 * \return 1 if all of the passed features are enabled/supported
 * \return 0 if any of the passed features are disabled/unsupported
 */
Uint8 GPU_IsFeatureEnabled(GPU_FeatureEnum feature);

/*! Clean up the renderer state. */
void GPU_CloseCurrentRenderer(void);

/*! Clean up the renderer state and shut down SDL_gpu. */
void GPU_Quit(void);




// Debugging, logging, and error handling

/*! Sets the global debug level.
 * GPU_DEBUG_LEVEL_0: Normal
 * GPU_DEBUG_LEVEL_1: Prints messages when errors are pushed via GPU_PushErrorCode()
 * GPU_DEBUG_LEVEL_2: Elevates warning logs to error priority
 * GPU_DEBUG_LEVEL_3: Elevates info logs to error priority
 */
void GPU_SetDebugLevel(GPU_DebugLevelEnum level);

/*! Returns the current global debug level. */
GPU_DebugLevelEnum GPU_GetDebugLevel(void);

/*! Prints an informational log message. */
void GPU_LogInfo(const char* format, ...);

/*! Prints a warning log message. */
void GPU_LogWarning(const char* format, ...);

/*! Prints an error log message. */
void GPU_LogError(const char* format, ...);

#define GPU_Log GPU_LogInfo

/*! Pushes a new error code onto the error stack.  If the stack is full, this function does nothing.
 * \param function The name of the function that pushed the error
 * \param error The error code to push on the error stack
 * \param details Additional information string, can be NULL.
 */
void GPU_PushErrorCode(const char* function, GPU_ErrorEnum error, const char* details, ...);

/*! Pops an error object from the error stack and returns it.  If the error stack is empty, it returns an error object with NULL function, GPU_ERROR_NONE error, and NULL details. */
GPU_ErrorObject GPU_PopErrorCode(void);

/*! Gets the string representation of an error code. */
const char* GPU_GetErrorString(GPU_ErrorEnum error);








// Renderer setup controls

/*! Translates a GPU_RendererEnum into a string. */
const char* GPU_GetRendererEnumString(GPU_RendererEnum id);

/*! Returns an initialized GPU_RendererID. */
GPU_RendererID GPU_MakeRendererID(GPU_RendererEnum id, int major_version, int minor_version);

/*! Gets the renderer identifier for the given registration index. */
GPU_RendererID GPU_GetRendererID(unsigned int index);

/*! Gets the number of registered (available) renderers. */
int GPU_GetNumRegisteredRenderers(void);

/*! Gets an array of identifiers for the registered (available) renderers. */
void GPU_GetRegisteredRendererList(GPU_RendererID* renderers_array);

/*! Creates a new renderer matching the given identifier. */
GPU_Renderer* GPU_AddRenderer(GPU_RendererID id);

/*! Deletes the renderer matching the given identifier. */
void GPU_RemoveRenderer(GPU_RendererID id);



// Renderer controls

/*! Gets the number of active (created) renderers. */
int GPU_GetNumActiveRenderers(void);

/*! Gets an array of identifiers for the active renderers. */
void GPU_GetActiveRendererList(GPU_RendererID* renderers_array);

/*! Gets the renderer for the given renderer index. */
GPU_Renderer* GPU_GetRenderer(unsigned int index);

/*! \return The renderer matching the given identifier. */
GPU_Renderer* GPU_GetRendererByID(GPU_RendererID id);

/*! \return The current renderer */
GPU_Renderer* GPU_GetCurrentRenderer(void);

/*! Switches the current renderer to the renderer matching the given identifier. */
void GPU_SetCurrentRenderer(GPU_RendererID id);





// Context / window controls

/*! \return The renderer's current context target. */
GPU_Target* GPU_GetContextTarget(void);

/*! \return The target that is associated with the given windowID. */
GPU_Target* GPU_GetWindowTarget(Uint32 windowID);

/*! Creates a separate context for the given window using the current renderer and returns a GPU_Target that represents it. */
GPU_Target* GPU_CreateTargetFromWindow(Uint32 windowID);

/*! Makes the given window the current rendering destination for the given context target.
 * This also makes the target the current context for image loading and window operations.
 * If the target does not represent a window, this does nothing.
 */
void GPU_MakeCurrent(GPU_Target* target, Uint32 windowID);

/*! Change the actual size of the current context target's window.  This resets the virtual resolution and viewport of the context target.
 * Aside from direct resolution changes, this should also be called in response to SDL_WINDOWEVENT_RESIZED window events for resizable windows. */
Uint8 GPU_SetWindowResolution(Uint16 w, Uint16 h);

/*! Enable/disable fullscreen mode for the current context target's window.
 * On some platforms, this may destroy the renderer context and require that textures be reloaded.  Unfortunately, SDL does not provide a notification mechanism for this.
 * \param use_desktop_resolution If true, lets the window change its resolution when it enters fullscreen mode (via SDL_WINDOW_FULLSCREEN_DESKTOP).
 * \return 0 if the new mode is windowed, 1 if the new mode is fullscreen.  */
Uint8 GPU_ToggleFullscreen(Uint8 use_desktop_resolution);

/*! Enables/disables alpha blending for shape rendering on the current window. */
void GPU_SetShapeBlending(Uint8 enable);

/*! Translates a blend preset into a blend mode. */
GPU_BlendMode GPU_GetBlendModeFromPreset(GPU_BlendPresetEnum preset);

/*! Sets the blending component functions for shape rendering. */
void GPU_SetShapeBlendFunction(GPU_BlendFuncEnum source_color, GPU_BlendFuncEnum dest_color, GPU_BlendFuncEnum source_alpha, GPU_BlendFuncEnum dest_alpha);

/*! Sets the blending component equations for shape rendering. */
void GPU_SetShapeBlendEquation(GPU_BlendEqEnum color_equation, GPU_BlendEqEnum alpha_equation);
	
/*! Sets the blending mode for shape rendering on the current window, if supported by the renderer. */
void GPU_SetShapeBlendMode(GPU_BlendPresetEnum mode);

/*! Sets the thickness of lines for the current context. 
 * \param thickness New line thickness in pixels measured across the line.  Default is 1.0f.
 * \return The old thickness value
 */
float GPU_SetLineThickness(float thickness);

/*! Returns the current line thickness value. */
float GPU_GetLineThickness(void);





// Target controls

/*! Creates a target that aliases the given target.  Aliases can be used to store target settings (e.g. viewports) for easy switching.
 * GPU_FreeTarget() frees the alias's memory, but does not affect the original. */
GPU_Target* GPU_CreateAliasTarget(GPU_Target* target);

/*! Creates a new render target from the given image.  It can then be accessed from image->target. */
GPU_Target* GPU_LoadTarget(GPU_Image* image);

/*! Deletes a render target in the proper way for this renderer. */
void GPU_FreeTarget(GPU_Target* target);

/*! Change the logical size of the given target.  Rendering to this target will be scaled as if the dimensions were actually the ones given. */
void GPU_SetVirtualResolution(GPU_Target* target, Uint16 w, Uint16 h);

/*! Converts screen space coordinates (such as from mouse input) to logical drawing coordinates. */
void GPU_GetVirtualCoords(GPU_Target* target, float* x, float* y, float displayX, float displayY);

/*! Reset the logical size of the given target to its original value. */
void GPU_UnsetVirtualResolution(GPU_Target* target);

/*! \return A GPU_Rect with the given values. */
GPU_Rect GPU_MakeRect(float x, float y, float w, float h);

/*! \return An SDL_Color with the given values. */
SDL_Color GPU_MakeColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/*! Sets the given target's viewport. */
void GPU_SetViewport(GPU_Target* target, GPU_Rect viewport);

/*! \return A GPU_Camera with position (0, 0, -10), angle of 0, and zoom of 1. */
GPU_Camera GPU_GetDefaultCamera(void);

/*! \return The camera of the given render target.  If target is NULL, returns the default camera. */
GPU_Camera GPU_GetCamera(GPU_Target* target);

/*! Sets the current render target's current camera.
 * \param target A pointer to the target that will copy this camera.
 * \param cam A pointer to the camera data to use or NULL to use the default camera.
 * \return The old camera. */
GPU_Camera GPU_SetCamera(GPU_Target* target, GPU_Camera* cam);

/*! \return The RGBA color of a pixel. */
SDL_Color GPU_GetPixel(GPU_Target* target, Sint16 x, Sint16 y);

/*! Sets the clipping rect for the given render target. */
GPU_Rect GPU_SetClipRect(GPU_Target* target, GPU_Rect rect);

/*! Sets the clipping rect for the given render target. */
GPU_Rect GPU_SetClip(GPU_Target* target, Sint16 x, Sint16 y, Uint16 w, Uint16 h);

/*! Turns off clipping for the given target. */
void GPU_UnsetClip(GPU_Target* target);

/*! Sets the modulation color for subsequent drawing of images and shapes on the given target. 
 *  This has a cumulative effect with the image coloring functions.
 *  e.g. GPU_SetRGB(image, 255, 128, 0); GPU_SetTargetRGB(target, 128, 128, 128);
 *  Would make the image draw with color of roughly (128, 64, 0).
 */
void GPU_SetTargetColor(GPU_Target* target, SDL_Color* color);

/*! Sets the modulation color for subsequent drawing of images and shapes on the given target. 
 *  This has a cumulative effect with the image coloring functions.
 *  e.g. GPU_SetRGB(image, 255, 128, 0); GPU_SetTargetRGB(target, 128, 128, 128);
 *  Would make the image draw with color of roughly (128, 64, 0).
 */
void GPU_SetTargetRGB(GPU_Target* target, Uint8 r, Uint8 g, Uint8 b);

/*! Sets the modulation color for subsequent drawing of images and shapes on the given target. 
 *  This has a cumulative effect with the image coloring functions.
 *  e.g. GPU_SetRGB(image, 255, 128, 0); GPU_SetTargetRGB(target, 128, 128, 128);
 *  Would make the image draw with color of roughly (128, 64, 0).
 */
void GPU_SetTargetRGBA(GPU_Target* target, Uint8 r, Uint8 g, Uint8 b, Uint8 a);




// Surface controls

/*! Load surface from an image file that is supported by this renderer.  Don't forget to SDL_FreeSurface() it. */
SDL_Surface* GPU_LoadSurface(const char* filename);

/*! Save surface to a file.  The file type is deduced from the extension.  Supported formats are: png, bmp, tga.  Returns 0 on failure. */
Uint8 GPU_SaveSurface(SDL_Surface* surface, const char* filename);





// Image controls

/*! Create a new, blank image with the given format.  Don't forget to GPU_FreeImage() it.
	 * \param w Image width in pixels
	 * \param h Image height in pixels
	 * \param format Format of color channels.
	 */
GPU_Image* GPU_CreateImage(Uint16 w, Uint16 h, GPU_FormatEnum format);

/*! Load image from an image file that is supported by this renderer.  Don't forget to GPU_FreeImage() it. */
GPU_Image* GPU_LoadImage(const char* filename);

/*! Creates an image that aliases the given image.  Aliases can be used to store image settings (e.g. modulation color) for easy switching.
 * GPU_FreeImage() frees the alias's memory, but does not affect the original. */
GPU_Image* GPU_CreateAliasImage(GPU_Image* image);

/*! Copy an image to a new image.  Don't forget to GPU_FreeImage() both. */
GPU_Image* GPU_CopyImage(GPU_Image* image);

/*! Deletes an image in the proper way for this renderer.  Also deletes the corresponding GPU_Target if applicable.  Be careful not to use that target afterward! */
void GPU_FreeImage(GPU_Image* image);

/*! Update an image from surface data. */
void GPU_UpdateImage(GPU_Image* image, SDL_Surface* surface, const GPU_Rect* surface_rect);

/*! Update an image from surface data. */
void GPU_UpdateSubImage(GPU_Image* image, const GPU_Rect* image_rect, SDL_Surface* surface, const GPU_Rect* surface_rect);

/*! Update an image from an array of pixel data. */
void GPU_UpdateImageBytes(GPU_Image* image, const GPU_Rect* image_rect, const unsigned char* bytes, int bytes_per_row);

/*! Save image to a file.  The file type is deduced from the extension.  Supported formats are: png, bmp, tga.  Returns 0 on failure. */
Uint8 GPU_SaveImage(GPU_Image* image, const char* filename);

/*! Loads mipmaps for the given image, if supported by the renderer. */
void GPU_GenerateMipmaps(GPU_Image* image);

/*! Sets the modulation color for subsequent drawing of the given image. */
void GPU_SetColor(GPU_Image* image, SDL_Color* color);

/*! Sets the modulation color for subsequent drawing of the given image. */
void GPU_SetRGB(GPU_Image* image, Uint8 r, Uint8 g, Uint8 b);

/*! Sets the modulation color for subsequent drawing of the given image. */
void GPU_SetRGBA(GPU_Image* image, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/*! Gets the current alpha blending setting. */
Uint8 GPU_GetBlending(GPU_Image* image);

/*! Enables/disables alpha blending for the given image. */
void GPU_SetBlending(GPU_Image* image, Uint8 enable);

/*! Sets the blending component functions. */
void GPU_SetBlendFunction(GPU_Image* image, GPU_BlendFuncEnum source_color, GPU_BlendFuncEnum dest_color, GPU_BlendFuncEnum source_alpha, GPU_BlendFuncEnum dest_alpha);

/*! Sets the blending component equations. */
void GPU_SetBlendEquation(GPU_Image* image, GPU_BlendEqEnum color_equation, GPU_BlendEqEnum alpha_equation);

/*! Sets the blending mode, if supported by the renderer. */
void GPU_SetBlendMode(GPU_Image* image, GPU_BlendPresetEnum mode);

/*! Sets the image filtering mode, if supported by the renderer. */
void GPU_SetImageFilter(GPU_Image* image, GPU_FilterEnum filter);

/*! Gets the current pixel snap setting.  The default value is GPU_SNAP_POSITION_AND_DIMENSIONS.  */
GPU_SnapEnum GPU_GetSnapMode(GPU_Image* image);

/*! Sets the pixel grid snapping mode for the given image. */
void GPU_SetSnapMode(GPU_Image* image, GPU_SnapEnum mode);

/*! Sets the image wrapping mode, if supported by the renderer. */
void GPU_SetWrapMode(GPU_Image* image, GPU_WrapEnum wrap_mode_x, GPU_WrapEnum wrap_mode_y);



// Surface / Image / Target conversions

/*! Copy SDL_Surface data into a new GPU_Image.  Don't forget to SDL_FreeSurface() the surface and GPU_FreeImage() the image.*/
GPU_Image* GPU_CopyImageFromSurface(SDL_Surface* surface);

/*! Copy GPU_Target data into a new GPU_Image.  Don't forget to GPU_FreeImage() the image.*/
GPU_Image* GPU_CopyImageFromTarget(GPU_Target* target);

/*! Copy GPU_Target data into a new SDL_Surface.  Don't forget to SDL_FreeSurface() the surface.*/
SDL_Surface* GPU_CopySurfaceFromTarget(GPU_Target* target);

/*! Copy GPU_Image data into a new SDL_Surface.  Don't forget to SDL_FreeSurface() the surface and GPU_FreeImage() the image.*/
SDL_Surface* GPU_CopySurfaceFromImage(GPU_Image* image);



// Rendering

/*! Clears the contents of the given render target.  Fills the target with color {0, 0, 0, 0}. */
void GPU_Clear(GPU_Target* target);

/*! Fills the given render target with a color.  If 'color' is NULL, {0, 0, 0, 0} is used. */
void GPU_ClearColor(GPU_Target* target, SDL_Color* color);

/*! Fills the given render target with a color. */
void GPU_ClearRGBA(GPU_Target* target, Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/*! Draws the given image to the given render target.
    * \param src_rect The region of the source image to use.
    * \param x Destination x-position
    * \param y Destination y-position */
void GPU_Blit(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y);

/*! Rotates and draws the given image to the given render target.
    * \param src_rect The region of the source image to use.
    * \param x Destination x-position
    * \param y Destination y-position
    * \param degrees Rotation angle (in degrees) */
void GPU_BlitRotate(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float degrees);

/*! Scales and draws the given image to the given render target.
    * \param src_rect The region of the source image to use.
    * \param x Destination x-position
    * \param y Destination y-position
    * \param scaleX Horizontal stretch factor
    * \param scaleY Vertical stretch factor */
void GPU_BlitScale(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float scaleX, float scaleY);

/*! Scales, rotates, and draws the given image to the given render target.
    * \param src_rect The region of the source image to use.
    * \param x Destination x-position
    * \param y Destination y-position
    * \param degrees Rotation angle (in degrees)
    * \param scaleX Horizontal stretch factor
    * \param scaleY Vertical stretch factor */
void GPU_BlitTransform(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float degrees, float scaleX, float scaleY);

/*! Scales, rotates around a pivot point, and draws the given image to the given render target.  The drawing point (x, y) coincides with the pivot point on the src image (pivot_x, pivot_y).
	* \param src_rect The region of the source image to use.
	* \param x Destination x-position
	* \param y Destination y-position
	* \param pivot_x Pivot x-position (in image coordinates)
	* \param pivot_y Pivot y-position (in image coordinates)
	* \param degrees Rotation angle (in degrees)
	* \param scaleX Horizontal stretch factor
	* \param scaleY Vertical stretch factor */
void GPU_BlitTransformX(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float pivot_x, float pivot_y, float degrees, float scaleX, float scaleY);

/*! Transforms and draws the given image to the given render target.
	* \param src_rect The region of the source image to use.
	* \param x Destination x-position
	* \param y Destination y-position
	* \param matrix3x3 3x3 matrix in column-major order (index = row + column*numColumns) */
void GPU_BlitTransformMatrix(GPU_Image* image, GPU_Rect* src_rect, GPU_Target* target, float x, float y, float* matrix3x3);

/*! Performs 'num_sprites' blits of the given image to the given target.
 * Note: GPU_BlitBatch() cannot interpret a mix of normal values and "passthrough" values due to format ambiguity.
 * \param values A tightly-packed array of position (x,y), src_rect (x,y,w,h) values in image coordinates, and color (r,g,b,a) values with a range from 0-255.  Pass NULL to render with only custom shader attributes.
 * \param flags Bit flags to control the interpretation of the array parameters.  The only passthrough option accepted is GPU_PASSTHROUGH_ALL.
 */
void GPU_BlitBatch(GPU_Image* image, GPU_Target* target, unsigned int num_sprites, float* values, GPU_BlitFlagEnum flags);

/*! Performs 'num_sprites' blits of the given image to the given target.
 * \param positions A tightly-packed array of (x,y) values
 * \param src_rects A tightly-packed array of (x,y,w,h) values in image coordinates
 * \param colors A tightly-packed array of (r,g,b,a) values with a range from 0-255
 * \param flags Bit flags to control the interpretation of the array parameters
 */
void GPU_BlitBatchSeparate(GPU_Image* image, GPU_Target* target, unsigned int num_sprites, float* positions, float* src_rects, float* colors, GPU_BlitFlagEnum flags);

/*! Renders triangles from the given set of vertices.  This lets you render arbitrary 2D geometry.
 * \param values A tightly-packed array of vertex position (x,y), image coordinates (s,t), and color (r,g,b,a) values with a range from 0-255.  Pass NULL to render with only custom shader attributes.
 * \param indices If not NULL, this is used to specify which vertices to use and in what order (i.e. it indexes the vertices in the 'values' array).
 * \param flags Bit flags to control the interpretation of the array parameters.  Since 'values' contains per-vertex data, GPU_PASSTHROUGH_VERTICES is ignored.  Texture coordinates are scaled down using the image dimensions and color components are normalized to [0.0, 1.0].
 */
void GPU_TriangleBatch(GPU_Image* image, GPU_Target* target, unsigned short num_vertices, float* values, unsigned int num_indices, unsigned short* indices, GPU_BlitFlagEnum flags);

/*! Send all buffered blitting data to the current context target. */
void GPU_FlushBlitBuffer(void);

/*! Updates the given target's associated window. */
void GPU_Flip(GPU_Target* target);






// Shapes

/*! Renders a colored point.
 * \param target The destination render target
 * \param x x-coord of the point
 * \param y y-coord of the point
 * \param color The color of the shape to render
 */
void GPU_Pixel(GPU_Target* target, float x, float y, SDL_Color color);

/*! Renders a colored line.
 * \param target The destination render target
 * \param x1 x-coord of starting point
 * \param y1 y-coord of starting point
 * \param x2 x-coord of ending point
 * \param y2 y-coord of ending point
 * \param color The color of the shape to render
 */
void GPU_Line(GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

/*! Renders a colored arc curve (circle segment).
 * \param target The destination render target
 * \param x x-coord of center point
 * \param y y-coord of center point
 * \param radius The radius of the circle / distance from the center point that rendering will occur
 * \param start_angle The angle to start from, in degrees.  Measured clockwise from the positive x-axis.
 * \param end_angle The angle to end at, in degrees.  Measured clockwise from the positive x-axis.
 * \param color The color of the shape to render
 */
void GPU_Arc(GPU_Target* target, float x, float y, float radius, float start_angle, float end_angle, SDL_Color color);

/*! Renders a colored filled arc (circle segment / pie piece).
 * \param target The destination render target
 * \param x x-coord of center point
 * \param y y-coord of center point
 * \param radius The radius of the circle / distance from the center point that rendering will occur
 * \param start_angle The angle to start from, in degrees.  Measured clockwise from the positive x-axis.
 * \param end_angle The angle to end at, in degrees.  Measured clockwise from the positive x-axis.
 * \param color The color of the shape to render
 */
void GPU_ArcFilled(GPU_Target* target, float x, float y, float radius, float start_angle, float end_angle, SDL_Color color);

/*! Renders a colored circle outline.
 * \param target The destination render target
 * \param x x-coord of center point
 * \param y y-coord of center point
 * \param radius The radius of the circle / distance from the center point that rendering will occur
 * \param color The color of the shape to render
 */
void GPU_Circle(GPU_Target* target, float x, float y, float radius, SDL_Color color);

/*! Renders a colored filled circle.
 * \param target The destination render target
 * \param x x-coord of center point
 * \param y y-coord of center point
 * \param radius The radius of the circle / distance from the center point that rendering will occur
 * \param color The color of the shape to render
 */
void GPU_CircleFilled(GPU_Target* target, float x, float y, float radius, SDL_Color color);

/*! Renders a colored annular sector outline (ring segment).
 * \param target The destination render target
 * \param x x-coord of center point
 * \param y y-coord of center point
 * \param inner_radius The inner radius of the ring
 * \param outer_radius The outer radius of the ring
 * \param start_angle The angle to start from, in degrees.  Measured clockwise from the positive x-axis.
 * \param end_angle The angle to end at, in degrees.  Measured clockwise from the positive x-axis.
 * \param color The color of the shape to render
 */
void GPU_Sector(GPU_Target* target, float x, float y, float inner_radius, float outer_radius, float start_angle, float end_angle, SDL_Color color);

/*! Renders a colored filled annular sector (ring segment).
 * \param target The destination render target
 * \param x x-coord of center point
 * \param y y-coord of center point
 * \param inner_radius The inner radius of the ring
 * \param outer_radius The outer radius of the ring
 * \param start_angle The angle to start from, in degrees.  Measured clockwise from the positive x-axis.
 * \param end_angle The angle to end at, in degrees.  Measured clockwise from the positive x-axis.
 * \param color The color of the shape to render
 */
void GPU_SectorFilled(GPU_Target* target, float x, float y, float inner_radius, float outer_radius, float start_angle, float end_angle, SDL_Color color);

/*! Renders a colored triangle outline.
 * \param target The destination render target
 * \param x1 x-coord of first point
 * \param y1 y-coord of first point
 * \param x2 x-coord of second point
 * \param y2 y-coord of second point
 * \param x3 x-coord of third point
 * \param y3 y-coord of third point
 * \param color The color of the shape to render
 */
void GPU_Tri(GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color);

/*! Renders a colored filled triangle.
 * \param target The destination render target
 * \param x1 x-coord of first point
 * \param y1 y-coord of first point
 * \param x2 x-coord of second point
 * \param y2 y-coord of second point
 * \param x3 x-coord of third point
 * \param y3 y-coord of third point
 * \param color The color of the shape to render
 */
void GPU_TriFilled(GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color);

/*! Renders a colored rectangle outline.
 * \param target The destination render target
 * \param x1 x-coord of top-left corner
 * \param y1 y-coord of top-left corner
 * \param x2 x-coord of bottom-right corner
 * \param y2 y-coord of bottom-right corner
 * \param color The color of the shape to render
 */
void GPU_Rectangle(GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

/*! Renders a colored filled rectangle.
 * \param target The destination render target
 * \param x1 x-coord of top-left corner
 * \param y1 y-coord of top-left corner
 * \param x2 x-coord of bottom-right corner
 * \param y2 y-coord of bottom-right corner
 * \param color The color of the shape to render
 */
void GPU_RectangleFilled(GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color);

/*! Renders a colored rounded (filleted) rectangle outline.
 * \param target The destination render target
 * \param x1 x-coord of top-left corner
 * \param y1 y-coord of top-left corner
 * \param x2 x-coord of bottom-right corner
 * \param y2 y-coord of bottom-right corner
 * \param radius The radius of the corners
 * \param color The color of the shape to render
 */
void GPU_RectangleRound(GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color);

/*! Renders a colored filled rounded (filleted) rectangle.
 * \param target The destination render target
 * \param x1 x-coord of top-left corner
 * \param y1 y-coord of top-left corner
 * \param x2 x-coord of bottom-right corner
 * \param y2 y-coord of bottom-right corner
 * \param radius The radius of the corners
 * \param color The color of the shape to render
 */
void GPU_RectangleRoundFilled(GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color);

/*! Renders a colored polygon outline.  The vertices are expected to define a convex polygon.
 * \param target The destination render target
 * \param num_vertices Number of vertices (x and y pairs)
 * \param vertices An array of vertex positions stored as interlaced x and y coords, e.g. {x1, y1, x2, y2, ...}
 * \param color The color of the shape to render
 */
void GPU_Polygon(GPU_Target* target, unsigned int num_vertices, float* vertices, SDL_Color color);

/*! Renders a colored filled polygon.  The vertices are expected to define a convex polygon.
 * \param target The destination render target
 * \param num_vertices Number of vertices (x and y pairs)
 * \param vertices An array of vertex positions stored as interlaced x and y coords, e.g. {x1, y1, x2, y2, ...}
 * \param color The color of the shape to render
 */
void GPU_PolygonFilled(GPU_Target* target, unsigned int num_vertices, float* vertices, SDL_Color color);






// Shaders

/*! Loads shader source from an SDL_RWops, compiles it, and returns the new shader object. */
Uint32 GPU_CompileShader_RW(GPU_ShaderEnum shader_type, SDL_RWops* shader_source);

/*! Loads shader source from a file, compiles it, and returns the new shader object. */
Uint32 GPU_LoadShader(GPU_ShaderEnum shader_type, const char* filename);

/*! Compiles shader source and returns the new shader object. */
Uint32 GPU_CompileShader(GPU_ShaderEnum shader_type, const char* shader_source);

/*! Links a shader program with any attached shader objects. */
Uint32 GPU_LinkShaderProgram(Uint32 program_object);

/*! Creates and links a shader program with the given shader objects. */
Uint32 GPU_LinkShaders(Uint32 shader_object1, Uint32 shader_object2);

/*! Deletes a shader object. */
void GPU_FreeShader(Uint32 shader_object);

/*! Deletes a shader program. */
void GPU_FreeShaderProgram(Uint32 program_object);

/*! Attaches a shader object to a shader program for future linking. */
void GPU_AttachShader(Uint32 program_object, Uint32 shader_object);

/*! Detaches a shader object from a shader program. */
void GPU_DetachShader(Uint32 program_object, Uint32 shader_object);

/*! \return The current shader program */
Uint32 GPU_GetCurrentShaderProgram(void);

/*! Returns 1 if the given shader program is a default shader for the current context, 0 otherwise. */
Uint8 GPU_IsDefaultShaderProgram(Uint32 program_object);

/*! Activates the given shader program.  Passing NULL for 'block' will disable the built-in shader variables for custom shaders until a GPU_ShaderBlock is set again. */
void GPU_ActivateShaderProgram(Uint32 program_object, GPU_ShaderBlock* block);

/*! Deactivates the current shader program (activates program 0). */
void GPU_DeactivateShaderProgram(void);

/*! Returns the last shader log message. */
const char* GPU_GetShaderMessage(void);

/*! Returns an integer representing the location of the specified attribute shader variable. */
int GPU_GetAttributeLocation(Uint32 program_object, const char* attrib_name);

/*! Returns a filled GPU_AttributeFormat object. */
GPU_AttributeFormat GPU_MakeAttributeFormat(int num_elems_per_vertex, GPU_TypeEnum type, Uint8 normalize, int stride_bytes, int offset_bytes);

/*! Returns a filled GPU_Attribute object. */
GPU_Attribute GPU_MakeAttribute(int location, void* values, GPU_AttributeFormat format);

/*! Returns an integer representing the location of the specified uniform shader variable. */
int GPU_GetUniformLocation(Uint32 program_object, const char* uniform_name);

/*! Loads the given shader program's built-in attribute and uniform locations. */
GPU_ShaderBlock GPU_LoadShaderBlock(Uint32 program_object, const char* position_name, const char* texcoord_name, const char* color_name, const char* modelViewMatrix_name);

/*! Sets the current shader block to use the given attribute and uniform locations. */
void GPU_SetShaderBlock(GPU_ShaderBlock block);

/*! Sets the given image unit to the given image so that a custom shader can sample multiple textures.
    \param image The source image/texture.  Pass NULL to disable the image unit.
    \param location The uniform location of a texture sampler
    \param image_unit The index of the texture unit to set.  0 is the first unit, which is used by SDL_gpu's blitting functions.  1 would be the second unit. */
void GPU_SetShaderImage(GPU_Image* image, int location, int image_unit);

/*! Fills "values" with the value of the uniform shader variable at the given location. */
void GPU_GetUniformiv(Uint32 program_object, int location, int* values);

/*! Sets the value of the integer uniform shader variable at the given location.
    This is equivalent to calling GPU_SetUniformiv(location, 1, 1, &value). */
void GPU_SetUniformi(int location, int value);

/*! Sets the value of the integer uniform shader variable at the given location. */
void GPU_SetUniformiv(int location, int num_elements_per_value, int num_values, int* values);

/*! Fills "values" with the value of the uniform shader variable at the given location. */
void GPU_GetUniformuiv(Uint32 program_object, int location, unsigned int* values);

/*! Sets the value of the unsigned integer uniform shader variable at the given location.
    This is equivalent to calling GPU_SetUniformuiv(location, 1, 1, &value). */
void GPU_SetUniformui(int location, unsigned int value);

/*! Sets the value of the unsigned integer uniform shader variable at the given location. */
void GPU_SetUniformuiv(int location, int num_elements_per_value, int num_values, unsigned int* values);

/*! Fills "values" with the value of the uniform shader variable at the given location. */
void GPU_GetUniformfv(Uint32 program_object, int location, float* values);

/*! Sets the value of the floating point uniform shader variable at the given location.
    This is equivalent to calling GPU_SetUniformfv(location, 1, 1, &value). */
void GPU_SetUniformf(int location, float value);

/*! Sets the value of the floating point uniform shader variable at the given location. */
void GPU_SetUniformfv(int location, int num_elements_per_value, int num_values, float* values);

/*! Fills "values" with the value of the uniform shader variable at the given location.  The results are identical to calling GPU_GetUniformfv().  Matrices are gotten in column-major order. */
void GPU_GetUniformMatrixfv(Uint32 program_object, int location, float* values);

/*! Sets the value of the matrix uniform shader variable at the given location.  The size of the matrices sent is specified by num_rows and num_columns.  Rows and columns must be between 2 and 4. */
void GPU_SetUniformMatrixfv(int location, int num_matrices, int num_rows, int num_columns, Uint8 transpose, float* values);

/*! Sets a constant-value shader attribute that will be used for each rendered vertex. */
void GPU_SetAttributef(int location, float value);

/*! Sets a constant-value shader attribute that will be used for each rendered vertex. */
void GPU_SetAttributei(int location, int value);

/*! Sets a constant-value shader attribute that will be used for each rendered vertex. */
void GPU_SetAttributeui(int location, unsigned int value);

/*! Sets a constant-value shader attribute that will be used for each rendered vertex. */
void GPU_SetAttributefv(int location, int num_elements, float* value);

/*! Sets a constant-value shader attribute that will be used for each rendered vertex. */
void GPU_SetAttributeiv(int location, int num_elements, int* value);

/*! Sets a constant-value shader attribute that will be used for each rendered vertex. */
void GPU_SetAttributeuiv(int location, int num_elements, unsigned int* value);

/*! Enables a shader attribute and sets its source data. */
void GPU_SetAttributeSource(int num_values, GPU_Attribute source);




#ifdef __cplusplus
}
#endif



#endif

