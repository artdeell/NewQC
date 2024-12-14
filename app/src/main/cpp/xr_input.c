//
// Created by CADIndie on 12/13/2024.
//

#include "xr_input.h"
#include "xr_include.h"
#include "xr_init.h"
#include <string.h>
#include <stdlib.h>

#define LOG_TAG __FILE_NAME__
#include "log.h"

xr_input_t xrInput;

XrActionSet actionSet;
XrAction palmPoseAction;
XrAction vibrateAction;
float vibrate[2] = {0, 0};
XrPath handPaths[2] = {0, 0};
XrSpace handPoseSpace[2];
XrActionStatePose handPoseState[2] = {{XR_TYPE_ACTION_STATE_POSE}, {XR_TYPE_ACTION_STATE_POSE}};
XrPosef handPose[2] = {
        {{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.5f}},
        {{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.5f}}};

XrPath createXrPath(const char path_string[]) {
    XrPath xrPath;
    XrResult result = xrStringToPath(xrinfo.instance, path_string, &xrPath);
    if (result != XR_SUCCESS) {
        LOGE("Failed to create XrPath: %i", result);
    }
    return xrPath;
}

bool createActionSet() {
    XrActionSetCreateInfo actionSetCreateInfo = {XR_TYPE_ACTION_SET_CREATE_INFO};
    strncpy(actionSetCreateInfo.actionSetName, "main-action-set", XR_MAX_ACTION_SET_NAME_SIZE);
    strncpy(actionSetCreateInfo.localizedActionSetName, "Main Action Set", XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE);
    actionSetCreateInfo.priority = 0;
    return OPENXR_CHECK(xrCreateActionSet(xrinfo.instance, &actionSetCreateInfo, &xrInput.actionSet), "Failed to create action set: %i");
}

void createAction(XrAction xrAction, char name[], char localizedName[], XrActionType xrActionType, size_t subaction_count) {
    XrActionCreateInfo actionCreateInfo = {XR_TYPE_ACTION_CREATE_INFO};
    actionCreateInfo.actionType = xrActionType;
    strncpy(actionCreateInfo.actionName, name, XR_MAX_ACTION_NAME_SIZE);
    strncpy(actionCreateInfo.localizedActionName, localizedName, XR_MAX_LOCALIZED_ACTION_NAME_SIZE);
    actionCreateInfo.countSubactionPaths = subaction_count;
    actionCreateInfo.subactionPaths = handPaths;
    OPENXR_CHECK(xrCreateAction(actionSet, &actionCreateInfo, &xrAction), "Failed to create action: %i");
}

void createDefaultActions() {
    createAction(palmPoseAction, "palm-pose", "Palm Pose",XR_ACTION_TYPE_POSE_INPUT, 2);
    createAction(vibrateAction, "vibrate", "Vibrate", XR_ACTION_TYPE_VIBRATION_OUTPUT, 2);
    handPaths[0] = createXrPath("/user/hand/left");
    handPaths[1] = createXrPath("/user/hand/right");
}

bool suggestBindings(const char profile_path[], XrActionSuggestedBinding bindings[], size_t binding_count) {
    XrInteractionProfileSuggestedBinding interactionProfileSuggestedBinding = {XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
    interactionProfileSuggestedBinding.interactionProfile = createXrPath(profile_path);
    interactionProfileSuggestedBinding.suggestedBindings = bindings;
    interactionProfileSuggestedBinding.countSuggestedBindings = (uint32_t)binding_count;
    return OPENXR_CHECK(xrSuggestInteractionProfileBindings(xrinfo.instance, &interactionProfileSuggestedBinding), "Failed to suggest bindings for profile path: %s");
}

void createSuggestedBindings() {
    XrActionSuggestedBinding bindings[] = {
        palmPoseAction, createXrPath("/user/hand/left/input/grip/pose"),
        palmPoseAction, createXrPath("/user/hand/right/input/grip/pose"),

        vibrateAction, createXrPath("/user/hand/left/output/haptic"),
        vibrateAction, createXrPath("/user/hand/right/output/haptic")
    };

    size_t binding_count = sizeof(bindings) / sizeof(bindings[0]);
    // TODO: Support the Khronos Simple Controller (missing floating point data)
    const char profile_path[] = "/interaction_profiles/oculus/touch_controller";

    if (!suggestBindings(profile_path, bindings, binding_count)) {
        LOGE("Failed to suggest bindings.");
    }
}

bool createActionPoseSpace(XrSession session, XrAction xrAction, const char subaction_path[], XrSpace xrSpace) {
    const XrPosef xrPoseIdentity = {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}};
    XrActionSpaceCreateInfo actionSpaceCI = {XR_TYPE_ACTION_SPACE_CREATE_INFO};
    actionSpaceCI.action = xrAction;
    actionSpaceCI.poseInActionSpace = xrPoseIdentity;

    if (subaction_path) {
        actionSpaceCI.subactionPath = createXrPath(subaction_path);
    }

    return OPENXR_CHECK(xrCreateActionSpace(session, &actionSpaceCI, &xrSpace), "Failed to create ActionSpace: %d");
}

void createActionPoses() {
    if (!createActionPoseSpace(xrinfo.session, palmPoseAction, "/user/hand/left",
                               handPoseSpace[0])) {
        LOGE("Failed to create hand pose space for left hand.");
    }

    if (!createActionPoseSpace(xrinfo.session, palmPoseAction, "/user/hand/right",
                               handPoseSpace[1])) {
        LOGE("Failed to create hand pose space for right hand.");
    }
}

bool attachActionSet() {
    XrSessionActionSetsAttachInfo actionSetsAttachInfo = {XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
    actionSetsAttachInfo.countActionSets = 1;
    actionSetsAttachInfo.actionSets = &actionSet;
    return OPENXR_CHECK(xrAttachSessionActionSets(xrinfo.session, &actionSetsAttachInfo), "Failed to attach action set to session: %i");
}

bool pollActions(XrTime predictedTime) {
    XrActiveActionSet activeActionSet = {};
    activeActionSet.actionSet = actionSet;
    activeActionSet.subactionPath = XR_NULL_PATH;
    XrActionsSyncInfo actionsSyncInfo = {XR_TYPE_ACTIONS_SYNC_INFO};
    actionsSyncInfo.countActiveActionSets = 1;
    actionsSyncInfo.activeActionSets = &activeActionSet;
    if(!OPENXR_CHECK(xrSyncActions(xrinfo.session, &actionsSyncInfo), "Failed to sync actions: %i")) return false;

    XrActionStateGetInfo actionStateGetInfo = {XR_TYPE_ACTION_STATE_GET_INFO};
    actionStateGetInfo.action = palmPoseAction;
    for (int i = 0; i < 2; i++) {
        actionStateGetInfo.subactionPath = handPaths[i];
        if (!OPENXR_CHECK(xrGetActionStatePose(xrinfo.session, &actionStateGetInfo, &handPoseState[i]), "Failed to get pose state: %i"))
            return false;
        if (handPoseState[i].isActive) {
            XrSpaceLocation spaceLocation = {XR_TYPE_SPACE_LOCATION};
            XrResult res = xrLocateSpace(handPoseSpace[i], xrinfo.localReferenceSpace, predictedTime, &spaceLocation);
            if (XR_UNQUALIFIED_SUCCESS(res) &&
                (spaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0 &&
                (spaceLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0) {
                handPose[i] = spaceLocation.pose;
            } else {
                handPoseState[i].isActive = false;
            }
        }
    }

    for (int i = 0; i < 2; i++) {
        vibrate[i] *= 0.5f;
        if (vibrate[i] < 0.01f)
            vibrate[i] = 0.0f;
        XrHapticVibration vibration = {XR_TYPE_HAPTIC_VIBRATION};
        vibration.amplitude = vibrate[i];
        vibration.duration = XR_MIN_HAPTIC_DURATION;
        vibration.frequency = XR_FREQUENCY_UNSPECIFIED;

        XrHapticActionInfo hapticActionInfo = {XR_TYPE_HAPTIC_ACTION_INFO};
        hapticActionInfo.action = vibrateAction;
        hapticActionInfo.subactionPath = handPaths[i];
        if(!OPENXR_CHECK(xrApplyHapticFeedback(xrinfo.session, &hapticActionInfo, (XrHapticBaseHeader *)&vibration), "Failed to apply haptic feedback: %i"))
            return false;
    }

    return true;
}