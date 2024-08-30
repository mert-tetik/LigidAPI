#ifndef LIGIDAPI_CANVAS_H
#define LIGIDAPI_CANVAS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "./Canvas_Utils.h"

// Define the struct
typedef struct{
    int width;
    int height;
    int channels;
    unsigned int opengl_texture_buffer_ID;
    unsigned int opengl_texture_buffer_ID_copy;

} LigidCanvas;

LigidCanvas* LigidAPI_create_canvas(int width, int height, int channels, LigidRGBA canvas_color);
void LigidAPI_delete_canvas(LigidCanvas* canvas);

int LigidAPI_paint_canvas(LigidCanvas* canvas, LigidBrush brush, LigidStroke stroke, LigidRGBA color); 
int LigidAPI_clear_canvas(LigidCanvas* canvas, LigidRGBA color); 

#ifdef __cplusplus
}
#endif

#endif // LIGIDAPI_CANVAS_H