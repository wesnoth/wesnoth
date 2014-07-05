#include "SDL_gpu.h"
#include <string.h>

#define CHECK_RENDERER(ret) \
GPU_Renderer* renderer = GPU_GetCurrentRenderer(); \
if(renderer == NULL) \
    return ret;


float GPU_SetLineThickness(float thickness)
{
	CHECK_RENDERER(1.0f);
	if(renderer->SetLineThickness == NULL)
		return 1.0f;
	
	return renderer->SetLineThickness(renderer, thickness);
}

float GPU_GetLineThickness(void)
{
	CHECK_RENDERER(1.0f);
	if(renderer->GetLineThickness == NULL)
		return 1.0f;
	
	return renderer->GetLineThickness(renderer);
}

void GPU_Pixel(GPU_Target* target, float x, float y, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->Pixel == NULL)
		return;
	
	renderer->Pixel(renderer, target, x, y, color);
}

void GPU_Line(GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->Line == NULL)
		return;
	
	renderer->Line(renderer, target, x1, y1, x2, y2, color);
}


void GPU_Arc(GPU_Target* target, float x, float y, float radius, float start_angle, float end_angle, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->Arc == NULL)
		return;
	
	renderer->Arc(renderer, target, x, y, radius, start_angle, end_angle, color);
}


void GPU_ArcFilled(GPU_Target* target, float x, float y, float radius, float start_angle, float end_angle, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->ArcFilled == NULL)
		return;
	
	renderer->ArcFilled(renderer, target, x, y, radius, start_angle, end_angle, color);
}

void GPU_Circle(GPU_Target* target, float x, float y, float radius, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->Circle == NULL)
		return;
	
	renderer->Circle(renderer, target, x, y, radius, color);
}

void GPU_CircleFilled(GPU_Target* target, float x, float y, float radius, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->CircleFilled == NULL)
		return;
	
	renderer->CircleFilled(renderer, target, x, y, radius, color);
}

void GPU_Sector(GPU_Target* target, float x, float y, float inner_radius, float outer_radius, float start_angle, float end_angle, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->Sector == NULL)
		return;
	
	renderer->Sector(renderer, target, x, y, inner_radius, outer_radius, start_angle, end_angle, color);
}

void GPU_SectorFilled(GPU_Target* target, float x, float y, float inner_radius, float outer_radius, float start_angle, float end_angle, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->SectorFilled == NULL)
		return;
	
	renderer->SectorFilled(renderer, target, x, y, inner_radius, outer_radius, start_angle, end_angle, color);
}

void GPU_Tri(GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->Tri == NULL)
		return;
	
	renderer->Tri(renderer, target, x1, y1, x2, y2, x3, y3, color);
}

void GPU_TriFilled(GPU_Target* target, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->TriFilled == NULL)
		return;
	
	renderer->TriFilled(renderer, target, x1, y1, x2, y2, x3, y3, color);
}

void GPU_Rectangle(GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->Rectangle == NULL)
		return;
	
	renderer->Rectangle(renderer, target, x1, y1, x2, y2, color);
}

void GPU_RectangleFilled(GPU_Target* target, float x1, float y1, float x2, float y2, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->RectangleFilled == NULL)
		return;
	
	renderer->RectangleFilled(renderer, target, x1, y1, x2, y2, color);
}

void GPU_RectangleRound(GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->RectangleRound == NULL)
		return;
	
	renderer->RectangleRound(renderer, target, x1, y1, x2, y2, radius, color);
}

void GPU_RectangleRoundFilled(GPU_Target* target, float x1, float y1, float x2, float y2, float radius, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->RectangleRoundFilled == NULL)
		return;
	
	renderer->RectangleRoundFilled(renderer, target, x1, y1, x2, y2, radius, color);
}

void GPU_Polygon(GPU_Target* target, unsigned int num_vertices, float* vertices, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->Polygon == NULL)
		return;
	
	renderer->Polygon(renderer, target, num_vertices, vertices, color);
}

void GPU_PolygonFilled(GPU_Target* target, unsigned int num_vertices, float* vertices, SDL_Color color)
{
	CHECK_RENDERER();
	if(renderer->PolygonFilled == NULL)
		return;
	
	renderer->PolygonFilled(renderer, target, num_vertices, vertices, color);
}

