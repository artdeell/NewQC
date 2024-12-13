//
// Created by maks on 05.12.2024.
//

#ifndef NEWQC_GL_SAFEPOINT_H
#define NEWQC_GL_SAFEPOINT_H

#define GL_SAFEPOINT do {while(glGetError() != 0) {}} while(0);

#define GL_RETURN(tv, fv, ...) do { \
GLenum error = glGetError();        \
if(error != GL_NO_ERROR) {          \
    LOGE(__VA_ARGS__);               \
    return fv;\
}else {                             \
    return tv;                      \
}\
} while(0)

#endif //NEWQC_GL_SAFEPOINT_H
