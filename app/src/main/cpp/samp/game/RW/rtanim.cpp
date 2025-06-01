#include "RenderWare.h"
#include "game/common.h"
#include "../vendor/armhook/patch.h"

void RtAnimAnimationFreeListCreateParams(RwInt32 blockSize, RwInt32 numBlocksToPrealloc) {
    CHook::CallFunction<void>(g_libGTASA + 0x287D38, blockSize, numBlocksToPrealloc);
}

RwBool RtAnimInitialize() {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x287D4C);
}

RwBool RtAnimRegisterInterpolationScheme(RtAnimInterpolatorInfo* interpolatorInfo) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x287E1C, interpolatorInfo);
}

RtAnimInterpolatorInfo* RtAnimGetInterpolatorInfo(RwInt32 typeID) {
    return CHook::CallFunction<RtAnimInterpolatorInfo*>(g_libGTASA + 0x287EB0, typeID);
}

RtAnimAnimation* RtAnimAnimationCreate(RwInt32 typeID, RwInt32 numFrames, RwInt32 flags, RwReal duration) {
    return CHook::CallFunction<RtAnimAnimation*>(g_libGTASA + 0x287F04, typeID, numFrames, flags, duration);
}

RwBool RtAnimAnimationDestroy(RtAnimAnimation* animation) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x287FE8, animation);
}

RtAnimAnimation* RtAnimAnimationRead(const RwChar* filename) {
    return CHook::CallFunction<RtAnimAnimation*>(g_libGTASA + 0x288010, filename);
}

RwBool RtAnimAnimationWrite(const RtAnimAnimation* animation, const RwChar* filename) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x28824C, animation, filename);
}

RtAnimAnimation* RtAnimAnimationStreamRead(RwStream* stream) {
    return CHook::CallFunction<RtAnimAnimation*>(g_libGTASA + 0x288080, stream);
}

RwBool RtAnimAnimationStreamWrite(const RtAnimAnimation* animation, RwStream* stream) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x2882A0, animation, stream);
}

RwInt32 RtAnimAnimationStreamGetSize(const RtAnimAnimation* animation) {
    return CHook::CallFunction<RwInt32>(g_libGTASA + 0x288374, animation);
}

RwUInt32 RtAnimAnimationGetNumNodes(const RtAnimAnimation* animation) {
    return CHook::CallFunction<RwUInt32>(g_libGTASA + 0x288394, animation);
}

RtAnimInterpolator* RtAnimInterpolatorCreate(RwInt32 numNodes, RwInt32 maxInterpKeyFrameSize) {
    return CHook::CallFunction<RtAnimInterpolator*>(g_libGTASA + 0x2883D8, numNodes, maxInterpKeyFrameSize);
}

void RtAnimInterpolatorDestroy(RtAnimInterpolator* anim) {
    CHook::CallFunction<void>(g_libGTASA + 0x28844C, anim);
}

RwBool RtAnimInterpolatorSetCurrentAnim(RtAnimInterpolator* animI, RtAnimAnimation* anim) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x288460, animI, anim);
}

RwBool RtAnimInterpolatorSetKeyFrameCallBacks(RtAnimInterpolator* anim, RwInt32 keyFrameTypeID) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x288588, anim, keyFrameTypeID);
}

void RtAnimInterpolatorSetAnimLoopCallBack(RtAnimInterpolator* anim, RtAnimCallBack callBack, void* data) {
    CHook::CallFunction<void>(g_libGTASA + 0x288608, anim, callBack, data);
}

void RtAnimInterpolatorSetAnimCallBack(RtAnimInterpolator* anim, RtAnimCallBack callBack, RwReal time, void* data) {
    CHook::CallFunction<void>(g_libGTASA + 0x288610, anim, callBack, time, data);
}

RwBool RtAnimInterpolatorCopy(RtAnimInterpolator* outAnim, RtAnimInterpolator* inAnim) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x28861C, outAnim, inAnim);
}

RwBool RtAnimInterpolatorSubAnimTime(RtAnimInterpolator* anim, RwReal time) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x288648, anim, time);
}

RwBool RtAnimInterpolatorAddAnimTime(RtAnimInterpolator* anim, RwReal time) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x2887B4, anim, time);
}

RwBool RtAnimInterpolatorSetCurrentTime(RtAnimInterpolator* anim, RwReal time) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x2889AC, anim, time);
}

RwBool RtAnimAnimationMakeDelta(RtAnimAnimation* animation, RwInt32 numNodes, RwReal time) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x2889E0, animation, numNodes, time);
}

RwBool RtAnimInterpolatorBlend(RtAnimInterpolator* outAnim, RtAnimInterpolator* inAnim1, RtAnimInterpolator* inAnim2, RwReal alpha) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x288B84, outAnim, inAnim1, inAnim2, alpha);
}

RwBool RtAnimInterpolatorAddTogether(RtAnimInterpolator* outAnim, RtAnimInterpolator* inAnim1, RtAnimInterpolator* inAnim2) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x288C24, outAnim, inAnim1, inAnim2);
}

RtAnimInterpolator* RtAnimInterpolatorCreateSubInterpolator(RtAnimInterpolator* parentAnim, RwInt32 startNode, RwInt32 numNodes, RwInt32 maxInterpKeyFrameSize) {
    return CHook::CallFunction<RtAnimInterpolator*>(g_libGTASA + 0x288CB4, parentAnim, startNode, numNodes, maxInterpKeyFrameSize);
}

RwBool RtAnimInterpolatorBlendSubInterpolator(RtAnimInterpolator* outAnim, RtAnimInterpolator* inAnim1, RtAnimInterpolator* inAnim2, RwReal alpha) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x288D4C, outAnim, inAnim1, inAnim2, alpha);
}

RwBool RtAnimInterpolatorAddSubInterpolator(RtAnimInterpolator* outAnim, RtAnimInterpolator* mainAnim, RtAnimInterpolator* subAnim) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x288F7C, outAnim, mainAnim, subAnim);
}
