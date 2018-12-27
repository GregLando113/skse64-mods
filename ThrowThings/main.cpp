#include <skse64/PluginAPI.h>
#include <skse64/GameInput.h>
#include <skse64_common/Relocation.h>
#include <skse64_common/BranchTrampoline.h>
#include <skse64_common/Utilities.h>
#include <skse64/gamethreads.h>
#include <skse64_common/SafeWrite.h>
#include <skse64/GameEvents.h>
#include <skse64/GameReferences.h>
#include <skse64/PapyrusVM.h>
#include <skse64/GameCamera.h>

#include <intrin.h>

#include <Windows.h>
#include <cstdio>

struct TESGrabReleaseEvent
{
	TESGrabReleaseEvent() : TESGrabReleaseEvent(nullptr, false) {}
	TESGrabReleaseEvent(TESObjectREFR* o, bool isheld) :
		obj(o), isHeld(isheld) {}

	TESObjectREFR* obj = nullptr;
	bool isHeld = false;
};


// 1268050 papyrus logger


TESGrabReleaseEvent g_curEvent;
unsigned g_holdTick = 0;
EventDispatcher<TESGrabReleaseEvent>* g_grabDispatch;


SKSETaskInterface* g_tasks = nullptr;

struct TESGrabReleaseEventHandler : public BSTEventSink<TESGrabReleaseEvent>
{
	EventResult ReceiveEvent(TESGrabReleaseEvent* evnt, EventDispatcher<TESGrabReleaseEvent>* dispatch) 
	{
		printf("[*] TESGrabReleaseEventHandler::ReceiveEvent obj=%p isHeld=%d ret=%p\n", evnt->obj, evnt->isHeld,  _ReturnAddress());
		g_curEvent = *evnt;
		return kEvent_Continue;
	}

	static void InstallHooks()
	{
		static TESGrabReleaseEventHandler c_inst;
		printf("TESGrabReleaseEventHandler vtbl = %p\n", *(uintptr_t*)&c_inst);
		auto dispatchList = GetEventDispatcherList();
		if (dispatchList) {
			g_grabDispatch = reinterpret_cast<EventDispatcher<TESGrabReleaseEvent>*>(&dispatchList->unk5D8);
			g_grabDispatch->AddEventSink(&c_inst);
		}
	}

};


typedef void ApplyHavokImpulse_t(VMClassRegistry* reg, UInt32 stackid, TESObjectREFR* agent, float x, float y, float z, float mag);
RelocPtr<ApplyHavokImpulse_t> g_applyHavokImpulse(0x993560);

typedef void PushActorAway_t(TESObjectREFR* agent, Actor* actor);
RelocPtr<PushActorAway_t> g_pushActorAway(0x996530);

class ApplyHavokImpulseTask : public TaskDelegate
{
public:

	ApplyHavokImpulseTask(TESObjectREFR* ref, float x, float y, float z, float mag)
		: m_ref(ref), m_x(x), m_y(y), m_z(z), m_mag(mag) {}


	void Run() override
	{
		g_applyHavokImpulse((*g_skyrimVM)->GetClassRegistry(), 0, m_ref, m_x, m_y, m_z, m_mag);
	}

	void Dispose() override
	{
		delete this;
	}

private:

	TESObjectREFR* m_ref;
	float m_x;
	float m_y;
	float m_z;
	float m_mag;
};


struct ReadyWeaponHandler 
	: public PlayerInputHandler 
{

	static const uintptr_t kReadyWeaponHandlerVtbl = 0x16892B0; // 1.5.62

	typedef void (ReadyWeaponHandler::*ProcessButton_t)(ButtonEvent* evnt, void* arg2);
	static ProcessButton_t c_originalProcess;

	void hkProcessButton(ButtonEvent* a_event, void* a_arg2)
	{


		if(!g_curEvent.isHeld)
			return (this->*c_originalProcess)(a_event, a_arg2);

		if (a_event->flags == 0) {

			//auto playerPos = (*g_thePlayer)->pos + (*g_thePlayer)->;
			//printf("playerPos = (%.2f,%.2f,%.2f)\n", playerPos.x, playerPos.y, playerPos.z);

			//auto formPos = g_curEvent.obj->pos;
			//printf("formPos = (%.2f,%.2f,%.2f)\n", formPos.x, formPos.y, formPos.z);
			//playerPos -= formPos;

			//auto mag = sqrtf(playerPos.x*playerPos.x + playerPos.y*playerPos.y + playerPos.z*playerPos.z);
			//playerPos /= mag;

			auto rot = (*g_thePlayer)->rot;

			float x = sin(rot.z);
			float y = cos(rot.z); 
			float z = -sin(rot.x);


			x *= (1.0f - abs(z));
			y *= (1.0f - abs(z));

			printf("[*] ReadyWeaponHandler::ProcessButton dev=%d evnt=%d v=(%.2f,%.2f,%.2f) flags=%X\n", a_event->deviceType, a_event->eventType, x, y, z, a_event->flags);
			g_tasks->AddTask(new ApplyHavokImpulseTask(g_curEvent.obj, x, y, z, 10 * g_holdTick));
			g_holdTick = 0;
			return;
		}

		g_holdTick++;
	}

	static void InstallHooks()
	{
		RelocPtr<ProcessButton_t> g_readyWeaponProcessButton(kReadyWeaponHandlerVtbl + (5 * 0x8));

		printf(" === ReadyWeaponHandler::InstallHooks() === %p\n", g_readyWeaponProcessButton.GetUIntPtr());
		c_originalProcess = *g_readyWeaponProcessButton;
		SafeWrite64(g_readyWeaponProcessButton.GetUIntPtr(), GetFnAddr(&hkProcessButton));
	}


};
ReadyWeaponHandler::ProcessButton_t ReadyWeaponHandler::c_originalProcess = nullptr;



void OnLoad(SKSEMessagingInterface::Message* message)
{
	if (message->type == SKSEMessagingInterface::kMessage_PostLoad) {
		ReadyWeaponHandler::InstallHooks();
		TESGrabReleaseEventHandler::InstallHooks();
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
	g_tasks = reinterpret_cast<SKSETaskInterface*>(skse->QueryInterface(kInterface_Task));
	iMessaging->RegisterListener(skse->GetPluginHandle(), "SKSE", OnLoad);
	return true;
}