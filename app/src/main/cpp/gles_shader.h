//
// Created by maks on 04.12.2024.
//

#ifndef NEWQC_GLES_SHADER_H
#define NEWQC_GLES_SHADER_H

#include <GLES2/gl2.h>
#include <stdbool.h>

typedef struct {
    GLenum pipelineStage;
    GLsizei sourceCount;
    const char* shaderSources[1];
} shader_source_t;

extern bool createProgram(GLuint* programId, size_t nShaders, shader_source_t* shaders);


#endif //NEWQC_GLES_SHADER_H
