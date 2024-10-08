#include <stdio.h>
#include <string.h>
#include <math.h>

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
    LigidAPIUtil_clearTexture(canvas.opengl_texture_buffer_ID, width, height, canvas_color.r, canvas_color.g, canvas_color.b, canvas_color.a);
    
    canvas.opengl_texture_buffer_ID_copy = 0;
    canvas.opengl_texture_buffer_ID_copy = LigidAPIUtil_createTexture(0, width, height, channels);
    LigidAPIUtil_clearTexture(canvas.opengl_texture_buffer_ID_copy, width, height, 0.f, 0.f, 0.f, 0.f);
    
    canvas.opengl_texture_buffer_stroke_ID = 0;
    canvas.opengl_texture_buffer_stroke_ID = LigidAPIUtil_createTexture(0, width, height, channels);
    LigidAPIUtil_clearTexture(canvas.opengl_texture_buffer_stroke_ID, width, height, 0.f, 0.f, 0.f, 0.f);
    
    canvas.display_opengl_texture_buffer_ID = 0;
    canvas.display_opengl_texture_buffer_ID = LigidAPIUtil_createTexture(0, width, height, channels);
    LigidAPIUtil_clearTexture(canvas.display_opengl_texture_buffer_ID, width, height, canvas_color.r, canvas_color.g, canvas_color.b, canvas_color.a);

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

typedef struct {
    float x;
    float y;
} LigidAPI_vec2;

static float distance(float x1, float y1, float x2, float y2) {
    // Calculate the differences between the corresponding coordinates
    float dx = x2 - x1;
    float dy = y2 - y1;

    // Compute the distance using the Euclidean distance formula
    return sqrtf(dx * dx + dy * dy);
}


static void get_whole_stroke(LigidAPI_vec2 positions[20][80], LigidStroke stroke, int* position_i, int* position_ii)
{
    int positions_i = 0;
    int positions_ii = 0;

    LigidAPI_vec2 start;
    start.x = stroke.start_pos_x;
    start.y = stroke.start_pos_y;
    
    LigidAPI_vec2 dest;
    dest.x = stroke.end_pos_x;
    dest.y = stroke.end_pos_y;
    
    positions[positions_ii][positions_i] = start;
    positions_i++;
    
    //----------------------PAINTING----------------------\\

    int differenceBetweenMousePoints = distance(start.x, start.y, dest.x, dest.y);

    float xposDif = (start.x - dest.x) / differenceBetweenMousePoints;
    float yposDif = (start.y - dest.y) / differenceBetweenMousePoints;

    for (size_t i = 0; i < differenceBetweenMousePoints; i++)
    {
        start.x -= xposDif;
        start.y -= yposDif;
        
        LigidAPI_vec2 strokeLocation = start;
        if(positions_i < 80){
            positions[positions_ii][positions_i] = strokeLocation;
            positions_i++;
        }
        else{
            positions_i = 0;
            positions_ii++;
            
            positions[positions_ii][positions_i] = strokeLocation;
        }
    }

    *position_i = positions_i;
    *position_ii = positions_ii;
}

static unsigned int paint_program;
static unsigned int update_program;

int LigidAPI_paint_canvas(LigidCanvas* canvas, LigidBrush brush, LigidStroke stroke, LigidRGBA color){

    if(brush.radius <= 0.f)
        return 0;

    const char* update_fragmentShaderSource = 
        "in vec2 TexCoords;"
        "out vec4 outClr;"
        "uniform sampler2D bg_txtr;"
        "uniform sampler2D stroke_txtr;"
        "uniform float opacity;"

        "void strokeBlendUniColor("
        "    float src, float srcA, float dst, out float color)"
        "{"
        "    color = dst + (1.0 - dst) * src;"
        "    if (color > srcA) {"
        "        color = max(dst, srcA);"
        "    }"
        "}"
        "void main() {"
        ""
        "   outClr = texture(bg_txtr, TexCoords);"
        "   vec4 stroke = texture(stroke_txtr, TexCoords) * opacity; "
        "   strokeBlendUniColor(stroke.a, 1., outClr.a, outClr.a);"
        "   outClr.rgb = mix(outClr.rgb, stroke.rgb, stroke.a);"
        "}";

    
    if(update_program == 0){
        update_program = LigidAPIUtil_load_shader_OpenGL(
                    "layout(location = 0) in vec3 aPos;"
                    "layout(location = 1) in vec2 aTexCoords;"
                    "out vec2 TexCoords;"
                    "void main() {"
                    "    TexCoords = aTexCoords;"
                    "    gl_Position = vec4(aPos, 1.0);"
                    "}",
                    update_fragmentShaderSource);
    }

    const char* paint_fragmentShaderSource = 
        "in vec2 TexCoords;"
        "uniform int eraser;"
        "uniform int posCount;"
        "const int maxPosSize = 80;"
        "uniform vec2 positions[maxPosSize];"
        "uniform vec2 paintingRes;"
        "uniform vec2 videoScale;"
        "uniform sampler2D bgTxtr;"
        "uniform vec3 paint_color;"
        "struct Brush {"
        "    float radius;"
        "    float hardness;"
        "    float sizeJitter;"
        "    float fade;"
        "    int sinWavePattern;"
        "    float rotation;"
        "    float rotationJitter;"
        "    float spacing;"
        "    float scatter;"
        "    float alphaJitter;"
        "    int individualTexture;"
        "    sampler2D txtr;"
        "};"
        "uniform Brush brush;"
        "out vec4 outClr;"
        "void strokeBlendUniColor("
        "    float src, float srcA, float dst, out float color)"
        "{"
        "    color = dst + (1.0 - dst) * src;"
        "    if (color > srcA) {"
        "        color = max(dst, srcA);"
        "    }"
        "}"
        "void main() {"
        "    float ratio = paintingRes.x / paintingRes.y;"
        "    vec2 uv = vec2(TexCoords.x, TexCoords.y);"
        "    float hardnessV = min(brush.hardness, 1.0);"
        "    float radius = brush.radius * paintingRes.x;"
        "    vec4 orgTxtr = texture(bgTxtr, vec2(uv.x, uv.y));"
        "    vec4 fRes = orgTxtr;"
        "    for(int i = 0; i < min(posCount, maxPosSize); i++) {"
        "        float src = 1.0;"
        "        vec2 pos = positions[i] / videoScale * paintingRes;"
        "        float dist = length(uv * paintingRes - pos) / radius;"
        "        src *= smoothstep(1.0, hardnessV * max(0.1, 1.0 - (2.0 / (radius))), dist);"
        "        if(eraser == 0) {"
        "            vec4 prevFRes = fRes;"
        "            strokeBlendUniColor(src, 1.0, fRes.a, fRes.a);"
        "            src *= 25.0;"
        "            if(src > 1.0)"
        "                src = 1.0;"
        "            fRes.rgb = mix(fRes.rgb, paint_color, src);"
        "        } else {"
        "            fRes.a *= 1.0 - src;"
        "        }"
        "    }"
        "    vec4 res = vec4(fRes.rgb, fRes.a);"
        "    outClr = res;"
        "}";

    
    if(paint_program == 0){
        paint_program = LigidAPIUtil_load_shader_OpenGL(
                    "layout(location = 0) in vec3 aPos;"
                    "layout(location = 1) in vec2 aTexCoords;"
                    "out vec2 TexCoords;"
                    "void main() {"
                    "    TexCoords = aTexCoords;"
                    "    gl_Position = vec4(aPos, 1.0);"
                    "}",
                    paint_fragmentShaderSource);
    }
    
    int position_i = 0;
    int position_ii = 0;
    LigidAPI_vec2 positions[20][80];
    get_whole_stroke(positions, stroke, &position_i, &position_ii);
    
    for (size_t ii = 0; ii < position_ii+1; ii++)
    {
        int pos_count = 80;
        if(ii == position_ii)
            pos_count = position_i; 
            
        LigidAPIUtil_useProgram(paint_program);
        LigidAPIUtil_uniformTexture(paint_program, "bgTxtr", 0, canvas->opengl_texture_buffer_ID_copy);
        LigidAPIUtil_uniformint(paint_program, "eraser", 0); 
        LigidAPIUtil_uniform2f(paint_program, "paintingRes", canvas->width, canvas->height);
        LigidAPIUtil_uniform2f(paint_program, "videoScale", canvas->width, canvas->height);
        LigidAPIUtil_uniform3f(paint_program, "paint_color", color.r, color.g, color.b);

        LigidAPIUtil_uniform1f(paint_program, "brush.radius", brush.radius);
        LigidAPIUtil_uniform1f(paint_program, "brush.hardness", brush.hardness);
        LigidAPIUtil_uniform1f(paint_program, "brush.sizeJitter", 0.f);
        LigidAPIUtil_uniform1f(paint_program, "brush.fade", 0.f);
        LigidAPIUtil_uniformint(paint_program, "brush.sinWavePattern", 0);
        LigidAPIUtil_uniform1f(paint_program, "brush.rotation", 0.f);
        LigidAPIUtil_uniform1f(paint_program, "brush.rotationJitter", 0.f);
        LigidAPIUtil_uniform1f(paint_program, "brush.spacing", 0.f);
        LigidAPIUtil_uniform1f(paint_program, "brush.scatter", 0.f);
        LigidAPIUtil_uniform1f(paint_program, "brush.alphaJitter", 0.f);
        LigidAPIUtil_uniformint(paint_program, "brush.individualTexture", 0);

        LigidAPIUtil_uniformint(paint_program, "posCount", pos_count);
        
        char uniform_name[32];
        for (int i = 0; i < pos_count; i++) {
            // Construct the uniform name
            sprintf(uniform_name, "positions[%zu]", i);

            // Call the function with the constructed uniform name
            LigidAPIUtil_uniform2f(paint_program, uniform_name, positions[ii][i].x, positions[ii][i].y);
        }

        LigidAPIUtil_copyPixelData(canvas->opengl_texture_buffer_stroke_ID, canvas->opengl_texture_buffer_ID_copy, canvas->width, canvas->height);
        
        LigidAPIUtil_applyFilter(canvas->opengl_texture_buffer_stroke_ID, canvas->width, canvas->height);
    }
    
    LigidAPIUtil_clearTexture(canvas->display_opengl_texture_buffer_ID, canvas->width, canvas->height, 0.f,0.f,0.f,0.f);

    LigidAPIUtil_useProgram(update_program);
    LigidAPIUtil_uniformTexture(update_program, "bg_txtr", 0, canvas->opengl_texture_buffer_ID);
    LigidAPIUtil_uniformTexture(update_program, "stroke_txtr", 1, canvas->opengl_texture_buffer_stroke_ID);
    LigidAPIUtil_uniform1f(update_program, "opacity", brush.opacity);

    LigidAPIUtil_applyFilter(canvas->display_opengl_texture_buffer_ID, canvas->width, canvas->height);
    

    return 1;
} 

int LigidAPI_update_canvas(LigidCanvas* canvas, LigidBrush brush){

    if(update_program == 0)
        return 0;

    LigidAPIUtil_copyPixelData(canvas->opengl_texture_buffer_ID, canvas->opengl_texture_buffer_ID_copy, canvas->width, canvas->height);

    LigidAPIUtil_useProgram(update_program);
    LigidAPIUtil_uniformTexture(update_program, "bg_txtr", 0, canvas->opengl_texture_buffer_ID_copy);
    LigidAPIUtil_uniformTexture(update_program, "stroke_txtr", 1, canvas->opengl_texture_buffer_stroke_ID);
    LigidAPIUtil_uniform1f(update_program, "opacity", brush.opacity);

    LigidAPIUtil_applyFilter(canvas->opengl_texture_buffer_ID, canvas->width, canvas->height);

    LigidAPIUtil_clearTexture(canvas->opengl_texture_buffer_stroke_ID, canvas->width, canvas->height, 0.f,0.f,0.f,0.f);

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

LigidStroke LigidAPI_project_stroke_to_render_area(LigidStroke stroke, LigidArea render_area, LigidArea window_area){
    stroke.start_pos_x -= render_area.pos_x;
    stroke.start_pos_y -= render_area.pos_y;
    stroke.end_pos_x -= render_area.pos_x;
    stroke.end_pos_y -= render_area.pos_y;

    stroke.start_pos_x *= window_area.width / render_area.width;
    stroke.start_pos_y *= window_area.height / render_area.height;
    stroke.end_pos_x *= window_area.width / render_area.width;
    stroke.end_pos_y *= window_area.height / render_area.height;

    return stroke;
}