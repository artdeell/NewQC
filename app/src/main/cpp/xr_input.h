//
// Created by CADIndie on 12/13/2024.
//

#ifndef NEWQC_XR_INPUT_H
#define NEWQC_XR_INPUT_H
#include "xr_include.h"
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

#endif //NEWQC_XR_INPUT_H