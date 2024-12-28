//
// Created by CADIndie on 12/13/2024.
//

#ifndef NEWQC_XR_INPUT_H
#define NEWQC_XR_INPUT_H
#include "xr_include.h"
#include "xr_linear_algebra.h"
#include <stdbool.h>


typedef struct {
    XrPosef handPose[2];
} xr_input_t;

extern xr_input_t xrInput;

bool pollActions(XrTime predictedTime);
bool attachActionSet();
bool createActionSet();
bool createDefaultActions();
bool createSuggestedBindings();
void createActionPoses();

bool rayIntersectsScreen(XrVector3f rayOrigin, XrVector3f rayVector, XrVector3f* outIntersectionPoint);
bool normalizeVectorToScreen(XrVector2f* out, XrVector3f point);
void getControllerRay(int controller, XrMatrix4x4f model, XrVector3f* startOut, XrVector3f* endOut);
bool rayIntersectsTriangle(XrVector3f rayOrigin, XrVector3f rayVector, XrVector3f triangle[3], XrVector3f* outIntersectionPoint);

#endif //NEWQC_XR_INPUT_H