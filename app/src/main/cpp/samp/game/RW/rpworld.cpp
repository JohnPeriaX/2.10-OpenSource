//
// Created by x1y2z on 11.04.2023.
//

#include "RenderWare.h"
#include "game/common.h"
#include "../vendor/armhook/patch.h"

RpClump* RpClumpForAllAtomics(RpClump* clump, RpAtomicCallBack callback, void* data) {
    if(clump)
        return CHook::CallFunction<RpClump*>(g_libGTASA + 0x2BA020, clump, callback, data);
}

RpGeometry* RpGeometryForAllMaterials(RpGeometry* geometry, RpMaterialCallBack fpCallBack, void* data) {
    return CHook::CallFunction<RpGeometry*>(g_libGTASA + 0x2BCE78, geometry, fpCallBack, data);
}

RwBool RpClumpDestroy(RpClump* clump) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x2BAAB0, clump);
}

RpClump* RpClumpRender(RpClump* clump) {
    return CHook::CallFunction<RpClump*>("_Z13RpClumpRenderP7RpClump", clump);
}

RpLight* RpLightCreate(RwInt32 type) {
    return CHook::CallFunction<RpLight*>(g_libGTASA + 0x2BE078, type);
}

RwBool RpLightDestroy(RpLight* light) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x2BE210, light);
}

RpWorld* RpWorldCreate(RwBBox* boundingBox) {
    return CHook::CallFunction<RpWorld*>(g_libGTASA + 0x2C6714, boundingBox);
}

RpWorld* RpWorldAddCamera(RpWorld* world, RwCamera* camera) {
    return CHook::CallFunction<RpWorld*>(g_libGTASA + 0x2C78F0, world, camera);
}

RpLight* RpLightSetColor(RpLight* light, const RwRGBAReal* color) {
    return CHook::CallFunction<RpLight*>(g_libGTASA + 0x2BD930, light, color);
}

RpAtomic* AtomicDefaultRenderCallBack(RpAtomic* atomic) {
    return CHook::CallFunction<RpAtomic*>(g_libGTASA + 0x2B9B08, atomic);
}

RpWorld* RpWorldAddLight(RpWorld* world, RpLight* light) {
    return CHook::CallFunction<RpWorld*>(g_libGTASA + 0x2C8588, world, light);
}

RpWorld* RpWorldRemoveLight(RpWorld* world, RpLight* light) {
    return CHook::CallFunction<RpWorld*>(g_libGTASA + 0x2C85F4, world, light);
}

RwBool RpAtomicDestroy(RpAtomic* atomic) {
    return CHook::CallFunction<RwBool>(g_libGTASA + 0x2BA534, atomic);
}

void RpClumpGtaCancelStream() {
    CHook::CallFunction<void>(g_libGTASA + 0x6F4E38);
}