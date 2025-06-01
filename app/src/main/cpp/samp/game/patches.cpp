#include "../main.h"
#include "../game/game.h"
#include "../vendor/armhook/patch.h"
#include "vehicleColoursTable.h"
#include "../settings.h"
extern CSettings* pSettings;

VehicleAudioPropertiesStruct VehicleAudioProperties[20000];
#include "game.h"
#include "World.h"
#include "net/netgame.h"

extern CGame* pGame;
void readVehiclesAudioSettings()
{
    char vehicleModel[50];
    int16_t pIndex = 0;

    FILE* pFile;

    char line[300];

    // ล้างข้อมูล VehicleAudioProperties
    memset(VehicleAudioProperties, 0x00, sizeof(VehicleAudioProperties));

    VehicleAudioPropertiesStruct CurrentVehicleAudioProperties;

    memset(&CurrentVehicleAudioProperties, 0x0, sizeof(VehicleAudioPropertiesStruct));

    char buffer[0xFF];
    sprintf(buffer, "%sSAMP/vehicleAudioSettings.cfg", g_pszStorage);
    pFile = fopen(buffer, "r");
    if (!pFile)
    {
        //Log("Cannot read vehicleAudioSettings.cfg");
        return;
    }

    // ไฟล์มีอยู่
    while (fgets(line, sizeof(line), pFile))
    {
        if (strncmp(line, ";the end", 8) == 0)
            break;

        if (line[0] == ';')
            continue;

        sscanf(line, "%s %d %d %d %d %f %f %d %f %d %d %d %d %f",
               vehicleModel,
               &CurrentVehicleAudioProperties.VehicleType,
               &CurrentVehicleAudioProperties.EngineOnSound,
               &CurrentVehicleAudioProperties.EngineOffSound,
               &CurrentVehicleAudioProperties.field_4,
               &CurrentVehicleAudioProperties.field_5,
               &CurrentVehicleAudioProperties.field_6,
               &CurrentVehicleAudioProperties.HornTon,
               &CurrentVehicleAudioProperties.HornHigh,
               &CurrentVehicleAudioProperties.DoorSound,
               &CurrentVehicleAudioProperties.RadioNum,
               &CurrentVehicleAudioProperties.RadioType,
               &CurrentVehicleAudioProperties.field_14,
               &CurrentVehicleAudioProperties.field_16);

        ((void (*)(const char* thiz, int16_t* a2))(g_libGTASA + 0x385E38 + 1))(vehicleModel, &pIndex);
        memcpy(&VehicleAudioProperties[pIndex-400], &CurrentVehicleAudioProperties, sizeof(VehicleAudioPropertiesStruct));
    }

    fclose(pFile);
}

void ApplyFPSPatch(uint8_t fps)
{
    CHook::WriteMemory(g_libGTASA + 0x70A38C, "\xE9\x0F\x1E\x32", 4);
    CHook::WriteMemory(g_libGTASA + 0x70A43C, "\xE8\x0F\x1E\x32", 4);
    CHook::WriteMemory(g_libGTASA + 0x70A458, "\xE9\x0F\x1E\x32", 4);

    FLog("New fps limit = %d", fps);
}

void DisableAutoAim()
{
    CHook::RET("_ZN10CPlayerPed22FindWeaponLockOnTargetEv"); // CPed::FindWeaponLockOnTarget (ค้นหาเป้าหมายการล็อกอาวุธ)
    CHook::RET("_ZN10CPlayerPed26FindNextWeaponLockOnTargetEP7CEntityb"); // CPed::FindNextWeaponLockOnTarget (ค้นหาเป้าหมายการล็อกอาวุธถัดไป)
    CHook::RET("_ZN4CPed21SetWeaponLockOnTargetEP7CEntity"); // CPed::SetWeaponLockOnTarget (ตั้งค่าเป้าหมายการล็อกอาวุธ)
}

void ApplySAMPPatchesInGame()
{
    FLog("Installing patches (ingame)..");

    /* ปลดล็อกแผนที่ */
    // CTheZones::ZonesVisited[100]
    memset((void*)(g_libGTASA + 0xC1BF92), 1, 100);
    // CTheZones::ZonesRevealed
    *(uint32_t*)(g_libGTASA + 0xC1BFF8) = 100;

    // แก้ไข task ของ CPlayerPed
    CHook::WriteMemory(g_libGTASA + 0x5C0BC4, (uintptr_t)"\x34\x00\x80\x52", 4);

    // การวาด blips บนเรดาร์
    CHook::NOP(g_libGTASA + 0x52522C, 2);
    CHook::NOP(g_libGTASA + 0x525E14, 2);

//  CHook::WriteMemory(g_libGTASA + 0x00341F84, (uintptr_t)"\x00\xF0\x21\xBE", 4);

    // ไม่ประมวลผลเสียงยานพาหนะ
    CHook::NOP(g_libGTASA + 0x674610, 2);
    CHook::NOP(g_libGTASA + 0x682C1C, 2);
    CHook::NOP(g_libGTASA + 0x68DD0C, 2);

    // ปิดวิทยุในเกม
    CHook::RET("_ZN20CAERadioTrackManager7ServiceEi");

    // แผนที่ในเมนู
    CHook::NOP(g_libGTASA + 0x36A6E8, 2); // ข้อความคำอธิบายแผนที่
    CHook::NOP(g_libGTASA + 0x36A6F8, 2); // ไอคอนคำอธิบาย
    CHook::NOP(g_libGTASA + 0x36A190, 2); // ชื่อสถานที่
}

int32_t CWorld__FindPlayerSlotWithPedPointer(CPedGTA* pPlayersPed)
{
    for(int i = 0; i < MAX_PLAYERS; ++i)
    {
        if(CWorld::Players[i].m_pPed == pPlayersPed)
            return i;
    }
    return -1;
}

void ApplyPatches_level0()
{
    FLog("ApplyPatches_level0");

    CHook::Write(g_libGTASA + 0x84E7A8, &CWorld::Players);
    CHook::Write(g_libGTASA + 0x8516D8, &CWorld::PlayerInFocus);

    CHook::Redirect("_ZN6CWorld28FindPlayerSlotWithPedPointerEPv", &CWorld__FindPlayerSlotWithPedPointer);

    // แก้ไข alpha raster
    CHook::WriteMemory(g_libGTASA + 0x23FDE0, (uintptr_t)"\x22\x00\x80\x52", 4);

    //CHook::RET("_ZN6CTrain10InitTrainsEv"); // CTrain::InitTrains (เริ่มต้นรถไฟ)

    CHook::RET("_ZN8CClothes4InitEv"); // CClothes::Init() (เริ่มต้นเสื้อผ้า)
    CHook::RET("_ZN8CClothes13RebuildPlayerEP10CPlayerPedb"); // CClothes::RebuildPlayer (สร้างผู้เล่นใหม่)

    CHook::RET("_ZNK35CPedGroupDefaultTaskAllocatorRandom20AllocateDefaultTasksEP9CPedGroupP4CPed"); // AllocateDefaultTasks (จัดสรรงานเริ่มต้น)
    CHook::RET("_ZN6CGlass4InitEv"); // CGlass::Init (เริ่มต้นกระจก)
    CHook::RET("_ZN8CGarages17Init_AfterRestartEv"); // CGarages::Init_AfterRestart (เริ่มต้นโรงรถหลังรีสตาร์ท)
    CHook::RET("_ZN6CGangs10InitialiseEv"); // CGangs::Initialise (เริ่มต้นแก๊.klw4
    CHook::RET("_ZN5CHeli9InitHelisEv"); // CHeli::InitHelis(void) (เริ่มต้นเฮลิคอปเตอร์)
    CHook::RET("_ZN11CFileLoader10LoadPickupEPKc"); // CFileLoader::LoadPickup (โหลดของที่เก็บได้)
    CHook::RET("_ZN14CLoadingScreen15DisplayPCScreenEv"); // Loading screen (หน้าจอโหลด)

    // entryexit
    //CHook::RET("_ZN17CEntryExitManager4InitEv"); // (เริ่มต้นตัวจัดการทางเข้า-ออก)
    //CHook::RET("_ZN17CEntryExitManager22PostEntryExitsCreationEv"); // (ดำเนินการหลังสร้างทางเข้า-ออก)

    CHook::RET("_ZN10CSkidmarks6UpdateEv"); // CSkidmarks::Update (อัปเดตรอยลื่นไถล)
    CHook::RET("_ZN10CSkidmarks6RenderEv"); // CSkidmarks::Render (เรนเดอร์รอยลื่นไถล)

    //CHook::RET("_ZN14SurfaceInfos_c17CreatesWheelGrassEj"); // SurfaceInfos_c::CreatesWheelGrass (สร้างเอฟเฟกต์หญ้าจากล้อ)
    //CHook::RET("_ZN14SurfaceInfos_c18CreatesWheelGravelEj"); // SurfaceInfos_c::CreatesWheelGravel (สร้างเอฟเฟกต์กรวดจากล้อ)
    //CHook::RET("_ZN14SurfaceInfos_c15CreatesWheelMudEj"); // SurfaceInfos_c::CreatesWheelMud (สร้างเอฟเฟกต์โคลนจากล้อ)
    CHook::RET("_ZN14SurfaceInfos_c16CreatesWheelDustEj"); // SurfaceInfos_c::CreatesWheelDust (สร้างเอฟเฟกต์ฝุ่นจากล้อ)
    //CHook::RET("_ZN14SurfaceInfos_c16CreatesWheelSandEj"); // SurfaceInfos_c::CreatesWheelSand (สร้างเอฟเฟกต์ทรายจากล้อ)
    CHook::RET("_ZN14SurfaceInfos_c17CreatesWheelSprayEj"); // SurfaceInfos_c::CreatesWheelSpray (สร้างเอฟเฟกต์น้ำกระเซ็นจากล้อ)

    //CHook::RET("_ZN4Fx_c13AddWheelGrassEP8CVehicle7CVectorhf"); // Fx_c::AddWheelGrass (เพิ่มเอฟเฟกต์หญ้าจากล้อ)
    //CHook::RET("_ZN4Fx_c14AddWheelGravelEP8CVehicle7CVectorhf"); // Fx_c::AddWheelGravel (เพิ่มเอฟเฟกต์กรวดจากล้อ)
    //CHook::RET("_ZN4Fx_c11AddWheelMudEP8CVehicle7CVectorhf"); // Fx_c::AddWheelMud (เพิ่มเอฟเฟกต์โคลนจากล้อ)
    CHook::RET("_ZN4Fx_c12AddWheelDustEP8CVehicle7CVectorhf"); // Fx_c::AddWheelDust (เพิ่มเอฟเฟกต์ฝุ่นจากล้อ)
    //CHook::RET("_ZN4Fx_c12AddWheelSandEP8CVehicle7CVectorhf"); // Fx_c::AddWheelSand (เพิ่มเอฟเฟกต์ทรายจากล้อ)
    CHook::RET("_ZN4Fx_c13AddWheelSprayEP8CVehicle7CVectorhhf"); // Fx_c::AddWheelSpray (เพิ่มเอฟเฟกต์น้ำกระเซ็นจากล้อ)
}

void ApplyGlobalPatches()
{
    FLog("Installing patches..");

    CHook::RET("_ZN17CVehicleModelInfo17SetCarCustomPlateEv"); // ป้ายทะเบียนเริ่มต้น

    CHook::RET("_Z16SaveGameForPause10eSaveTypesPc"); // ไม่บันทึกเมื่อหยุดชั่วคราว หน้าจอดำ

    // ไอคอนสีดำ
    CHook::WriteMemory(g_libGTASA + 0x52737C, (uintptr_t)"\x1E\x00\x00\x14", 4); // B 0x5273F4
    CHook::WriteMemory(g_libGTASA + 0x5273F4, (uintptr_t)"\xE1\x03\x14\x2A", 4); // mov w1, w20

    /*// CRadar::DrawEntityBlip (แปลงสี)
    CHook::WriteMemory(g_libGTASA + 0x5258D8, (uintptr_t)"\x22\x00\x00\x14", 4); // B 0x525960
    CHook::WriteMemory(g_libGTASA + 0x525960, (uintptr_t)"\xE1\x03\x16\x2A", 4); // mov w1, W22

    // CRadar::DrawCoordBlip (แปลงสี)
    CHook::WriteMemory(g_libGTASA + 0x524F58, (uintptr_t)"\xCC\xFF\xFF\x17", 4); // B 0x524E88
    CHook::WriteMemory(g_libGTASA + 0x524E88, (uintptr_t)"\xE1\x03\x16\x2A", 4); // mov w1, W22*/

    // ป้องกันการขัดข้องของคำอธิบาย
    CHook::NOP(g_libGTASA + 0x36A690, 1);

    //ApplyShadowPatch();

    //CDebugInfo::ApplyDebugPatches();

    CHook::RET("_ZN12CAudioEngine16StartLoadingTuneEv"); // เสียงหน้าจอโหลด

    // DefaultPCSaveFileName
    char* DefaultPCSaveFileName = (char*)(g_libGTASA + 0x88CB08);
    memcpy(DefaultPCSaveFileName, "GTASAMP", 8);

    CHook::NOP(g_libGTASA + 0x004D8700, 1);  // ป้องกันการขัดข้องของ CCoronas::RenderSunReflection
    CHook::NOP(g_libGTASA + 0x006A852C, 1);  // ไม่ให้อาวุธเมื่อออกจากรถ (เช่น ไม้กอล์ฟ, ปืนลูกซอง ฯลฯ)
    CHook::NOP(g_libGTASA + 0x006A84E0, 1);  // ไม่ให้อาวุธเมื่อออกจากรถ (เช่น ไม้กอล์ฟ, ปืนลูกซอง ฯลฯ)

    CHook::RET("_ZN17CVehicleRecording4LoadEP8RwStreamii"); // CVehicleRecording::Load (โหลดบันทึกยานพาหนะ)

    CHook::RET("_ZN18CMotionBlurStreaks6UpdateEv"); // อัปเดตการเบลอจากการเคลื่อนไหว
    CHook::RET("_ZN7CCamera16RenderMotionBlurEv"); // เรนเดอร์การเบลอจากการเคลื่อนไหว

    CHook::RET("_ZN11CPlayerInfo17FindObjectToStealEP4CPed"); // ป้องกันการขัดข้อง
    CHook::RET("_ZN26CAEGlobalWeaponAudioEntity21ServiceAmbientGunFireEv"); // CAEGlobalWeaponAudioEntity::ServiceAmbientGunFire (จัดการเสียงปืนรอบข้าง)
    CHook::RET("_ZN30CWidgetRegionSteeringSelection4DrawEv"); // CWidgetRegionSteeringSelection::Draw (วาดการเลือกพวงมาลัย)
    CHook::RET("_ZN23CTaskSimplePlayerOnFoot18PlayIdleAnimationsEP10CPlayerPed"); // CTaskSimplePlayerOnFoot::PlayIdleAnimations (เล่นแอนิเมชัน idle)
    CHook::RET("_ZN13CCarEnterExit17SetPedInCarDirectEP4CPedP8CVehicleib"); // CCarEnterExit::SetPedInCarDirect (ตั้งค่าตัวละครในรถโดยตรง)
    CHook::RET("_ZN6CRadar10DrawLegendEiii"); // CRadar::DrawLgegend (วาดคำอธิบายเรดาร์)
    CHook::RET("_ZN6CRadar19AddBlipToLegendListEhi"); // CRadar::AddBlipToLegendList (เพิ่ม blip ในรายการคำอธิบาย)

    CHook::RET("_ZN11CAutomobile35CustomCarPlate_BeforeRenderingStartEP17CVehicleModelInfo"); // CAutomobile::CustomCarPlate_BeforeRenderingStart (ก่อนเริ่มเรนเดอร์ป้ายทะเบียน)
    CHook::RET("_ZN11CAutomobile33CustomCarPlate_AfterRenderingStopEP17CVehicleModelInfo"); // CAutomobile::CustomCarPlate_AfterRenderingStop (หลังหยุดเรนเดอร์ป้ายทะเบียน)
    CHook::RET("_ZN7CCamera8CamShakeEffff"); // CCamera::CamShake (การสั่นของกล้อง)
    CHook::RET("_ZN7CEntity23PreRenderForGlassWindowEv"); // CEntity::PreRenderForGlassWindow (เตรียมเรนเดอร์สำหรับหน้าต่างกระจก)
    CHook::RET("_ZN8CMirrors16RenderReflBufferEb"); // CMirrors::RenderReflBuffer (เรนเดอร์บัฟเฟอร์สะท้อน)
    CHook::RET("_ZN4CHud23DrawBustedWastedMessageEv"); // CHud::DrawBustedWastedMessage // ถูกจับ/เสียชีวิต
    CHook::RET("_ZN4CHud14SetHelpMessageEPKcPtbbbj"); // CHud::SetHelpMessage (ตั้งค่าข้อความช่วยเหลือ)
    CHook::RET("_ZN4CHud24SetHelpMessageStatUpdateEhtff"); // CHud::SetHelpMessageStatUpdate (ตั้งค่าการอัปเดตสถานะข้อความช่วยเหลือ)
    CHook::RET("_ZN6CCheat16ProcessCheatMenuEv"); // CCheat::ProcessCheatMenu (ประมวลผลเมนูโกง)
    CHook::RET("_ZN6CCheat13ProcessCheatsEv"); // CCheat::ProcessCheats (ประมวลผลการโกง)
    CHook::RET("_ZN6CCheat16AddToCheatStringEc"); // CCheat::AddToCheatString (เพิ่มสตริงการโกง)
    CHook::RET("_ZN6CCheat12WeaponCheat1Ev"); // CCheat::WeaponCheat1 (โกงอาวุธ 1)
    CHook::RET("_ZN6CCheat12WeaponCheat2Ev"); // CCheat::WeaponCheat2 (โกงอาวุธ 2)
    CHook::RET("_ZN6CCheat12WeaponCheat3Ev"); // CCheat::WeaponCheat3 (โกงอาวุธ 3)
    CHook::RET("_ZN6CCheat12WeaponCheat4Ev"); // CCheat::WeaponCheat4 (โกงอาวุธ 4)
    CHook::RET("_ZN8CGarages14TriggerMessageEPcsts"); // CGarages::TriggerMessage (ทริกเกอร์ข้อความโรงรถ)

    CHook::RET("_ZN11CPopulation6AddPedE8ePedTypejRK7CVectorb"); // CPopulation::AddPed (เพิ่มตัวละคร)
    CHook::RET("_ZN6CPlane27DoPlaneGenerationAndRemovalEv"); // CPlane::DoPlaneGenerationAndRemoval (จัดการการสร้างและลบเครื่องบิน)

    CHook::RET("_ZN10CEntryExit19GenerateAmbientPedsERK7CVector"); // CEntryExit::GenerateAmbientPeds (สร้างตัวละครรอบข้าง)
    CHook::RET("_ZN8CCarCtrl31GenerateOneEmergencyServicesCarEj7CVector"); // CCarCtrl::GenerateOneEmergencyServicesCar (สร้างรถฉุกเฉิน)
    CHook::RET("_ZN11CPopulation17AddPedAtAttractorEiP9C2dEffect7CVectorP7CEntityi"); // CPopulation::AddPedAtAttractor crash. wtf stuff? (เพิ่มตัวละครที่ตัวดึงดูด - ป้องกันการขัดข้อง)

    CHook::RET("_ZN7CDarkel26RegisterCarBlownUpByPlayerEP8CVehiclei"); // CDarkel__RegisterCarBlownUpByPlayer_hook (บันทึกรถที่ถูกระเบิดโดยผู้เล่น)
    CHook::RET("_ZN7CDarkel25ResetModelsKilledByPlayerEi"); // CDarkel__ResetModelsKilledByPlayer_hook (รีเซ็ตโมเดลที่ถูกฆ่าโดยผู้เล่น)
    CHook::RET("_ZN7CDarkel25QueryModelsKilledByPlayerEii"); // CDarkel__QueryModelsKilledByPlayer_hook (สอบถามโมเดลที่ถูกฆ่าโดยผู้เล่น)
    CHook::RET("_ZN7CDarkel27FindTotalPedsKilledByPlayerEi"); // CDarkel__FindTotalPedsKilledByPlayer_hook (ค้นหาจำนวนตัวละครที่ถูกฆ่าโดยผู้เล่น)
    CHook::RET("_ZN7CDarkel20RegisterKillByPlayerEPK4CPed11eWeaponTypebi"); // CDarkel__RegisterKillByPlayer_hook (บันทึกการฆ่าโดยผู้เล่น)

    //CHook::NOP(g_libGTASA + 0x55774C, 1); // CStreaming::ms_memoryAvailable = (int)v24

    CHook::NOP(g_libGTASA + 0x5C3258, 1);  // CCamera::ClearPlayerWeaponMode from CPlayerPed::ClearWeaponTarget (ล้างโหมดอาวุธของผู้เล่น)
    //CHook::WriteMemory(g_libGTASA + 0x266FC8, "\xF5\x03\x08\x32", 4); //  RenderQueue::RenderQueue
    CHook::WriteMemory(g_libGTASA + 0x4D644C, "\x1F\x0D\x00\x71", 4); // RE3: แก้ไขการ优化ของ R* ที่ป้องกันการเกิดตัวละคร

    CHook::RET("_ZN10CPlayerPed14AnnoyPlayerPedEb"); // CPedSamp::AnnoyPlayerPed (รบกวนตัวละครผู้เล่น)
    CHook::RET("_ZN11CPopulation15AddToPopulationEffff"); // CPopulation::AddToPopulation (เพิ่มในประชากร)

    CHook::RET("_ZN23CAEPedSpeechAudioEntity11AddSayEventEisjfhhh"); // CPed::Say (การพูดของตัวละคร)

    CHook::RET("_ZN10CPedGroups7ProcessEv"); // CPedGroups::Process (ประมวลผลกลุ่มตัวละคร)
    CHook::RET("_ZN21CPedGroupIntelligence7ProcessEv"); // CPedGroupIntelligence::Process (ประมวลผลความฉลาดของกลุ่มตัวละคร)
    CHook::RET("_ZN19CPedGroupMembership9SetLeaderEP4CPed"); // CPedGroupMembership::SetLeader (ตั้งค่าผู้นำกลุ่ม)
    CHook::RET("_ZN21CPedGroupIntelligence5FlushEv"); // CPedGroupIntelligence::Flush (ล้างความฉลาดของกลุ่ม)

    CHook::RET("_ZN22CRealTimeShadowManager4InitEv"); // CRealTimeShadowManager::Init (เริ่มต้นตัวจัดการเงาแบบเรียลไทม์)
    CHook::RET("_ZN22CRealTimeShadowManager6UpdateEv"); // CRealTimeShadowManager::Update (อัปเดตตัวจัดการเงาแบบเรียลไทม์)

    CHook::RET("_ZN22CRealTimeShadowManager20ReturnRealTimeShadowEP15CRealTimeShadow"); // CRealTimeShadowManager::ReturnRealTimeShadow from ~CPhysical (คืนค่าเงาแบบเรียลไทม์)
    CHook::RET("_ZN8CShadows19RenderStaticShadowsEb"); // CShadows::RenderStaticShadows (เรนเดอร์เงาคงที่)
    CHook::RET("_ZN8CMirrors16BeforeMainRenderEv"); // CMirrors::BeforeMainRender(void) (ก่อนการเรนเดอร์หลัก)
    CHook::RET("_ZN8CMirrors17RenderReflectionsEv"); // การเรนเดอร์สะท้อน

    CHook::RET("_ZN8CCarCtrl18GenerateRandomCarsEv"); // สร้างรถแบบสุ่ม
}

void InstallVehicleEngineLightPatches()
{
    // แก้ไขไฟท้าย (ประเภท)
    CHook::WriteMemory(g_libGTASA + 0x591272, (uintptr_t)"\x02", 1);
    CHook::WriteMemory(g_libGTASA + 0x59128E, (uintptr_t)"\x02", 1);
}