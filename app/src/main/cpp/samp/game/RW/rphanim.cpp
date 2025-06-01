
#include "rphanim.h"
#include "game/common.h"
#include "../vendor/armhook/patch.h"

void RpHAnimHierarchySetFreeListCreateParams(RwInt32 blockSize, RwInt32 numBlocksToPrealloc) {
    CHook::CallFunction<void>(g_libGTASA + 0x254B18, blockSize, numBlocksToPrealloc);
}

RpHAnimHierarchy* RpHAnimHierarchyCreate(RwInt32 numNodes, RwUInt32* nodeFlags, RwInt32* nodeIDs, RpHAnimHierarchyFlag flags, RwInt32 maxInterpKeyFrameSize) {
    return CHook::CallFunction<RpHAnimHierarchy*>(g_libGTASA + 0x255390, numNodes, nodeFlags, nodeIDs, flags, maxInterpKeyFrameSize);
}

RpHAnimHierarchy* RpHAnimHierarchyCreateFromHierarchy(RpHAnimHierarchy* hierarchy, RpHAnimHierarchyFlag flags, RwInt32 maxInterpKeyFrameSize) {
    return CHook::CallFunction<RpHAnimHierarchy*>(g_libGTASA + 0x25570C, hierarchy, flags, maxInterpKeyFrameSize);
}

RpHAnimHierarchy* RpHAnimHierarchyDestroy(RpHAnimHierarchy* hierarchy) {
    return CHook::CallFunction<RpHAnimHierarchy*>(g_libGTASA + 0x2554D4, hierarchy);
}

RpHAnimHierarchy* RpHAnimHierarchyCreateSubHierarchy(RpHAnimHierarchy* parentHierarchy, RwInt32 startNode, RpHAnimHierarchyFlag flags, RwInt32 maxInterpKeyFrameSize) {
    return CHook::CallFunction<RpHAnimHierarchy*>(g_libGTASA + 0x255580, parentHierarchy, startNode, flags, maxInterpKeyFrameSize);
}

RpHAnimHierarchy* RpHAnimHierarchyAttach(RpHAnimHierarchy* hierarchy) {
    return CHook::CallFunction<RpHAnimHierarchy*>(g_libGTASA + 0x2558A0, hierarchy);
}

RpHAnimHierarchy* RpHAnimHierarchyDetach(RpHAnimHierarchy* hierarchy) {
    return CHook::CallFunction<RpHAnimHierarchy*>(g_libGTASA + 0x2559C0, hierarchy);
}

RpHAnimHierarchy* RpHAnimHierarchyAttachFrameIndex(RpHAnimHierarchy* hierarchy, RwInt32 nodeIndex) {
    return CHook::CallFunction<RpHAnimHierarchy*>(g_libGTASA + 0x2559F4, hierarchy, nodeIndex);
}

RpHAnimHierarchy* RpHAnimHierarchyDetachFrameIndex(RpHAnimHierarchy* hierarchy, RwInt32 nodeIndex) {
    return CHook::CallFunction<RpHAnimHierarchy*>(g_libGTASA + 0x255B54, hierarchy, nodeIndex);
}

RwBool RpHAnimFrameSetHierarchy(RwFrame* frame, RpHAnimHierarchy* hierarchy) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x255B70, frame, hierarchy);
}

RpHAnimHierarchy* RpHAnimFrameGetHierarchy(RwFrame* frame) {
    return CHook::CallFunction<RpHAnimHierarchy*>(g_libGTASA + 0x255BA0, frame);
}

RwMatrix* RpHAnimHierarchyGetMatrixArray(RpHAnimHierarchy* hierarchy) {
    return CHook::CallFunction<RwMatrix*>(g_libGTASA + 0x255B68, hierarchy);
}

RwBool RpHAnimHierarchyUpdateMatrices(RpHAnimHierarchy* hierarchy) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x255C20, hierarchy);//
}

RwInt32 RpHAnimIDGetIndex(RpHAnimHierarchy* hierarchy, RwInt32 ID) {
    return CHook::CallFunction<RwInt32>(g_libGTASA + 0x255BE8, hierarchy, ID);
}

RwBool RpHAnimPluginAttach() {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x254B2C);
}

void RpHAnimKeyFrameApply(void* matrix, void* voidIFrame) {
    CHook::CallFunction<void>(g_libGTASA + 0x256440, matrix, voidIFrame);
}

void RpHAnimKeyFrameBlend(void* voidOut, void* voidIn1, void* voidIn2, RwReal alpha) {
    CHook::CallFunction<void>(g_libGTASA + 0x25694C, voidOut, voidIn1, voidIn2, alpha);
}

void RpHAnimKeyFrameInterpolate(void* voidOut, void* voidIn1, void* voidIn2, RwReal time, void* customData) {
    CHook::CallFunction<void>(g_libGTASA + 0x256500, voidOut, voidIn1, voidIn2, time, customData);
}

void RpHAnimKeyFrameAdd(void* voidOut, void* voidIn1, void* voidIn2) {
    CHook::CallFunction<void>(g_libGTASA + 0x256FB8, voidOut, voidIn1, voidIn2);
}

void RpHAnimKeyFrameMulRecip(void* voidFrame, void* voidStart) {
    CHook::CallFunction<void>(g_libGTASA + 0x256EFC, voidFrame, voidStart);
}

RtAnimAnimation* RpHAnimKeyFrameStreamRead(RwStream* stream, RtAnimAnimation* animation) {
    return CHook::CallFunction<RtAnimAnimation*>(g_libGTASA + 0x256D84, stream, animation);
}

RwBool RpHAnimKeyFrameStreamWrite(const RtAnimAnimation* animation, RwStream* stream) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x256E58, animation, stream);
}

RwInt32 RpHAnimKeyFrameStreamGetSize(const RtAnimAnimation* animation) {
    return CHook::CallFunction<RwInt32>(g_libGTASA + 0x256EEC, animation);
}

RwBool RpHAnimFrameSetID(RwFrame* frame, RwInt32 id) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x255BB8, frame, id);
}

RwInt32 RpHAnimFrameGetID(RwFrame* frame) {
    return CHook::CallFunction<RwInt32>(g_libGTASA + 0x255BD4, frame);
}
