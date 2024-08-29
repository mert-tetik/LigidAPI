#ifndef LIGIDAPI_H
#define LIGIDAPI_H


//------------------------------

#ifdef __cplusplus
extern "C" {
#endif

#define LIGIDAPI_INIT_SUCCESS 0 
#define LIGIDAPI_INIT_INVALID_RENDER_API 1 
#define LIGIDAPI_INIT_INVALID_RENDER_API_VERSION 2 
/*!
    Initialize LigidAPI.

    Must be called before calling other LigidAPI functions.

    @param render_API either "OPENGL" or "VULKAN"
    @param render_API_version (for OpenGL = "#version 330 core \n" (version must be between 330 & 460) (DON'T FORGET TO PUT \n ))

    @return LIGIDAPI_INIT_SUCCESS, LIGIDAPI_INIT_INVALID_RENDER_API, LIGIDAPI_INIT_INVALID_RENDER_API_VERSION

    If render api is not set correctly it'll be automatically assigned as OPENGL.
    If render api version is not set correctly it'll be set to minimum version.
*/
int LigidAPI_init(const char* render_API, const char* render_API_version);


#ifdef __cplusplus
}
#endif

//------------------------------

#endif // LIGIDAPI_H