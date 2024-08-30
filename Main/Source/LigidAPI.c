#include <stdio.h>
#include <string.h>

#include "../Headers/LigidAPI.h"

#include <vulkan/vulkan.h>

int LIGIDAPI_OPENGL = 1;
int LIGIDAPI_VULKAN = 0;
const char* LIGIDAPI_RENDER_API_VERSION = "#version 330 core \n";

int LigidAPI_init(const char* render_API, const char* render_API_version){
    if(render_API == "OPENGL"){
        LIGIDAPI_OPENGL = 1;
        LIGIDAPI_VULKAN = 0;

        LIGIDAPI_RENDER_API_VERSION = render_API_version;
        return LIGIDAPI_INIT_SUCCESS;
    }
    /*
    else if(render_API == "VULKAN"){
        LIGIDAPI_OPENGL = 0;
        LIGIDAPI_VULKAN = 1;
    }
    */
    else{
        return LIGIDAPI_INIT_INVALID_RENDER_API;
    }

    return LIGIDAPI_INIT_INVALID_RENDER_API;
}