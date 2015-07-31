#ifndef _SDL_GPU_OPENGL_1_H__
#define _SDL_GPU_OPENGL_1_H__

#include "SDL_gpu.h"

#if !defined(SDL_GPU_DISABLE_OPENGL) && !defined(SDL_GPU_DISABLE_OPENGL_1)

    // Hacks to fix compile errors due to polluted namespace
    #ifdef _WIN32
    #define _WINUSER_H
    #define _WINGDI_H
    #endif

    #include "glew.h"
	
	#if defined(GL_EXT_bgr) && !defined(GL_BGR)
		#define GL_BGR GL_BGR_EXT
	#endif
	#if defined(GL_EXT_bgra) && !defined(GL_BGRA)
		#define GL_BGRA GL_BGRA_EXT
	#endif
	#if defined(GL_EXT_abgr) && !defined(GL_ABGR)
		#define GL_ABGR GL_ABGR_EXT
	#endif
	
	#undef glBlendFuncSeparate
    #define glBlendFuncSeparate glBlendFuncSeparateEXT
	#undef glBlendEquation
    #define glBlendEquation glBlendEquationEXT
	#undef glBlendEquationSeparate
    #define glBlendEquationSeparate glBlendEquationSeparateEXT
    
	#undef GL_MIRRORED_REPEAT
    #define GL_MIRRORED_REPEAT GL_MIRRORED_REPEAT_ARB
	
	#undef glCreateShader
	#undef GL_VERTEX_SHADER
	#undef GL_FRAGMENT_SHADER
	#undef glShaderSource
	#undef glCompileShader
	#undef glGetShaderiv
	#undef GL_COMPILE_STATUS
	#undef glGetShaderInfoLog
	#undef glDeleteShader
	#undef glActiveTexture
	#undef GL_TEXTURE0
	#define glCreateShader glCreateShaderObjectARB
	#define GL_VERTEX_SHADER GL_VERTEX_SHADER_ARB
	#define GL_FRAGMENT_SHADER GL_FRAGMENT_SHADER_ARB
	#define glShaderSource glShaderSourceARB
	#define glCompileShader glCompileShaderARB
	#define glGetShaderiv glGetObjectParameterivARB
	#define GL_COMPILE_STATUS GL_OBJECT_COMPILE_STATUS_ARB
	#define glGetShaderInfoLog glGetInfoLogARB
	#define glDeleteShader glDeleteObjectARB
	#define glActiveTexture glActiveTextureARB
	#define GL_TEXTURE0 GL_TEXTURE0_ARB
	
	#undef glCreateProgram
	#undef glAttachShader
	#undef glLinkProgram
	#undef GL_LINK_STATUS
	#undef glGetProgramiv
	#undef glGetProgramInfoLog
	#undef glUseProgram
	#undef glDeleteProgram
	#define glCreateProgram glCreateProgramObjectARB
	#define glAttachShader glAttachObjectARB
	#define glLinkProgram glLinkProgramARB
	#define GL_LINK_STATUS GL_OBJECT_LINK_STATUS_ARB
	#define glGetProgramiv glGetObjectParameterivARB
	#define glGetProgramInfoLog glGetInfoLogARB
	#define glUseProgram glUseProgramObjectARB
	#define glDeleteProgram glDeleteObjectARB
	
	#undef glGetUniformLocation
	#undef glGetUniformiv
	#undef glUniform1i
	#undef glUniform1iv
	#undef glUniform2iv
	#undef glUniform3iv
	#undef glUniform4iv
	#undef glUniform1f
	#undef glUniform1fv
	#undef glUniform2fv
	#undef glUniform3fv
	#undef glUniform4fv
	#undef glUniformMatrix4fv
	#define glGetUniformLocation glGetUniformLocationARB
	#define glGetUniformiv glGetUniformivARB
	#define glUniform1i glUniform1iARB
	#define glUniform1iv glUniform1ivARB
	#define glUniform2iv glUniform2ivARB
	#define glUniform3iv glUniform3ivARB
	#define glUniform4iv glUniform4ivARB
	#define glUniform1f glUniform1fARB
	#define glUniform1fv glUniform1fvARB
	#define glUniform2fv glUniform2fvARB
	#define glUniform3fv glUniform3fvARB
	#define glUniform4fv glUniform4fvARB
	#define glUniformMatrix4fv glUniformMatrix4fvARB
	
	#undef glGetAttribLocation
	#undef glVertexAttrib1f
	#undef glVertexAttrib2f
	#undef glVertexAttrib3f
	#undef glVertexAttrib4f
	#undef glVertexAttribI1i
	#undef glVertexAttribI2i
	#undef glVertexAttribI3i
	#undef glVertexAttribI4i
	#undef glVertexAttribI1ui
	#undef glVertexAttribI2ui
	#undef glVertexAttribI3ui
	#undef glVertexAttribI4ui
	#define glGetAttribLocation glGetAttribLocationARB
	#define glVertexAttrib1f glVertexAttrib1fARB
	#define glVertexAttrib2f glVertexAttrib2fARB
	#define glVertexAttrib3f glVertexAttrib3fARB
	#define glVertexAttrib4f glVertexAttrib4fARB
	#define glVertexAttribI1i glVertexAttrib1sARB
	#define glVertexAttribI2i glVertexAttrib2sARB
	#define glVertexAttribI3i glVertexAttrib3sARB
	#define glVertexAttribI4i glVertexAttrib4sARB
	#define glVertexAttribI1ui glVertexAttrib1sARB
	#define glVertexAttribI2ui glVertexAttrib2sARB
	#define glVertexAttribI3ui glVertexAttrib3sARB
	#define glVertexAttribI4ui glVertexAttrib4sARB
	
	#undef glGenBuffers
	#undef glDeleteBuffers
	#undef glBindBuffer
	#undef glBufferData
	#undef glBufferSubData
	#undef GL_ARRAY_BUFFER
	#define glGenBuffers glGenBuffersARB
	#define glDeleteBuffers glDeleteBuffersARB
	#define glBindBuffer glBindBufferARB
	#define glBufferData glBufferDataARB
	#define glBufferSubData glBufferSubDataARB
	#define GL_ARRAY_BUFFER GL_ARRAY_BUFFER_ARB
	
	
	#undef glEnableVertexAttribArray
	#undef glDisableVertexAttribArray
	#undef glVertexAttribPointer
	#define glEnableVertexAttribArray glEnableVertexAttribArrayARB
	#define glDisableVertexAttribArray glDisableVertexAttribArrayARB
	#define glVertexAttribPointer glVertexAttribPointerARB
	
#endif



#define GPU_CONTEXT_DATA ContextData_OpenGL_1
#define GPU_IMAGE_DATA ImageData_OpenGL_1
#define GPU_TARGET_DATA TargetData_OpenGL_1




#define GPU_DEFAULT_TEXTURED_VERTEX_SHADER_SOURCE \
"#version 110\n\
\
attribute vec2 gpu_Vertex;\n\
attribute vec2 gpu_TexCoord;\n\
attribute vec4 gpu_Color;\n\
uniform mat4 gpu_ModelViewProjectionMatrix;\n\
\
varying vec4 color;\n\
varying vec2 texCoord;\n\
\
void main(void)\n\
{\n\
	color = gpu_Color;\n\
	texCoord = vec2(gpu_TexCoord);\n\
	gl_Position = gpu_ModelViewProjectionMatrix * vec4(gpu_Vertex, 0.0, 1.0);\n\
}"

// Tier 3 uses shader attributes to send position, texcoord, and color data for each vertex.
#define GPU_DEFAULT_UNTEXTURED_VERTEX_SHADER_SOURCE \
"#version 110\n\
\
attribute vec2 gpu_Vertex;\n\
attribute vec4 gpu_Color;\n\
uniform mat4 gpu_ModelViewProjectionMatrix;\n\
\
varying vec4 color;\n\
\
void main(void)\n\
{\n\
	color = gpu_Color;\n\
	gl_Position = gpu_ModelViewProjectionMatrix * vec4(gpu_Vertex, 0.0, 1.0);\n\
}"


#define GPU_DEFAULT_TEXTURED_FRAGMENT_SHADER_SOURCE \
"#version 110\n\
\
varying vec4 color;\n\
varying vec2 texCoord;\n\
\
uniform sampler2D tex;\n\
\
void main(void)\n\
{\n\
    gl_FragColor = texture2D(tex, texCoord) * color;\n\
}"

#define GPU_DEFAULT_UNTEXTURED_FRAGMENT_SHADER_SOURCE \
"#version 110\n\
\
varying vec4 color;\n\
\
void main(void)\n\
{\n\
    gl_FragColor = color;\n\
}"





typedef struct ContextData_OpenGL_1
{
	SDL_Color last_color;
	Uint8 last_use_texturing;
	unsigned int last_shape;
	Uint8 last_use_blending;
	GPU_BlendMode last_blend_mode;
	GPU_Rect last_viewport;
	GPU_Camera last_camera;
	Uint8 last_camera_inverted;
	
	GPU_Image* last_image;
	GPU_Target* last_target;
	float* blit_buffer;  // Holds sets of 4 vertices and 4 tex coords interleaved (e.g. [x0, y0, z0, s0, t0, ...]).
	unsigned short blit_buffer_num_vertices;
	unsigned short blit_buffer_max_num_vertices;
	unsigned short* index_buffer;  // Indexes into the blit buffer so we can use 4 vertices for every 2 triangles (1 quad)
	unsigned int index_buffer_num_vertices;
	unsigned int index_buffer_max_num_vertices;
	
    
    unsigned int blit_VBO[2];  // For double-buffering
    Uint8 blit_VBO_flop;
    GPU_ShaderBlock shader_block[2];
    GPU_ShaderBlock current_shader_block;
    
	GPU_AttributeSource shader_attributes[16];
	unsigned int attribute_VBO[16];
} ContextData_OpenGL_1;

typedef struct ImageData_OpenGL_1
{
    int refcount;
	Uint32 handle;
	Uint32 format;
} ImageData_OpenGL_1;

typedef struct TargetData_OpenGL_1
{
    int refcount;
	Uint32 handle;
	Uint32 format;
} TargetData_OpenGL_1;



#endif
