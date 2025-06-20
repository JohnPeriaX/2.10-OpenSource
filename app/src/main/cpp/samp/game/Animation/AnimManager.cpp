/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/

#include <cassert>

#include "../common.h"
#include "AnimManager.h"
#include "game/MemoryMgr.h"
#include "../vendor/armhook/patch.h"
#include "game/CFileMgr.h"
#include "game/FileLoader.h"
#include "game/Streaming.h"
#include "game/Core/KeyGen.h"
#include "AnimBlendClumpData.h"
#include "game/Plugins/RpAnimBlendPlugin/RpAnimBlend.h"
#include "game/Models/ModelInfo.h"
#include "game/RW/RwHelper.h"
#include "game/Enums/eModelID.h"
#include "game/RW/rpskin.h"
#include "game/General.h"
#include "AnimAssocDescriptions.h"

AnimAssocDefinition CAnimManager::*ms_aAnimAssocDefinitions[NUM_ANIM_ASSOC_GROUPS];

struct IfpHeader {
    uint32 ident;
    uint32 size;
} header;

// 0x5BF6B0
void CAnimManager::Initialise() {
    CAnimManager::ms_numAnimations = 0;
    CAnimManager::ms_numAnimBlocks = 0;
    CAnimManager::ms_numAnimAssocDefinitions = 0x76;

  //  ms_numAnimAssocDefinitions = 118; // ANIM_TOTAL_GROUPS aka NUM_ANIM_ASSOC_GROUPS
    ms_animCache.Init(50);
    ReadAnimAssociationDefinitions();
    RegisterAnimBlock("ped");
}

// 0x5BC910
void CAnimManager::ReadAnimAssociationDefinitions() {
    return CHook::CallFunction<void>(g_libGTASA + 0x560CC4);
    // return plugin::Call<0x5BC910>();

//    char name[32], block[32], type[32];
//    bool isAnimSection = false;
//    AnimAssocDefinition* animStyle;
//    int animCount;
//
//  //  CFileMgr::SetDir("");
//    auto file = CFileMgr::OpenFile("DATA/ANIMGRP.DAT", "rb");
//
//    for (auto line = CFileLoader::LoadLine(file); line; line = CFileLoader::LoadLine(file)) {
//        if (!*line || *line == '#')
//            continue;
//
//        if (isAnimSection) {
//            if (sscanf(CFileLoader::ms_line, "%s", name) == 1) {
//                if (!memcmp(name, "end", 4)) {
//                    isAnimSection = false;
//                } else {
//                    AddAnimToAssocDefinition(animStyle, name);
//                }
//            }
//        }
//        else
//        {
//            sscanf(CFileLoader::ms_line, "%s %s %s %d", name, block, type, &animCount) == 4;
//            animStyle = AddAnimAssocDefinition(name, block, MODEL_MALE01, animCount, aStdAnimDescs);
//            isAnimSection = true;
//        }
//    }
//    CFileMgr::CloseFile(file);
}

// 0x4D4130
void CAnimManager::Shutdown() {
    for (auto i = 0; i < NUM_ANIM_BLOCKS; i++) {
        CStreaming::RemoveModel(IFPToModelId(i));
    }

    for (auto i = 0; i < ms_numAnimations; i++) {
        ms_aAnimations[i].Shutdown();
    }

    ms_animCache.Shutdown();
    delete[] ms_aAnimAssocGroups;
}

CAnimBlock* CAnimManager::GetAnimationBlock(AssocGroupId animGroup) { // ��� �� ��� ��������. ���� ����
    return ms_aAnimAssocGroups[animGroup].m_pAnimBlock;
}

// 0x4D3940
CAnimBlock* CAnimManager::GetAnimationBlock(const char* name) {
    for (auto& block : std::span{ ms_aAnimBlocks.data(), (size_t)ms_numAnimBlocks }) {
        if (strcmp(block.szName, name) == 0) {
            return &block;
        }
    }
    return nullptr;
}

int32 CAnimManager::GetAnimationBlockIndex(AssocGroupId animGroup) {
    return GetAnimationBlock(animGroup) - ms_aAnimBlocks.data();
}

int32 CAnimManager::GetAnimationBlockIndex(CAnimBlock* animBlock) {
    return animBlock - ms_aAnimBlocks.data();
}

// 0x4D3990
int32 CAnimManager::GetAnimationBlockIndex(const char* name) {

    auto animBlock = GetAnimationBlock(name);

    return GetAnimationBlockIndex(animBlock);
}

// 0x4D39B0
AssocGroupId CAnimManager::GetFirstAssocGroup(const char* name) {
    // ANIM_TOTAL_GROUPS
    for (auto i = 0; i < ANIM_GROUP_MAN; i++) {
        if (!strcmp(ms_aAnimAssocDefinitions[i].blockName, name)) {
            return static_cast<AssocGroupId>(i);
        }
    }

    return ANIM_GROUP_MAN;
}

// 0x4D39F0
CAnimBlendHierarchy* CAnimManager::GetAnimation(uint32 hash, const CAnimBlock* animBlock) {
    CAnimBlendHierarchy* hier = &ms_aAnimations[animBlock->startAnimation];

    for (auto i = 0; i < animBlock->animationCount; i++) {
        if (hier->m_hashKey == hash) {
            return hier;
        }
        hier++;
    }
    return nullptr;
}

// 0x4D42F0
CAnimBlendHierarchy* CAnimManager::GetAnimation(const char* animName, const CAnimBlock* animBlock) {
    return GetAnimation(CKeyGen::GetUppercaseKey(animName), animBlock);
}

// 0x4D3A20
const char* CAnimManager::GetAnimGroupName(AssocGroupId groupId) {
    return ms_aAnimAssocDefinitions[groupId].groupName;
}

// 0x4D3A30
const char* CAnimManager::GetAnimBlockName(AssocGroupId groupId) {
    return ms_aAnimAssocDefinitions[groupId].blockName;
}

// NOTSA
AssocGroupId CAnimManager::GetAnimationGroupId(const char* name) {
    for (auto i = 0; i < ms_numAnimAssocDefinitions; i++) {
        if (std::string_view{ name } == GetAnimGroupName((AssocGroupId)i)) {
            return (AssocGroupId)i;
        }
    }
    return AssocGroupId::ANIM_GROUP_DEFAULT;
}

// 0x4D3A40
CAnimBlendAssociation* CAnimManager::CreateAnimAssociation(AssocGroupId groupId, AnimationId animId) {
    return ms_aAnimAssocGroups[groupId].CopyAnimation(animId);
}

// 0x4D3A60
CAnimBlendStaticAssociation* CAnimManager::GetAnimAssociation(AssocGroupId groupId, AnimationId animId) {
    return ms_aAnimAssocGroups[groupId].GetAnimation(animId);
}

// 0x4D3A80
CAnimBlendStaticAssociation* CAnimManager::GetAnimAssociation(AssocGroupId groupId, const char* animName) {
    return ms_aAnimAssocGroups[groupId].GetAnimation(animName);
}

// 0x4D3AA0
CAnimBlendAssociation* CAnimManager::AddAnimation(RpClump* clump, AssocGroupId groupId, AnimationId animId) {
    return CHook::CallFunction<CAnimBlendAssociation*>(g_libGTASA + 0x466B64, clump, groupId, animId);

    CAnimBlendAssociation* anim = CreateAnimAssociation(groupId, animId);
    CAnimBlendClumpData* clumpData = RpClumpGetAnimBlendClumpData(clump);
    if (anim->IsMoving()) {
        CAnimBlendAssociation* syncAnim;
        CAnimBlendLink* link;
        for (link = clumpData->m_Associations.next; link; link = link->next) {
            syncAnim = CAnimBlendAssociation::FromLink(link);
            if (syncAnim->IsMoving())
                break;
        }
        if (link) {
            anim->SyncAnimation(syncAnim);
            anim->m_nFlags |= ANIMATION_STARTED;
        } else
            anim->Start(0.0f);
    } else
        anim->Start(0.0f);

    clumpData->m_Associations.Prepend(&anim->m_Link);
    return anim;
}

// 0x4D4330
CAnimBlendAssociation* CAnimManager::AddAnimation(RpClump* clump, CAnimBlendHierarchy* hier, int32 clumpAssocFlag) {
    return CHook::CallFunction<CAnimBlendAssociation*>(g_libGTASA + 0x466C14, clump, hier, clumpAssocFlag);

    CAnimBlendAssociation* anim = new CAnimBlendAssociation(clump, hier);
    anim->m_nFlags |= clumpAssocFlag;
    anim->ReferenceAnimBlock();
    UncompressAnimation(hier);
    CAnimBlendClumpData* clumpData = RpClumpGetAnimBlendClumpData(clump);
    if (anim->IsMoving()) {
        CAnimBlendAssociation* syncAnim;
        CAnimBlendLink* link;
        for (link = clumpData->m_Associations.next; link; link = link->next) {
            syncAnim = CAnimBlendAssociation::FromLink(link);
            if (syncAnim->IsMoving())
                break;
        }
        if (link) {
            anim->SyncAnimation(syncAnim);
            anim->m_nFlags |= ANIMATION_STARTED;
        } else
            anim->Start(0.0f);
    } else
        anim->Start(0.0f);

    clumpData->m_Associations.Prepend(&anim->m_Link);
    return anim;
}

// 0x4D3B30
CAnimBlendAssociation* CAnimManager::AddAnimationAndSync(RpClump* clump, CAnimBlendAssociation* animBlendAssoc, AssocGroupId groupId, AnimationId animId) {
    return CHook::CallFunction<CAnimBlendAssociation*>(g_libGTASA + 0x466CE0, clump, animBlendAssoc, groupId, animId);

    CAnimBlendAssociation* anim = CreateAnimAssociation(groupId, animId);
    CAnimBlendClumpData* clumpData = RpClumpGetAnimBlendClumpData(clump);
    if (anim->IsMoving() && animBlendAssoc) {
        anim->SyncAnimation(animBlendAssoc);
        anim->m_nFlags |= ANIMATION_STARTED;
    } else
        anim->Start(0.0f);

    clumpData->m_Associations.Prepend(&anim->m_Link);
    return anim;
}

// 0x4D3BA0
AnimAssocDefinition* CAnimManager::AddAnimAssocDefinition(const char* groupName, const char* blockName, uint32 modelIndex, uint32 animsCount, AnimDescriptor* descriptor) {
    return CHook::CallFunction<AnimAssocDefinition*>(g_libGTASA + 0x467500, groupName, blockName, modelIndex, animsCount, descriptor);

    /*
    auto* def = &ms_aAnimAssocDefinitions[ms_numAnimAssocDefinitions++];
    strcpy_s(def->groupName, groupName);
    strcpy_s(def->blockName, blockName);
    def->modelIndex = modelIndex;
    def->animsCount = animsCount;
    def->animDesc   = descriptor;
    def->animNames  = new char*[animsCount];
    for (auto i = 0; i < animsCount; i++) {
        def->animNames[i] = new char[24];
        *def->animNames[i] = '\0';
    }
    return def;
    */
}

// 0x4D3C80
void CAnimManager::AddAnimToAssocDefinition(AnimAssocDefinition* pDef, const char* animName) {
    char **v2; // r3
    char *v3; // r0
    char *v4; // t1

    v2 = *(char ***)(pDef + 0x28);
    v3 = *v2;
    if ( **v2 )
    {
        do
        {
            v4 = v2[1];
            ++v2;
            v3 = v4;
        }
        while ( *v4 );
    }
    strcpy(v3, animName);

}

// 0x4C4DC0
bool IsClumpSkinned(RpClump *clump) {
    auto* atomic = GetFirstAtomic(clump);
    return atomic && RpSkinGeometryGetSkin(RpAtomicGetGeometry(atomic));
}

// 0x4D3CC0
void CAnimManager::CreateAnimAssocGroups() {
    for (auto i = 0; i < ms_numAnimAssocDefinitions; i++) {
        CAnimBlendAssocGroup* group = &ms_aAnimAssocGroups[i];
        AnimAssocDefinition* def = &ms_aAnimAssocDefinitions[i];
        CAnimBlock* block = GetAnimationBlock(def->blockName);
        if (block == nullptr || !block->bLoaded || group->m_pAssociations)
            continue;

        RpClump* clump = nullptr;
        if (def->modelIndex != MODEL_INVALID) {
            auto model = CModelInfo::GetModelInfo(def->modelIndex);
            clump = CHook::CallFunction<RpClump*>(*(uintptr_t*)(model->vtable + 0x2C*2), model);
            RpAnimBlendClumpInit(clump);
        }

        group->m_nGroupID = i;
        group->m_nIdOffset = def->animDesc->animId;
        group->CreateAssociations(def->blockName, clump, const_cast<char**>(def->animNames), def->animsCount); // todo: remove const_cast
        for (auto j = 0u; j < group->m_nNumAnimations; j++) {
            group->GetAnimation(def->animDesc[j].animId)->m_nFlags |= def->animDesc[j].flags;
        }

        if (clump) {
            if (IsClumpSkinned(clump)) {
                RpClumpForAllAtomics(clump, AtomicRemoveAnimFromSkinCB, nullptr);
            }
            RpClumpDestroy(clump);
        }
    }
}

// 0x4D3E50
int32 CAnimManager::RegisterAnimBlock(const char* name) {
    CAnimBlock* animBlock = GetAnimationBlock(name);
    if (animBlock == nullptr) {
        animBlock = &ms_aAnimBlocks[ms_numAnimBlocks++];
        strncpy(animBlock->szName, name, MAX_ANIM_BLOCK_NAME);
        animBlock->animationCount = 0;
        animBlock->animationStyle = GetFirstAssocGroup(name);
        assert(animBlock->usRefs == 0);
    }
    return GetAnimationBlockIndex(animBlock);
}

// 0x4D3ED0
void CAnimManager::RemoveLastAnimFile() {
    return CHook::CallFunction<void>(g_libGTASA + 0x468710);

    ms_numAnimBlocks--;
    ms_numAnimations = ms_aAnimBlocks[ms_numAnimBlocks].startAnimation;
    for (auto i = 0; i < ms_aAnimBlocks[ms_numAnimBlocks].animationCount; i++) {
        ms_aAnimations[ms_aAnimBlocks[ms_numAnimBlocks].startAnimation + i].Shutdown();
    }

    ms_aAnimBlocks[ms_numAnimBlocks].bLoaded = false;
}

// 0x4D3F40
void CAnimManager::RemoveAnimBlock(int32 index) {
    return CHook::CallFunction<void>(g_libGTASA + 0x4687D4, index);

    CAnimBlock* block = &ms_aAnimBlocks[index];
    for (auto i = 0; i < ms_numAnimAssocDefinitions; i++) {
        if (ms_aAnimAssocGroups[i].m_pAnimBlock == block) {
            ms_aAnimAssocGroups[i].DestroyAssociations();
        }
    }

    for (auto i = 0; i < block->animationCount; i++) {
        ms_aAnimations[block->startAnimation + i].Shutdown();
    }

    block->bLoaded = false;
    block->usRefs = 0;
}

// 0x4D3FB0
void CAnimManager::AddAnimBlockRef(int32 index) {
    ms_aAnimBlocks[index].usRefs++;
}

// 0x4D3FD0
void CAnimManager::RemoveAnimBlockRef(int32 index) {
    ms_aAnimBlocks[index].usRefs--;
    /* see RemoveAnimBlockRefWithoutDelete, logically here should be called RemoveModel or something
    if (--ms_aAnimBlocks[index].usRefs == 0) {
        CStreaming::RemoveModel(IFPToModelId(index));
    }
    */
}

// 0x4D3FF0
void CAnimManager::RemoveAnimBlockRefWithoutDelete(int32 index) {
    ms_aAnimBlocks[index].usRefs--;
}

// 0x4D4010
int32 CAnimManager::GetNumRefsToAnimBlock(int32 index) {
    return ms_aAnimBlocks[index].usRefs;
}

// 0x4D41C0
void CAnimManager::UncompressAnimation(CAnimBlendHierarchy* hier) {
    return CHook::CallFunction<void>(g_libGTASA + 0x466740, hier);
}

// 0x4D42A0
void CAnimManager::RemoveFromUncompressedCache(CAnimBlendHierarchy* hier) {
    return CHook::CallFunction<void>(g_libGTASA + 0x4668AC, hier);
}

// 0x4D4410
CAnimBlendAssociation* CAnimManager::BlendAnimation(RpClump* clump, CAnimBlendHierarchy* animBlendHier, int32 flags, float clumpAssocBlendData) {
//    return CHook::CallFunction<CAnimBlendAssociation*>(g_libGTASA + 0x0033DE70 + 1, clump, animBlendHier, flags, clumpAssocBlendData);

    /*
    CAnimBlendClumpData* clumpData = RpClumpGetAnimBlendClumpData(clump);
    bool isPartial = (flags & ANIMATION_PARTIAL) == 0;
    int removePrevAnim = 0;
    CAnimBlendLink* link;
    CAnimBlendAssociation *anim, *found = nullptr;
    for (link = clumpData->m_Associations.next; link; link = link->next) {
        anim = CAnimBlendAssociation::FromLink(link);
        if (anim->m_pHierarchy == animBlendHier) {
            found = anim;
        } else if (isPartial == anim->IsPartial()) {
            if (anim->m_fBlendAmount > 0.0f) {
                float blendDelta = -delta * anim->m_fBlendAmount;
                if (blendDelta < anim->m_fBlendDelta || !isPartial) { // || (anim->m_pHierarchy->animBlockId == animBlendHier->animBlockId && anim->m_pHierarchy->animBlockId)) {
                    if (blendDelta > -0.05f) {
                        blendDelta = -0.05f;
                    }
                    anim->m_fBlendDelta = blendDelta;
                }
            } else {
                anim->m_fBlendDelta = -1.0f;
            }
            anim->m_nFlags |= ANIMATION_FREEZE_LAST_FRAME;
            removePrevAnim = 1;
        }
    }

    if (found) {
        found->m_fBlendDelta = (1.0f - found->m_fBlendAmount) * delta;
        if (found->m_fCurrentTime == found->m_pHierarchy->m_fTotalTime) { // !found->IsRunning() && ...
            found->Start(0.0f);
        }
    } else {
        found = new CAnimBlendAssociation(clump, animBlendHier);
        found->m_nFlags |= flags;
        found->ReferenceAnimBlock();
        UncompressAnimation(animBlendHier);
        clumpData->m_Associations.Prepend(&found->m_Associations);
        found->Start(0.0f);
        if (!removePrevAnim && !isPartial) {
            found->m_fBlendAmount = 1.0f;
            return found;
        }
        found->m_fBlendAmount = 0.0f;
        found->m_fBlendDelta = delta;
    }
    UncompressAnimation(found->m_pHierarchy);
    return found;
    */
}

// 0x4D4610
CAnimBlendAssociation* CAnimManager::BlendAnimation(RpClump* clump, AssocGroupId groupId, AnimationId animId, float clumpAssocBlendData) {
    return CHook::CallFunction<CAnimBlendAssociation*>(g_libGTASA + 0x466F3C, clump, groupId, animId, clumpAssocBlendData);

    /*
    auto removePrevAnim = false;
    CAnimBlendClumpData* clumpData = RpClumpGetAnimBlendClumpData(clump);
    CAnimBlendStaticAssociation* animProps = GetAnimAssociation(groupId, animId);
    CAnimBlendAssociation* anim;
    bool isMovement = animProps->IsMovement();
    bool isPartial = animProps->IsPartial();
    bool isIndestructible = animProps->IsIndestructible();
    CAnimBlendLink* link;
    CAnimBlendAssociation *found = nullptr, *movementAnim = nullptr;
    for (link = clumpData->link.next; link; link = link->next) {
        anim = CAnimBlendAssociation::FromLink(link);
        if (isMovement && anim->IsMovement())
            movementAnim = anim;
        if (anim->m_nAnimId == animId && anim->m_nAnimGroup == groupId)
            found = anim;
        else {
            if (isPartial == anim->IsPartial() && isIndestructible == anim->IsIndestructible()) {
                if (anim->m_fBlendAmount > 0.0f) {
                    float blendDelta = -delta * anim->m_fBlendAmount;
                    if (blendDelta < anim->m_fBlendDelta || !isPartial) {
                        if (blendDelta > -0.05f)
                            blendDelta = -0.05f;
                        anim->m_fBlendDelta = blendDelta;
                    }
                } else {
                    anim->m_fBlendDelta = -1.0f;
                }
                anim->flags |= ANIMATION_FREEZE_LAST_FRAME;
                removePrevAnim = true;
            }
        }
    }

    if (found) {
        found->m_fBlendDelta = (1.0f - found->m_fBlendAmount) * delta;
        if (!found->IsRunning() && found->currentTime == found->hierarchy->m_fTotalTime)
            found->Start(0.0f);
    } else {
        found = AddAnimationAndSync(clump, movementAnim, groupId, animId);
        if (!removePrevAnim && !isPartial) {
            found->m_fBlendAmount = 1.0f;
            return found;
        }
        found->m_fBlendAmount = 0.0f;
        found->m_fBlendDelta = delta;
    }
    UncompressAnimation(found->hierarchy);
    return found;
    */
}

// 0x4D5620
void CAnimManager::LoadAnimFiles() {
    RwStream* stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, "ANIM\\PED.IFP");
    assert(stream);
    LoadAnimFile(stream, true);
    RwStreamClose(stream, nullptr);

    ms_aAnimAssocGroups = new CAnimBlendAssocGroup[ms_numAnimAssocDefinitions];
    CreateAnimAssocGroups();
}

// 0x0033E650
void CAnimManager::LoadAnimFile(RwStream* stream, bool loadCompressed, char const(*uncompressedAnimations)[32]) {

    RwStreamRead(stream, &header, sizeof(IfpHeader));
    if (header.ident == '3PNA' || header.ident == '2PNA') {
        LoadAnimFile_ANP23(stream, loadCompressed, header.ident == '3PNA');
    } else if (header.ident == 'KPNA') {
        LoadAnimFile_ANPK(stream, loadCompressed, uncompressedAnimations);
    }
}

inline void CAnimManager::LoadAnimFile_ANPK(RwStream* stream, bool compress, const char (*uncompressedAnims)[32]) {
#define ROUND_SIZE(x)                                                                                                                                                              \
    if ((x)&3)                                                                                                                                                                     \
    (x) += 4 - ((x)&3)

    /*
    IfpHeader info, name, dgan, cpan, anim;
    char buf[256];
    float* fbuf = (float*)buf;

    // block name
    // RwStreamRead(stream, &anpk, sizeof(IfpHeader));
    // assert(anpk.ident[0] == 'A' && anpk.ident[1] == 'N' && anpk.ident[2] == 'P');
    // if (anpk.ident[3] != 'K')
    //     return LoadAnimFile_2(stream, compress, uncompressedAnims, anpk.ident[3] == '3');
    // ROUND_SIZE(anpk.size);

    RwStreamRead(stream, &info, sizeof(IfpHeader));
    ROUND_SIZE(info.size);
    RwStreamRead(stream, buf, info.size);
    CAnimBlock* animBlock = GetAnimationBlock(buf + 4);
    if (animBlock) {
        if (animBlock->animationCount == 0) {
            animBlock->animationCount = *(int*)buf;
            animBlock->startAnimation = ms_numAnimations;
        }
    } else {
        animBlock = &ms_aAnimBlocks[ms_numAnimBlocks++];
        strncpy_s(animBlock->szName, buf + 4, MAX_ANIMBLOCK_NAME);
        animBlock->animationCount = *(int*)buf;
        animBlock->startAnimation = ms_numAnimations;
        animBlock->animationStyle = GetFirstAssocGroup(animBlock->szName);
    }

    DEV_LOG("Loading ANIMS {}", animBlock->szName);
    animBlock->bLoaded = true;

    int animIndex = animBlock->startAnimation;
    for (auto j = 0; j < animBlock->animationCount; j++) {
        assert(animIndex < ARRAY_SIZE(ms_aAnimations));
        CAnimBlendHierarchy* hier = &ms_aAnimations[animIndex++];

        // animation name
        RwStreamRead(stream, &name, sizeof(IfpHeader));
        ROUND_SIZE(name.size);
        RwStreamRead(stream, buf, name.size);
        //printf("Loading hierarchy %s\n", buf);
        hier->SetName(buf);

        //#ifdef ANIM_COMPRESSION
        bool compressHier = compress;
        //#else
        //        bool compressHier = false;
        //#endif
        if (uncompressedAnims) {
            for (int i = 0; uncompressedAnims[i][0]; i++) {
                //if (!stricmp(uncompressedAnims[i], buf))
                if (CKeyGen::GetUppercaseKey(uncompressedAnims[i]) == hier->m_hashKey)// {
                    //debug("Loading %s uncompressed\n", hier->name);
                    compressHier = false;
                //}
            }
        }

        hier->m_bRunningCompressed = compressHier;
        hier->m_bKeepCompressed = false;

        // DG info has number of nodes/sequences
        RwStreamRead(stream, (char*)&dgan, sizeof(IfpHeader));
        ROUND_SIZE(dgan.size);
        RwStreamRead(stream, (char*)&info, sizeof(IfpHeader));
        ROUND_SIZE(info.size);
        RwStreamRead(stream, buf, info.size);
        int nSeq = *(int*)buf;
        hier->numSequences = nSeq;
        //hier->sequences = (CAnimBlendSequence*)CMemoryMgr::Malloc(nSeq * sizeof(CAnimBlendSequence));
        hier->sequences = new CAnimBlendSequence[nSeq];

        for (auto k = 0; k < nSeq; k++) { // or seq++ ?
            CAnimBlendSequence* seq = &hier->sequences[k];
            // Each node has a name and key frames
            RwStreamRead(stream, &cpan, sizeof(IfpHeader));
            ROUND_SIZE(cpan.size);
            RwStreamRead(stream, &anim, sizeof(IfpHeader));
            ROUND_SIZE(anim.size);
            RwStreamRead(stream, buf, anim.size);
            int numFrames = *(int*)(buf + 28);
            seq->SetName(buf);
            if (anim.size == 44)
                seq->SetBoneTag(*(int*)(buf + 40));
            if (numFrames == 0)
                continue;
            //printf("Loading %s frames %d\n", buf, numFrames);

            bool hasScale = false;
            bool hasTranslation = false;
            RwStreamRead(stream, &info, sizeof(info));
            //if (numFrames == 0)
            //    continue;
            if (strncmp(info.ident, "KRTS", 4) == 0) {
                hasScale = true;
                seq->SetNumFrames(numFrames, true, compressHier, NULL);
            } else if (strncmp(info.ident, "KRT0", 4) == 0) {
                hasTranslation = true;
                seq->SetNumFrames(numFrames, true, compressHier, NULL);
            } else if (strncmp(info.ident, "KR00", 4) == 0) {
                seq->SetNumFrames(numFrames, false, compressHier, NULL);
            }
            //if (seq->numFrames)
            // if(strstr(seq->name, "L Toe"))
            //	debug("anim %s has toe keyframes\n", hier->name); // BUG: seq->name

            for (auto l = 0; l < numFrames; l++) {
                if (hasScale) {
                    RwStreamRead(stream, buf, 0x2C);
                    CQuaternion rot(fbuf[0], fbuf[1], fbuf[2], fbuf[3]);
                    rot.Conjugate();
                    CVector trans(fbuf[4], fbuf[5], fbuf[6]);

                    if (compressHier) {
                        KeyFrameTransCompressed* kf = (KeyFrameTransCompressed*)seq->GetKeyFrameCompressed(l);
                        kf->SetRotation(rot);
                        kf->SetTranslation(trans);
                        // scaling ignored
                        kf->SetTime(fbuf[10]); // absolute time here
                    } else {
                        KeyFrameTrans* kf = (KeyFrameTrans*)seq->GetKeyFrame(l);
                        kf->rotation = rot;
                        kf->translation = trans;
                        // scaling ignored
                        kf->deltaTime = fbuf[10]; // absolute time here
                    }
                } else if (hasTranslation) {
                    RwStreamRead(stream, buf, 0x20);
                    CQuaternion rot(fbuf[0], fbuf[1], fbuf[2], fbuf[3]);
                    rot.Conjugate();
                    CVector trans(fbuf[4], fbuf[5], fbuf[6]);

                    if (compressHier) {
                        KeyFrameTransCompressed* kf = (KeyFrameTransCompressed*)seq->GetKeyFrameCompressed(l);
                        kf->SetRotation(rot);
                        kf->SetTranslation(trans);
                        kf->SetTime(fbuf[7]); // absolute time here
                    } else {
                        KeyFrameTrans* kf = (KeyFrameTrans*)seq->GetKeyFrame(l);
                        kf->rotation = rot;
                        kf->translation = trans;
                        kf->deltaTime = fbuf[7]; // absolute time here
                    }
                } else {
                    RwStreamRead(stream, buf, 0x14);
                    CQuaternion rot(fbuf[0], fbuf[1], fbuf[2], fbuf[3]);
                    rot.Conjugate();

                    if (compressHier) {
                        KeyFrameCompressed* kf = (KeyFrameCompressed*)seq->GetKeyFrameCompressed(l);
                        kf->SetRotation(rot);
                        kf->SetTime(fbuf[4]); // absolute time here
                    } else {
                        KeyFrame* kf = (KeyFrame*)seq->GetKeyFrame(l);
                        kf->rotation = rot;
                        kf->deltaTime = fbuf[4]; // absolute time here
                    }
                }
            }
        }

        if (!compressHier) {
            hier->RemoveQuaternionFlips();
            hier->CalcTotalTime();
        }
    }

    if (animIndex > ms_numAnimations) {
        ms_numAnimations = animIndex;
    }
    */
#undef ROUND_SIZE
}

// NOTSA
inline void CAnimManager::LoadAnimFile_ANP23(RwStream* stream, bool compress, bool isANP3) {
    char buf[256];
    // char name[24];
    int nAnims, nSeq;

    RwStreamRead(stream, &buf, 24);   // animation name
    RwStreamRead(stream, &nAnims, 4);

    CAnimBlock* animBlock = GetAnimationBlock(buf);
    if (animBlock) {
        if (animBlock->animationCount == 0) {
            animBlock->animationCount = nAnims;
            animBlock->startAnimation = ms_numAnimations;
        }
    } else {
        animBlock = &ms_aAnimBlocks[ms_numAnimBlocks++];
        strncpy(animBlock->szName, buf, MAX_ANIM_BLOCK_NAME);
        animBlock->animationCount = nAnims;
        animBlock->startAnimation = ms_numAnimations;
        animBlock->animationStyle = GetFirstAssocGroup(animBlock->szName);
    }

    // debug("Loading ANIMS %s (%d)\n", animBlock->name, nAnims);
    animBlock->bLoaded = true;

    if (!nAnims)
        return;

    auto animIndex = animBlock->startAnimation;
    for (auto j = 0; j < nAnims; j++) {
        assert(animIndex < (int32)std::size(ms_aAnimations));
        CAnimBlendHierarchy* hier = &ms_aAnimations[animIndex++];

        // Each node has a name and key frames
        RwStreamRead(stream, &buf, 24);
        RwStreamRead(stream, &nSeq, 4);
        CAnimBlendSequence* st = nullptr;
        if (isANP3) {
            int n[2];
            RwStreamRead(stream, &n, sizeof(n));
            st = static_cast<CAnimBlendSequence*>(CMemoryMgr::Malloc(n[0]));
            //hier->m_bRunningCompressed = n[1] & 1; // unused?
        }
        hier->SetName(buf);
        hier->m_nSeqCount = nSeq;
        hier->m_pSequences = new CAnimBlendSequence[nSeq]; // (CAnimBlendSequence*)CMemoryMgr::Malloc(nSeq * sizeof(CAnimBlendSequence));
        hier->m_bKeepCompressed = false;

        CAnimBlendSequence* seq = hier->m_pSequences;
        for (auto k = 0; k < hier->m_nSeqCount; k++, seq++) {
            struct {
                int32 type;
                int32 frames_count;
                int32 bone_id;
            } sdata;
            RwStreamRead(stream, &buf, 24);
            RwStreamRead(stream, &sdata, sizeof(sdata));
            seq->SetName(buf);
            seq->SetBoneTag(sdata.bone_id);
            size_t data_size = 0;
            bool bIsRoot, bIsCompressed, bInvalidType = false;
            switch (sdata.type) {
                case 1:
                    data_size = sizeof(KeyFrame) * sdata.frames_count;
                    bIsRoot = false;
                    bIsCompressed = false;
                    break;
                case 2:
                    data_size = sizeof(KeyFrameTrans) * sdata.frames_count;
                    bIsRoot = true;
                    bIsCompressed = false;
                    break;
                case 3:
                    data_size = sizeof(KeyFrameCompressed) * sdata.frames_count;
                    bIsRoot = false;
                    bIsCompressed = true;
                    break;
                case 4:
                    data_size = sizeof(KeyFrameTransCompressed) * sdata.frames_count;
                    bIsRoot = true;
                    bIsCompressed = true;
                    break;
                default:
                    bInvalidType = true;
                    break;
            }
            if (!bInvalidType) {
                if (k == 0) {
                    hier->m_bRunningCompressed = bIsCompressed;
                }

                seq->SetNumFrames(sdata.frames_count, bIsRoot, bIsCompressed, st);
                RwStreamRead(stream, seq->m_pFrames, data_size);
                if (isANP3) {
                    // st += data_size; //(void *)((uint32)st + data_size);
                    st = (CAnimBlendSequence*)((size_t)st + data_size);
                    seq->m_usingExternalMemory = true;
                    // seq->usFlags |= EXTERNAL_KEYFRAMES_MEM;
                }
            }
        }
        if (!hier->m_bRunningCompressed) {
            hier->RemoveQuaternionFlips();
            hier->CalcTotalTime();
        }
    }

    if (animIndex > ms_numAnimations) {
        ms_numAnimations = animIndex;
    }
}


// ---- hooks

CAnimBlendStaticAssociation* CAnimManager__GetAnimAssociation1(AssocGroupId groupId, AnimationId animId) {
    return CAnimManager::GetAnimAssociation(groupId, animId);
}

CAnimBlendStaticAssociation* CAnimManager__GetAnimAssociation2(AssocGroupId groupId, const char* animName) {
    return CAnimManager::GetAnimAssociation(groupId, animName);
}

void CAnimManager::InjectHooks() {
    CHook::Write(g_libGTASA + 0x84EA38, &ms_numAnimAssocDefinitions);

    CHook::Write(g_libGTASA + 0x850780, &ms_aAnimBlocks);
    CHook::Write(g_libGTASA + 0x84F4B0, &ms_numAnimBlocks);

    CHook::Write(g_libGTASA + 0x84DF30, &ms_aAnimAssocGroups);

    CHook::Write(g_libGTASA + 0x84C3D8, &ms_aAnimations);
    CHook::Write(g_libGTASA + 0x84DE00, &ms_numAnimations);
    CHook::Write(g_libGTASA + 0x84F7E0, &ms_animCache);

    CHook::Redirect("_ZN12CAnimManager18GetAnimAssociationE12AssocGroupId11AnimationId", &CAnimManager__GetAnimAssociation1);
    CHook::Redirect("_ZN12CAnimManager18GetAnimAssociationE12AssocGroupIdPKc", &CAnimManager__GetAnimAssociation2);

    CHook::Redirect("_ZN12CAnimManager10InitialiseEv", &CAnimManager::Initialise);
    CHook::Redirect("_ZN12CAnimManager13LoadAnimFilesEv", &CAnimManager::LoadAnimFiles);
}