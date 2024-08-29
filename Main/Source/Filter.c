#include <glad/glad.h>

#include "../Headers/Filter.h"
#include "../Headers/OpenGL_specific.h"

LigidFilter LigidAPI_filter_invert = {
                                        "in vec2 TexCoords;"
                                        "uniform sampler2D txtr;"
                                        "uniform float variable_1;"
                                        "uniform float variable_2;"
                                        "uniform float variable_3;"
                                        "uniform float variable_4;"
                                        "uniform float variable_5;"
                                        "out vec4 fragColor;"
                                        "void main() {"
                                        "    vec4 src = texture(txtr, vec2(TexCoords.x, TexCoords.y));"
                                        "    fragColor = src;"
                                        "   if(variable_1 == 1.)"
                                        "       fragColor.r = 1. - src.r;"
                                        "   if(variable_2 == 1.)"
                                        "       fragColor.g = 1. - src.g;"
                                        "   if(variable_3 == 1.)"
                                        "       fragColor.b = 1. - src.b;"
                                        "   if(variable_4 == 1.)"
                                        "       fragColor.a = 1. - src.a;"
                                        "}", 
                                        0,0,0,0,0,0
                                    };

int LigidAPI_apply_filter(
                                        unsigned int texture, 
                                        unsigned int width, 
                                        unsigned int height, 
                                        unsigned int filter_texture, 
                                        unsigned int filter_width, 
                                        unsigned int filter_height, 
                                        LigidFilter* filter, 
                                        float variable_1, 
                                        float variable_2, 
                                        float variable_3, 
                                        float variable_4, 
                                        float variable_5
                                    )
{
    if(filter->ID == 0){
        filter->ID = LigidAPIUtil_load_shader_OpenGL(
                    "layout(location = 0) in vec3 aPos;"
                    "layout(location = 1) in vec2 aTexCoords; "
                    "out vec2 TexCoords;  "
                    "void main() {"
                    "    TexCoords = aTexCoords;  "
                    "    gl_Position = vec4(aPos, 1.0);"
                    "}"
                    ,filter->shader);
    }

    glUseProgram(filter->ID);
    glUniform1i(glGetUniformLocation(filter->ID, "txtr"), 0);
    glUniform1f(glGetUniformLocation(filter->ID, "variable_1"), variable_1);
    glUniform1f(glGetUniformLocation(filter->ID, "variable_2"), variable_2);
    glUniform1f(glGetUniformLocation(filter->ID, "variable_3"), variable_3);
    glUniform1f(glGetUniformLocation(filter->ID, "variable_4"), variable_4);
    glUniform1f(glGetUniformLocation(filter->ID, "variable_5"), variable_5);

    return LigidAPIUtil_applyFilter(texture, filter->ID, width, height, filter_texture, filter_width, filter_height);
}