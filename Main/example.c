#include <stdio.h>
#include <string.h>

#include "./Headers/Canvas.h"

int main(){
    printf("START \n");

    LigidCanvas* canvas = LigidAPI_create_canvas(1920, 1080, 4);
    
    LigidBrush brush = LigidAPI_create_brush(0.01f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0);
    LigidAPI_paint_canvas(canvas, brush, LigidAPI_get_stroke(0.5f, 0.5f, 0.7f, 0.7f), LigidAPI_get_color(0.7f, 0.f, 0.3f, 1.f));    

    LigidAPI_delete_canvas(canvas);

    printf("FINISH \n");

    return 1;
}