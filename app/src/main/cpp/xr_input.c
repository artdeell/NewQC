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

static XrActionSet actionSet = XR_NULL_HANDLE;
static XrAction palmPoseAction = XR_NULL_HANDLE;
static XrAction vibrateAction = XR_NULL_HANDLE;
static float vibrate[2] = {0, 0};
static XrPath handPaths[2];
static XrSpace handPoseSpace[2];
static XrActionStatePose handPoseState[2] = {{XR_TYPE_ACTION_STATE_POSE}, {XR_TYPE_ACTION_STATE_POSE}};
static XrPosef handPose[2] = {
        {{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.5f}},
        {{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.5f}}};

bool createActionSet() {
    XrActionSetCreateInfo actionSetCreateInfo = {XR_TYPE_ACTION_SET_CREATE_INFO};
    strncpy(actionSetCreateInfo.actionSetName, "main-action-set", XR_MAX_ACTION_SET_NAME_SIZE);
    strncpy(actionSetCreateInfo.localizedActionSetName, "Main Action Set", XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE);
    actionSetCreateInfo.priority = 0;
    XR_FAILRETURN(xrCreateActionSet(xrinfo.instance, &actionSetCreateInfo, &actionSet), false);
    return true;
}

bool createAction(XrAction xrAction, char name[], char localizedName[], XrActionType xrActionType, size_t subaction_count) {
    XrActionCreateInfo actionCreateInfo = {XR_TYPE_ACTION_CREATE_INFO};
    actionCreateInfo.actionType = xrActionType;
    strncpy(actionCreateInfo.actionName, name, XR_MAX_ACTION_NAME_SIZE);
    strncpy(actionCreateInfo.localizedActionName, localizedName, XR_MAX_LOCALIZED_ACTION_NAME_SIZE);
    actionCreateInfo.countSubactionPaths = subaction_count;
    actionCreateInfo.subactionPaths = handPaths;
    XR_FAILRETURN(xrCreateAction(actionSet, &actionCreateInfo, &xrAction), false);
    return true;
}

bool createDefaultActions() {
    XR_FAILRETURN(xrStringToPath(xrinfo.instance, "/user/hand/left", &handPaths[0]), false);
    XR_FAILRETURN(xrStringToPath(xrinfo.instance, "/user/hand/right", &handPaths[1]), false);
    createAction(palmPoseAction, "palm-pose", "Palm Pose",XR_ACTION_TYPE_POSE_INPUT, 2);
    createAction(vibrateAction, "vibrate", "Vibrate", XR_ACTION_TYPE_VIBRATION_OUTPUT, 2);
    return true;
}

bool suggestBindings(XrPath path, const XrActionSuggestedBinding bindings[], int binding_count) {
    XrInteractionProfileSuggestedBinding interactionProfileSuggestedBinding = {XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
    interactionProfileSuggestedBinding.interactionProfile = path;
    interactionProfileSuggestedBinding.suggestedBindings = bindings;
    interactionProfileSuggestedBinding.countSuggestedBindings = binding_count;
    XR_FAILRETURN(xrSuggestInteractionProfileBindings(xrinfo.instance, &interactionProfileSuggestedBinding), false);
    return true;
}

bool createSuggestedBindings() {
    XrPath posePath[2];
    XR_FAILRETURN(xrStringToPath(xrinfo.instance, "/user/hand/left/input/grip/pose", &posePath[0]), false);
    XR_FAILRETURN(xrStringToPath(xrinfo.instance, "/user/hand/right/input/grip/pose", &posePath[1]), false);

    XrPath vibratePath[2];
    XR_FAILRETURN(xrStringToPath(xrinfo.instance, "/user/hand/left/output/haptic", &vibratePath[0]), false);
    XR_FAILRETURN(xrStringToPath(xrinfo.instance, "/user/hand/right/output/haptic", &vibratePath[1]), false);

    const XrActionSuggestedBinding bindings[4] = {
        palmPoseAction, posePath[0],
        palmPoseAction, posePath[1],

        vibrateAction, vibratePath[0],
        vibrateAction, vibratePath[1]
    };

    XrPath controllerPath;
    XR_FAILRETURN(xrStringToPath(xrinfo.instance, "/interaction_profiles/khr/simple_controller", &controllerPath), false);
    suggestBindings(controllerPath, bindings, 4);
}

bool createActionPoseSpace(XrSession session, XrAction xrAction, XrSpace xrSpace, XrPath subactionPath) {
    const XrPosef xrPoseIdentity = {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}};
    XrActionSpaceCreateInfo actionSpaceCI = {XR_TYPE_ACTION_SPACE_CREATE_INFO};
    actionSpaceCI.action = xrAction;
    actionSpaceCI.poseInActionSpace = xrPoseIdentity;
    actionSpaceCI.subactionPath = subactionPath;

    XR_FAILRETURN(xrCreateActionSpace(session, &actionSpaceCI, &xrSpace), false);
    return true;
}

void createActionPoses() {
    if (!createActionPoseSpace(xrinfo.session, palmPoseAction, handPoseSpace[0], handPaths[0])) {
        LOGE("Failed to create hand pose space for left hand.");
    }

    if (!createActionPoseSpace(xrinfo.session, palmPoseAction,handPoseSpace[1], handPaths[1])) {
        LOGE("Failed to create hand pose space for right hand.");
    }
}

bool attachActionSet() {
    XrSessionActionSetsAttachInfo actionSetsAttachInfo = {XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
    actionSetsAttachInfo.countActionSets = 1;
    actionSetsAttachInfo.actionSets = &actionSet;
    XR_FAILRETURN(xrAttachSessionActionSets(xrinfo.session, &actionSetsAttachInfo), false);
    return true;
}

bool pollActions(XrTime predictedTime) {
    XrActiveActionSet activeActionSet = {};
    activeActionSet.actionSet = actionSet;
    activeActionSet.subactionPath = XR_NULL_PATH;
    XrActionsSyncInfo actionsSyncInfo = {XR_TYPE_ACTIONS_SYNC_INFO};
    actionsSyncInfo.countActiveActionSets = 1;
    actionsSyncInfo.activeActionSets = &activeActionSet;
    XR_FAILRETURN(xrSyncActions(xrinfo.session, &actionsSyncInfo), false);

    XrActionStateGetInfo actionStateGetInfo = {XR_TYPE_ACTION_STATE_GET_INFO};
    actionStateGetInfo.action = palmPoseAction;
    for (int i = 0; i < 2; i++) {
        actionStateGetInfo.subactionPath = handPaths[i];
        XR_FAILRETURN(xrGetActionStatePose(xrinfo.session, &actionStateGetInfo, &handPoseState[i]), false);
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
        XR_FAILRETURN(xrApplyHapticFeedback(xrinfo.session, &hapticActionInfo, (XrHapticBaseHeader *)&vibration), false);
    }

    return true;
}