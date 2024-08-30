
#ifdef LIGIDAPI_OPENGL_IMPLEMENTATION

#ifdef __cplusplus
extern "C" {
#endif

    void LigidAPIUtil_checkShaderCompileErrors(unsigned int shader, const char* type)
    {
        GLint success;
        GLchar infoLog[1024];
        if(type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if(!success)
            {
                glGetShaderInfoLog(shader, 1024, 0, infoLog);
                printf("ERROR::SHADER_COMPILATION_ERROR");
                printf(type);
                printf("\n");
                printf(infoLog);
                printf("\n");
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if(!success)
            {
                glGetProgramInfoLog(shader, 1024, 0, infoLog);
                printf("ERROR::PROGRAM_LINKING_ERROR");
                printf(type);
                printf("\n");
                printf(infoLog);
                printf("\n");
            }
        }
    }

    static const char* ligidapi_add_to_strings(const char* str1, const char* str2) {
        // Calculate the total length required (+1 for the null terminator)
        size_t len1 = strlen(str1);
        size_t len2 = strlen(str2);
        size_t totalLength = len1 + len2 + 1;

        // Allocate memory for the new string
        char* result = (char*)malloc(totalLength * sizeof(char));
        if (result == NULL) {
            // Handle memory allocation failure
            printf("Memory allocation failed!\n");
            return NULL;
        }

        // Copy the first string into the new memory
        strcpy(result, str1);

        // Concatenate the second string
        strcat(result, str2);

        return result;
    }

    extern const char* LIGIDAPI_RENDER_API_VERSION;

    unsigned int LigidAPIUtil_load_shader_OpenGL(const char* vertexCode, const char* fragmentCode){
        
        const char* vShaderCode = ligidapi_add_to_strings(LIGIDAPI_RENDER_API_VERSION, vertexCode);
        const char * fShaderCode = ligidapi_add_to_strings(LIGIDAPI_RENDER_API_VERSION, fragmentCode);
        
        // 2. compile shaders
        unsigned int vertex, fragment;

        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, 0);
        glCompileShader(vertex);
        LigidAPIUtil_checkShaderCompileErrors(vertex, "VERTEX");
        
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, 0);
        glCompileShader(fragment);
        LigidAPIUtil_checkShaderCompileErrors(fragment, "FRAGMENT");

        // shader Program
        unsigned int ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);

        glLinkProgram(ID);
        LigidAPIUtil_checkShaderCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);

        return ID;
    }

    int LigidAPIUtil_applyFilter(
                                            unsigned int texture, 
                                            unsigned int shaderProgram, 
                                            unsigned int width, 
                                            unsigned int height, 
                                            unsigned int filter_texture, 
                                            unsigned int filter_width, 
                                            unsigned int filter_height
                                        ) 
    {
        // Create a framebuffer
        GLuint framebuffer;
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        // Attach the texture to the framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, filter_texture, 0);

        // Check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            printf("Framebuffer is not complete!\n");
            return 0;
        }

        // Bind the shader program
        glUseProgram(shaderProgram);

        // Define a full-screen quad (two triangles)
        float quadVertices[] = {
            // Positions      // TexCoords
            -1.0f, -1.0f,     0.0f, 0.0f,
            1.0f, -1.0f,     1.0f, 0.0f,
            1.0f,  1.0f,     1.0f, 1.0f,
            
            -1.0f, -1.0f,     0.0f, 0.0f,
            1.0f,  1.0f,     1.0f, 1.0f,
            -1.0f,  1.0f,     0.0f, 1.0f
        };

        // Create a VAO and VBO for the quad
        GLuint VAO, VBO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Texture coordinate attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Render the quad
        glViewport(0, 0, filter_width, filter_height);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Unbind the framebuffer to return to the default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Cleanup
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteFramebuffers(1, &framebuffer);

        return 1;
    }

    unsigned int LigidAPIUtil_createTexture(unsigned char* pxs, int width, int height, int channels)
    {
        unsigned int texture;
        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        // Setup filtering parameters for display
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Upload pixels into texture
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

        int channel_val;
        int internal_val;
        if(channels == 1){
            channel_val = GL_RED;
            internal_val = GL_R8;
        }
        if(channels == 2){
            channel_val = GL_RG;
            internal_val = GL_RG8;
        }
        if(channels == 3){
            channel_val = GL_RGB;
            internal_val = GL_RGB8;
        }
        if(channels == 4){
            channel_val = GL_RGBA;
            internal_val = GL_RGBA8;
        }
        else{
            channel_val = GL_RGBA;
            internal_val = GL_RGBA8;
        }
        
        glTexImage2D(GL_TEXTURE_2D, 0, internal_val, width, height, 0, channel_val, GL_UNSIGNED_BYTE, pxs);

        return texture;
    }

    void LigidAPIUtil_clearTexture(unsigned int texture, int width, int height, float r, float g, float b, float a){
        GLuint framebuffer;
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        // Attach the texture to the framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        glViewport(0, 0, width, height);
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT);

        // Unbind the framebuffer to return to the default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // Cleanup
        glDeleteFramebuffers(1, &framebuffer);
    }

    void LigidAPIUtil_useProgram(unsigned int program){
        glUseProgram(program);
    }

    void LigidAPIUtil_uniformTexture(unsigned int program, const char* location, int slot, unsigned int texture){
        glUniform1i(glGetUniformLocation(program, location), slot); glActiveTexture(GL_TEXTURE0 + slot); glBindTexture(GL_TEXTURE_2D, texture);
    }
    void LigidAPIUtil_uniformint(unsigned int program, const char* location, int value){
        glUniform1i(glGetUniformLocation(program, location), value);
    }
    void LigidAPIUtil_uniform1f(unsigned int program, const char* location, float value){
        glUniform1f(glGetUniformLocation(program, location), value);
    }
    void LigidAPIUtil_uniform2f(unsigned int program, const char* location, float value1, float value2){
        glUniform2f(glGetUniformLocation(program, location), value1, value2);
    }
    void LigidAPIUtil_uniform3f(unsigned int program, const char* location, float value1, float value2, float value3){
        glUniform3f(glGetUniformLocation(program, location), value1, value2, value3);
    }

    void LigidAPIUtil_copyPixelData(unsigned int from, unsigned int to, int width, int height){
        //Copy the texture
        unsigned int duplicate_FBO;
        glGenFramebuffers(1, &duplicate_FBO);
        
        glBindFramebuffer(GL_FRAMEBUFFER, duplicate_FBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, from, 0);

        // Bind the requested texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, to);

        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, width, height, 0);
        glDeleteFramebuffers(1, &duplicate_FBO);
    }

    #else

    void LigidAPIUtil_checkShaderCompileErrors(unsigned int shader, const char* type);
    unsigned int LigidAPIUtil_load_shader_OpenGL(const char* vertexCode, const char* fragmentCode);
    int LigidAPIUtil_applyFilter(unsigned int texture, unsigned int shaderProgram, unsigned int width, unsigned int height, unsigned int filter_texture, unsigned int filter_width, unsigned int filter_height);
    unsigned int LigidAPIUtil_createTexture(unsigned char* pxs, int width, int height, int channels);
    void LigidAPIUtil_clearTexture(unsigned int texture, int width, int height, float r, float g, float b, float a);

    void LigidAPIUtil_useProgram(unsigned int program);
    void LigidAPIUtil_uniformTexture(unsigned int program, const char* location, int slot, unsigned int texture);
    void LigidAPIUtil_uniformint(unsigned int program, const char* location, int value);
    void LigidAPIUtil_uniform1f(unsigned int program, const char* location, float value);
    void LigidAPIUtil_uniform2f(unsigned int program, const char* location, float value1, float value2);
    void LigidAPIUtil_uniform3f(unsigned int program, const char* location, float value1, float value2, float value3);

    void LigidAPIUtil_copyPixelData(unsigned int from, unsigned int to, int width, int height);

    #endif

#ifdef __cplusplus
}
#endif