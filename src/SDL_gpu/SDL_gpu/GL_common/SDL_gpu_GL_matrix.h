#ifndef _SDL_GPU_GL_MATRIX_H__
#define _SDL_GPU_GL_MATRIX_H__

#ifdef __cplusplus
extern "C" {
#endif

// Basic matrix operations
void GPU_MatrixCopy(float* result, const float* A);
void GPU_MatrixIdentity(float* result);
void GPU_Multiply4x4(float* result, float* A, float* B);
void GPU_MultiplyAndAssign(float* result, float* A);


// State-specific operations
#define GPU_MODELVIEW 0
#define GPU_PROJECTION 1

const char* GPU_GetMatrixString(float* A);
void GPU_MatrixMode(int matrix_mode);
float* GPU_GetCurrentMatrix(void);
float* GPU_GetModelView(void);
float* GPU_GetProjection(void);
void GPU_GetModelViewProjection(float* result);
void GPU_PushMatrix(void);
void GPU_PopMatrix(void);
void GPU_LoadIdentity(void);
void GPU_Ortho(float left, float right, float bottom, float top, float near, float far);
void GPU_Frustum(float right, float left, float bottom, float top, float near, float far);
void GPU_Translate(float x, float y, float z);
void GPU_Scale(float sx, float sy, float sz);
void GPU_Rotate(float degrees, float x, float y, float z);
void GPU_MultMatrix(float* matrix4x4);

#ifdef __cplusplus
}
#endif

#endif
