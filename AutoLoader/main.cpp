#include <skse64/PluginAPI.h>
#include <skse64/GameInput.h>
#include <skse64_common/Relocation.h>
#include <skse64_common/BranchTrampoline.h>
#include <skse64_common/Utilities.h>
#include <skse64/gamethreads.h>
#include <skse64_common/SafeWrite.h>
#include <skse64/GameEvents.h>
#include <skse64/PapyrusNativeFunctions.h>

#include <Windows.h>
#include <cstdio>


SKSETaskInterface* g_tasks;

// centeroncell cb=96DA30 vt=16F2D58


typedef void CenterOnCell_t(VMClassRegistry* cls, UInt64 stackid, StaticFunctionTag*, BSFixedString&);
RelocPtr<CenterOnCell_t> g_centerOnCell(0x96DA30);

struct LoadGameTask : TaskDelegate {

	void Run() override
	{
		static StaticFunctionTag t;
		static BSFixedString str("Whiterun");
		g_centerOnCell((*g_skyrimVM)->GetClassRegistry(), 0, &t, str);
	}

	void Dispose() override
	{
		delete this;
	}


};


void OnLoad(SKSEMessagingInterface::Message* message)
{
	if (message->type == SKSEMessagingInterface::kMessage_PostPostLoad) 
	{
		g_tasks->AddTask(new LoadGameTask());
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

	g_tasks = reinterpret_cast<SKSETaskInterface*>(skse->QueryInterface(kInterface_Task));
	auto iMessaging = reinterpret_cast<SKSEMessagingInterface*>(skse->QueryInterface(kInterface_Messaging));
	iMessaging->RegisterListener(skse->GetPluginHandle(), "SKSE", OnLoad);
	return true;
}