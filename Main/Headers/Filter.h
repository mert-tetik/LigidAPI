#ifndef LIGIDAPI_FILTER_H
#define LIGIDAPI_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct{
    const char* shader;
    unsigned int ID;
    
    float variable_1;
    float variable_2;
    float variable_3;
    float variable_4;
    float variable_5;

} LigidFilter;

/*!
    variable_1 = "1." if invert red channel
    variable_2 = "1." if invert green channel
    variable_3 = "1." if invert blue channel
    variable_4 = "1." if invert alpha channel
*/
extern LigidFilter LigidAPI_filter_invert;
/*!
    variable_1 = brightness -1.f - 1.f
    variable_1 = contrast -1.f - 1.f
*/
extern LigidFilter LigidAPI_filter_brightness;

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
                                    );


#ifdef __cplusplus
}
#endif


#endif