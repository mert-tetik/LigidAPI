#ifndef CANVAS_H
#define CANVAS_H

#include "./Canvas_Utils.h"

// Define the struct
typedef struct{
    int width;
    int height;
    int channels;
    unsigned int opengl_texture_buffer_ID;
    unsigned int opengl_framebuffer_ID;

} LigidCanvas;

LigidCanvas* LigidAPI_create_canvas(int width, int height, int channels);
void LigidAPI_delete_canvas(LigidCanvas* canvas);

int LigidAPI_paint_canvas(LigidCanvas* canvas, LigidBrush brush, LigidStroke stroke, LigidRGBA color); 
int LigidAPI_clear_canvas(LigidCanvas* canvas, LigidRGBA color); 

#endif // CANVAS_H