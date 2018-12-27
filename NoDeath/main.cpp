#include <skse64/PluginAPI.h>
#include <skse64/GameInput.h>
#include <skse64/GameAPI.h>
#include <skse64/GameReferences.h>
#include <skse64_common/Relocation.h>
#include <skse64_common/BranchTrampoline.h>
#include <skse64_common/Utilities.h>
#include <skse64/gamethreads.h>
#include <skse64_common/SafeWrite.h>

#include <skse64/GameEvents.h>

#include <Windows.h>
#include <cstdio>


struct NoDeathEvent : public BSTEventSink<TESDeathEvent> {

	EventResult ReceiveEvent(TESDeathEvent* evnt, EventDispatcher<TESDeathEvent>* dis) {
		
		if (evnt->source == *g_thePlayer) {
			printf("NoDeathEvent state=%d\n", evnt->state);
			evnt->state = 0;
			return kEvent_Abort;
		}
		return kEvent_Continue;
	}

} g_noDeath;

template <typename EventT, typename EventArgT = EventT>
struct DaEventDispatcher
{
	typedef BSTEventSink<EventT> SinkT;

	tArray<SinkT*>		eventSinks;			// 000
	tArray<SinkT*>		addBuffer;			// 018 - schedule for add
	tArray<SinkT*>		removeBuffer;		// 030 - schedule for remove
	SimpleLock			lock;				// 048
	bool				stateFlag;			// 050 - some internal state changed while sending
	char				pad[7];				// 051
};

void OnLoad(SKSEMessagingInterface::Message* message)
{
	if (message->type == SKSEMessagingInterface::kMessage_PostLoadGame) {
		if (!GetEventDispatcherList())
			printf("GetEventDispatcherList() = null\n");
		else {
			auto deathDispatcher = reinterpret_cast<DaEventDispatcher<TESDeathEvent>*>(&(GetEventDispatcherList()->deathDispatcher));
			deathDispatcher->eventSinks.Clear();
		}
	}
}


extern "C" __declspec(dllexport)
bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)
{
	info->infoVersion  = PluginInfo::kInfoVersion;
	info->name         = "No Death";
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