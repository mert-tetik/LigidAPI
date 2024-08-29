// #include <GL/GL.h>
// #include <GL/GLU.h>

// void LigidAPIUtil_load_shader_OpenGL(const char* vertexCode, const char* fragmentCode){
    
//     gl_clean_errors();
    
//     const char* vShaderCode = vertexCode;
//     const char * fShaderCode = fragmentCode;
    
//     // 2. compile shaders
//     unsigned int vertex, fragment;

//     // vertex shader
//     vertex = glCreateShader(GL_VERTEX_SHADER);
//     gl_test_error("vertex = glCreateShader(GL_VERTEX_SHADER);");
//     glShaderSource(vertex, 1, &vShaderCode, NULL);
//     gl_test_error("glShaderSource(vertex, 1, &vShaderCode, NULL);");
//     glCompileShader(vertex);
//     gl_test_error("glCompileShader(vertex);");
//     checkCompileErrors(vertex, "VERTEX");
    
//     // fragment Shader
//     fragment = glCreateShader(GL_FRAGMENT_SHADER);
//     gl_test_error("fragment = glCreateShader(GL_FRAGMENT_SHADER);");
//     glShaderSource(fragment, 1, &fShaderCode, NULL);
//     gl_test_error("glShaderSource(fragment, 1, &fShaderCode, NULL);");
//     glCompileShader(fragment);
//     gl_test_error("glCompileShader(fragment);");
//     checkCompileErrors(fragment, "FRAGMENT");

//     // shader Program
//     ID = glCreateProgram();
//     gl_test_error("ID = glCreateProgram();");
//     glAttachShader(ID, vertex);
//     gl_test_error("glAttachShader(ID, vertex);");
//     glAttachShader(ID, fragment);
//     gl_test_error("glAttachShader(ID, fragment);");

//     glLinkProgram(ID);
//     gl_test_error("glLinkProgram(ID);");
//     checkCompileErrors(ID, "PROGRAM");
//     // delete the shaders as they're linked into our program now and no longer necessary
//     glDeleteShader(vertex);
//     gl_test_error("glDeleteShader(vertex);");
//     glDeleteShader(fragment);
//     gl_test_error("glDeleteShader(fragment);");
// }