typedef struct{
    float radius;
    float hardness;
    float opacity;
    float spacing;
    float sizeJitter;
    float scatter;
    float fade;
    float rotation;
    float rotationJitter;
    float alphaJitter;
    int sinWavePattern;
} LigidBrush;

typedef struct{
    float start_pos_x;
    float start_pos_y;
    float end_pos_x;
    float end_pos_y;
} LigidStroke;

typedef struct{
    float r;
    float g;
    float b;
    float a;
} LigidRGBA;

LigidBrush LigidAPI_create_brush(float radius, float hardness, float opacity, float spacing, float sizeJitter, float scatter, 
                            float fade, float rotation, float rotationJitter, float alphaJitter, int sinWavePattern); 

LigidRGBA LigidAPI_get_color(float r, float g, float b, float a);
LigidStroke LigidAPI_get_stroke(float start_pos_x, float start_pos_y, float end_pos_x, float end_pos_y);