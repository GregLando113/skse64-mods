#include <skse64/PluginAPI.h>
#include <skse64/GameInput.h>
#include <skse64_common/Relocation.h>
#include <skse64_common/BranchTrampoline.h>
#include <skse64_common/Utilities.h>
#include <skse64/gamethreads.h>
#include <skse64_common/SafeWrite.h>
#include <skse64/GameEvents.h>

#include <Windows.h>
#include <cstdio>

struct TESGrabReleaseEvent
{

};


class MappedEventDispatcherList
{
public:
	EventDispatcher<void>								unk00;					//	00					  
	EventDispatcher<void>								unk58;					//  58  - sink offset 010 BGSEventProcessedEvent
	EventDispatcher<TESActiveEffectApplyRemoveEvent>	unkB0;					//  B0  - sink offset 018 TESActivateEvent
	EventDispatcher<void>								unk108;					//  108 - sink offset 020 
	EventDispatcher<void>								unk160;					//  160 - sink offset 028 TESActorLocationChangeEvent
	EventDispatcher<TESCellAttachDetachEvent>			unk1B8;					//  1B8 - sink offset 030
	EventDispatcher<void>								unk210;					//  210 - sink offset 038 TESCellAttachDetachEvent
	EventDispatcher<void>								unk2C0;					//  2C0 - sink offset 040 TESCellFullyLoadedEvent
	EventDispatcher<TESCombatEvent>						combatDispatcher;		//  318 - sink offset 048 TESCombatEvent
	EventDispatcher<TESContainerChangedEvent>			unk370;					//  370 - sink offset 050 TESContainerChangedEvent
	EventDispatcher<TESDeathEvent>						deathDispatcher;		//  3C8 - sink offset 058 TESDeathEvent
	EventDispatcher<void>								unk420;					//  420 - sink offset 068
	EventDispatcher<void>								unk478;					//  478 - sink offset 070
	EventDispatcher<void>								unk4D0;					//  4D0 - sink offset 078 TESEquipEvent
	EventDispatcher<void>								unk528;					//  528 - sink offset 080
	EventDispatcher<void>								unk580;					//  580 - sink offset 088 TESFurnitureEvent
	EventDispatcher<void>								unk5D8;					//  5D8 - sink offset 090 TESGrabReleaseEvent
	EventDispatcher<void>								unk630;					//  630 - sink offset 098 
	EventDispatcher<TESInitScriptEvent>					initScriptDispatcher;	//  688 - sink offset 0A0 TESInitScriptEvent
	EventDispatcher<void>								unk6E0;					//  6E0 - sink offset 0A8 
	EventDispatcher<void>								unk738;					//  738 - sink offset 0B0 TESLockChangedEvent
	EventDispatcher<void>								unk790;					//  790 - sink offset 0B8 TESMagicEffectApplyEvent
	EventDispatcher<void>								unk7E8;					//  7E8 - sink offset 0C0
	EventDispatcher<void>								unk840;					//  840 - sink offset 0C8 TESMoveAttachDetachEvent
	EventDispatcher<TESObjectLoadedEvent>				objectLoadedDispatcher;	//  898 - sink offset 0D0 TESObjectLoadedEvent
	EventDispatcher<void>								unk8F0;					//  8F0 - sink offset 0D8
	EventDispatcher<void>								unk948;					//  948 - sink offset 0E0 TESOpenCloseEvent
	EventDispatcher<void>								unk9A0;					//  9A0 - sink offset 0E8 TESPackageEvent
	EventDispatcher<void>								unk9F8;					//  9F8 - sink offset 0F0 TESPerkEntryRunEvent
	EventDispatcher<void>								unkA50;					//  A50 - sink offset 0F8 TESQuestInitEvent
	EventDispatcher<void>								unkAA8;					//  AA8 - sink offset 100 TESQuestStageEvent
	EventDispatcher<void>								unkB00;					//  B00 - sink offset 108 TESQuestStageItemDoneEvent
	EventDispatcher<void>								unkB58;					//  B58 - sink offset 110 TESQuestStartStopEvent
	EventDispatcher<void>								unkBB0;					//  BB0 - sink offset 118 
	EventDispatcher<void>								unkC08;					//  C08 - sink offset 120 TESResolveNPCTemplatesEvent
	EventDispatcher<void>								unkC60;					//  C60 - sink offset 128 TESSceneEvent
	EventDispatcher<void>								unkCB8;					//  CB8 - sink offset 130
	EventDispatcher<void>								unkD10;					//  D10 - sink offset 138 TESScenePhaseEvent
	EventDispatcher<void>								unkD68;					//  D68 - sink offset 140
	EventDispatcher<void>								unkDC0;					//  DC0 - sink offset 148
	EventDispatcher<void>								unkE18;					//  E18 - sink offset 150
	EventDispatcher<void>								unkE70;					//  E70 - sink offset 158
	EventDispatcher<void>								unkEC8;					//  EC8 - sink offset 160
	EventDispatcher<void>								unkF20;					//  F20 - sink offset 168 TESTopicInfoEvent
	EventDispatcher<void>								unkF78;					//  F78 - sink offset 170 TESTrackedStatsEvent
	EventDispatcher<void>								unkFD0;					//  FD0 - sink offset 178
	EventDispatcher<void>								unk1028;				// 1028 - sink offset 180 TESTriggerEvent
	EventDispatcher<void>								unk1080;				// 1080 - sink offset 188 TESTriggerEnterEvent
	EventDispatcher<void>								unk10D8;				// 10D8 - sink offset 190 TESTriggerLeaveEvent
	EventDispatcher<void>								unk1130;				// 1130 - sink offset 198 TESUniqueIDChangeEvent
	EventDispatcher<void>								unk1188;				// 1188 - sink offset 200
	EventDispatcher<void>								unk11E0;				// 11E0 - sink offset 208
	EventDispatcher<void>								unk1238;				// 1238 - sink offset 210
	EventDispatcher<TESUniqueIDChangeEvent>				uniqueIdChangeDispatcher;	// 1290 - sink offset 218
};


struct EventLog : public BSTEventSink<void>
{

	EventLog(int idx) : index(idx) {}

	EventResult ReceiveEvent(void* evnt, EventDispatcher<void>* dispatch)
	{
		printf("EventLog[0x%X] = %p\n", 0x58 * index, *(uintptr_t*)(dispatch->eventSinks[dispatch->eventSinks.count-1]) - RelocationManager::s_baseAddr);
		dispatch->RemoveEventSink(this);
		return kEvent_Continue;
	}
	
	int index;

};


void OnLoad(SKSEMessagingInterface::Message* message)
{

	if (message->type == SKSEMessagingInterface::kMessage_PostLoad) {
		auto dispatchList = GetEventDispatcherList();
		if (dispatchList) {
			EventDispatcher<void>* dispatch = reinterpret_cast<EventDispatcher<void>*>(dispatchList);
			for (int i = 0; i < 53; i++) {
				dispatch[i].AddEventSink(new EventLog(i+1));
			}
		}
	}
}


extern "C" __declspec(dllexport)
bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)
{
	info->infoVersion  = PluginInfo::kInfoVersion;
	info->name         = "Throw Things";
	info->version      = VERSION_CODE(0, 1, 0);


	return true;
}

extern "C" __declspec(dllexport)
bool SKSEPlugin_Load(const SKSEInterface * skse)
{
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	SetWindowPos(GetConsoleWindow(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	printf(" ====== START ===== \n");

	auto iMessaging = reinterpret_cast<SKSEMessagingInterface*>(skse->QueryInterface(kInterface_Messaging));
	iMessaging->RegisterListener(skse->GetPluginHandle(), "SKSE", OnLoad);
	return true;
}