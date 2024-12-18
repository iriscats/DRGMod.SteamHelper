#include <stdio.h>
#include <bit>
#include <string>
#include <thread>
#include <chrono>

#include <Mod/CppUserModBase.hpp>
#include <DynamicOutput/DynamicOutput.hpp>
#include <Unreal/UObject.hpp>
#include <Unreal/UFunction.hpp>
#include <Unreal/BPMacros.hpp>
#include <Unreal/FProperty.hpp>
#include <Unreal/UStruct.hpp>
#include <Unreal/UObjectGlobals.hpp>
#include <UE4SSProgram.hpp>

using namespace std;
using namespace RC;
using namespace RC::Unreal;


struct InputParams {
	UObject* Player;
} in;

FString property_text{};
bool has_result = false;
bool is_hooked = false;

void HookPre_SteamHelper_GetSteamId(UnrealScriptFunctionCallableContext& context, void* custom_data)
{
	has_result = false;
	//Output::send<LogLevel::Verbose>(STR("HookPre_WaveTimer_GetWaveTimes\n"));
	auto params = context.GetParams<InputParams>();

	std::vector<UObject*> connections;
	UObjectGlobals::FindAllOf(STR("SteamSocketsNetConnection"), connections);
	for (auto& connection_object : connections)
	{
		UObject* view_target_ptr = *connection_object->GetValuePtrByPropertyNameInChain<UObject*>(STR("ViewTarget"));
		if (view_target_ptr && params.Player == view_target_ptr)
		{
			auto player_id_property = connection_object->GetPropertyByNameInChain(STR("PlayerId"));
			if (player_id_property)
			{
				has_result = true;
				auto connection_object_ptr = player_id_property->ContainerPtrToValuePtr<void*>(connection_object);
				property_text.Clear();
				player_id_property->ExportTextItem(property_text, connection_object_ptr, connection_object_ptr, static_cast<UObject*>(connection_object), NULL);
			}
		}
	}
}


void HookPost_SteamHelper_GetSteamId(UnrealScriptFunctionCallableContext& context, void* custom_data)
{
	if (has_result)
	{
		auto params = context.TheStack.OutParms();
		auto out_param = (FString*)params->PropAddr;
		out_param->Clear();
		out_param->SetCharArray(property_text.GetCharTArray());
	}
}


void InitHook() {
	if (is_hooked) {
		return;
	}
	// You are allowed to use the 'Unreal' namespace in this function and anywhere else after this function has fired.
	auto GetSteamIdFunction = UObjectGlobals::StaticFindObject<UFunction*>(nullptr, nullptr, STR("/Game/SteamHelper/SteamHelper.SteamHelper_C:GetSteamIds"));

	Output::send<LogLevel::Verbose>(STR("Object Name: {}\n"), GetSteamIdFunction->GetFullName());
	if (GetSteamIdFunction) {

		UObjectGlobals::RegisterHook(GetSteamIdFunction, HookPre_SteamHelper_GetSteamId, HookPost_SteamHelper_GetSteamId, nullptr);
		Output::send<LogLevel::Verbose>(STR("Hook success\n"));
		is_hooked = true;
	}
}


class MyAwesomeMod : public RC::CppUserModBase
{
public:
	MyAwesomeMod() : CppUserModBase()
	{
		ModName = STR("SteamHelper");
		ModVersion = STR("1.0");
		ModDescription = STR("This is my awesome mod");
		ModAuthors = STR("iriscat");
		// Do not change this unless you want to target a UE4SS version
		// other than the one you're currently building with somehow.
		//ModIntendedSDKVersion = STR("2.6");
	}

	~MyAwesomeMod() override
	{
	}

	auto on_update() -> void override
	{
		InitHook();
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}

	auto on_unreal_init() -> void override
	{
		// You are allowed to use the 'Unreal' namespace in this function and anywhere else after this function has fired.
	}

	auto on_program_start() -> void override
	{
	}

};

#define MY_AWESOME_MOD_API __declspec(dllexport)
extern "C"
{
	MY_AWESOME_MOD_API RC::CppUserModBase* start_mod()
	{
		return new MyAwesomeMod();
	}

	MY_AWESOME_MOD_API void uninstall_mod(RC::CppUserModBase* mod)
	{
		delete mod;
	}
}