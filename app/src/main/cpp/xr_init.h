//
// Created by maks on 03.12.2024.
//

#ifndef NEWQC_XR_INIT_H
#define NEWQC_XR_INIT_H
#include "xr_include.h"
#include <stdbool.h>
#include <jni.h>
#include <GLES3/gl3.h>

typedef struct {
    uint32_t width, height;
    XrSwapchain swapchain;
    GLuint swapchainTextureCount;
    GLuint* swapchainTextures;
} render_target_t;

typedef struct {
    XrInstance instance;
    XrSystemId systemId;
    XrSession session;
    XrSpace localReferenceSpace;
    XrViewConfigurationType configurationType;
    render_target_t renderTarget;
    uint32_t nViews;
    bool hasSession;
} xr_state_t;

typedef struct {
    JavaVM* applicationVm;
    jobject applicationActivity;
} android_jni_data_t;

extern xr_state_t xrinfo;

bool xriInitialize(android_jni_data_t* jniData);
bool xriInitSession();
bool xriStartSession();
void xriEndSession();
void xriFreeSession();
void xriFree();

#endif //NEWQC_XR_INIT_H
