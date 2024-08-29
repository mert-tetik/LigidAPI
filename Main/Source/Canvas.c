#include <stdio.h>
#include <string.h>

#include "../Headers/Canvas.h"
#include "../Headers/OpenGL_specific.h"

#ifndef LIGIDAPI_MAX_CANVAS_SIZE 
#define LIGIDAPI_MAX_CANVAS_SIZE 20
#endif

static int canvas_creation_index = -1;
static LigidCanvas canvases[LIGIDAPI_MAX_CANVAS_SIZE];

LigidCanvas* LigidAPI_create_canvas(int width, int height, int channels, LigidRGBA canvas_color){
    // Create the canvas
    LigidCanvas canvas;
    canvas.width = width;
    canvas.height = height;
    canvas.channels = channels;

    canvas.opengl_texture_buffer_ID = 0;
    canvas.opengl_texture_buffer_ID = LigidAPIUtil_createTexture(0, width, height, channels);

    canvas_creation_index++;

    if(canvas_creation_index < LIGIDAPI_MAX_CANVAS_SIZE){
        // Return the canvas
        canvases[canvas_creation_index] = canvas;
        return &canvases[canvas_creation_index];
    }
    else{
        return 0;
    }
}

void LigidAPI_delete_canvas(LigidCanvas* canvas){

}

int LigidAPI_paint_canvas(LigidCanvas* canvas, LigidBrush brush, LigidStroke stroke, LigidRGBA color){
    return 1;
} 

int LigidAPI_clear_canvas(LigidCanvas* canvas, LigidRGBA color){
    return 1;
} 

LigidBrush LigidAPI_create_brush(float radius, float hardness, float opacity, float spacing, float sizeJitter, float scatter, 
                            float fade, float rotation, float rotationJitter, float alphaJitter, int sinWavePattern){
    LigidBrush brush;
    brush.radius = radius;
    brush.hardness = hardness;
    brush.opacity = opacity;
    brush.spacing = spacing;
    brush.sizeJitter = sizeJitter;
    brush.scatter = scatter;
    brush.fade = fade;
    brush.rotation = rotation;
    brush.rotationJitter = rotationJitter;
    brush.alphaJitter = alphaJitter;
    brush.sinWavePattern = sinWavePattern;

    return brush;
}

LigidRGBA LigidAPI_get_color(float r, float g, float b, float a){
    LigidRGBA color;
    color.r = r; 
    color.g = g; 
    color.b = b; 
    color.a = a; 
    
    color.r_u = (unsigned char)(r * 255.f); 
    color.g_u = (unsigned char)(g * 255.f); 
    color.b_u = (unsigned char)(b * 255.f); 
    color.a_u = (unsigned char)(a * 255.f); 

    return color;
}

LigidStroke LigidAPI_get_stroke(float start_pos_x, float start_pos_y, float end_pos_x, float end_pos_y){
    LigidStroke stroke;
    stroke.start_pos_x = start_pos_x; 
    stroke.start_pos_y = start_pos_y; 
    stroke.end_pos_x = end_pos_x; 
    stroke.end_pos_y = end_pos_y; 

    return stroke;
}