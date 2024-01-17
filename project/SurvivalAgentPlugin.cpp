#include "stdafx.h"
#include "SurvivalAgentPlugin.h"
#include "BehaviourTree.h"
#include "IExamInterface.h"
#include "Blackboard.h"
#include "Brain.h"
#include "Branches.h"

SurvivalAgentPlugin::~SurvivalAgentPlugin()
{
	SAFE_DELETE(m_BehaviourTree)
	SAFE_DELETE(m_pBrain)
}

//Called only once, during initialization
void SurvivalAgentPlugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	//Information for the leaderboards!
	info.BotName = "Ren Yamashiro";
	info.Student_Name = "Tibo Van Hoorebeke"; //No special characters allowed. Highscores won't work with special characters.
	info.Student_Class = "2DAE10";
	info.LB_Password = "I_Believe_In_Acheron";//Don't use a real password! This is only to prevent other students from overwriting your highscore!

	//---------------------------------------
	// Setting up the AI
	//---------------------------------------

	m_pBrain = new Brain();

	Blackboard* pBlackboard{ CreateBlackboard() };
	m_BehaviourTree = new BT::BehaviourTree(pBlackboard, 
		new BT::Selector({
			Branch::PurgeZoneHandling(),
			Branch::ZombieHandling(),
			Branch::ItemHandling(),
			Branch::PickUpHandling(),
			Branch::HouseHandling()
		}));
}

Blackboard* SurvivalAgentPlugin::CreateBlackboard() const
{
	Blackboard* pBlackboard = new Blackboard();

	pBlackboard->AddData("Brain", m_pBrain);
	pBlackboard->AddData("Interface", m_pInterface);
	pBlackboard->AddData("Steering", SteeringPlugin_Output{});
	pBlackboard->AddData("Target", m_Target);
	pBlackboard->AddData("Spin", false);
	pBlackboard->AddData("FailSafe", std::chrono::steady_clock::time_point{});
	pBlackboard->AddData("MaxFailSafe", 2.f);
	pBlackboard->AddData("FailSafeDoOnce", false);

	pBlackboard->AddData("TargetZombie", EnemyInfo{});
	pBlackboard->AddData("angleDiff", float{});
	pBlackboard->AddData("TimerShotgun", std::chrono::steady_clock::time_point{});
	pBlackboard->AddData("TimerShotgunDoOnce", false);
	pBlackboard->AddData("MaxTimeShotgun", 1.f);
	pBlackboard->AddData("TimerPistol", std::chrono::steady_clock::time_point{});
	pBlackboard->AddData("TimerPistolDoOnce", false);
	pBlackboard->AddData("MaxTimePistol", 1.f);

	pBlackboard->AddData("TargetItem", ItemInfo{});
	pBlackboard->AddData("NextFreeSlot", 0);

	pBlackboard->AddData("TargetHouse", HouseInfo{});
	pBlackboard->AddData("TimerBeforeLeaving", std::chrono::steady_clock::time_point{});
	pBlackboard->AddData("TimerBeforeLeavingDoOnce", false);
	pBlackboard->AddData("MaxTimeBeforeLeaving", 3.f);

	return pBlackboard;
}

void SurvivalAgentPlugin::UpdateBlackboard(const SteeringPlugin_Output& steering)
{
	Blackboard* pBlackboard{ m_BehaviourTree->GetBlackboard() };

	pBlackboard->ChangeData("Steering", steering);
	pBlackboard->GetData("Target", m_Target);
}

//Called only once
void SurvivalAgentPlugin::DllInit()
{
	//Called when the plugin is loaded
}

//Called only once
void SurvivalAgentPlugin::DllShutdown()
{
	//Called when the plugin gets unloaded
}

////Called only once, during initialization. Only works in DEBUG Mode
//void SurvivalAgentPlugin::InitGameDebugParams(GameDebugParams& params)
//{
//	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
//	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
//	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
//	params.EnemyCount = 20; //How many enemies? (Default = 20)
//	params.GodMode = false; //GodMode > You can't die, can be useful to inspect certain Behaviours (Default = false)
//	params.LevelFile = "GameLevel.gppl";
//	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
//	params.StartingDifficultyStage = 1;
//	params.InfiniteStamina = false;
//	params.SpawnDebugPistol = false;
//	params.SpawnDebugShotgun = false;
//	params.SpawnPurgeZonesOnMiddleClick = true;
//	params.PrintDebugMessages = true;
//	params.ShowDebugItemNames = true;
//	params.Seed = static_cast<int>(time(nullptr)); //-1 = don't set seed. Any other number = fixed seed //TIP: use Seed = int(time(nullptr)) for pure randomness
//}
//
////Only Active in DEBUG Mode
////(=Use only for Debug Purposes)
//void SurvivalAgentPlugin::Update_Debug(float dt)
//{
//	//Demo Event Code
//	//In the end your Agent should be able to walk around without external input
//	if (m_pInterface->Input_IsMouseButtonUp(Elite::InputMouseButton::eLeft))
//	{
//		//Update_Debug target based on input
//		Elite::MouseData mouseData = m_pInterface->Input_GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft);
//		const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y));
//		m_Target = m_pInterface->Debug_ConvertScreenToWorld(pos);
//	}
//	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Space))
//	{
//		m_CanRun = true;
//	}
//	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Left))
//	{
//		m_AngSpeed -= Elite::ToRadians(10);
//	}
//	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Right))
//	{
//		m_AngSpeed += Elite::ToRadians(10);
//	}
//	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_G))
//	{
//		m_GrabItem = true;
//	}
//	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_U))
//	{
//		m_UseItem = true;
//	}
//	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_R))
//	{
//		m_RemoveItem = true;
//	}
//	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_X))
//	{
//		m_DestroyItemsInFOV = true;
//	}
//	else if (m_pInterface->Input_IsKeyboardKeyUp(Elite::eScancode_Space))
//	{
//		m_CanRun = false;
//	}
//	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Delete))
//	{
//		m_pInterface->RequestShutdown();
//	}
//	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_KP_Minus))
//	{
//		if (m_InventorySlot > 0)
//			--m_InventorySlot;
//	}
//	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_KP_Plus))
//	{
//		if (m_InventorySlot < 4)
//			++m_InventorySlot;
//	}
//	else if (m_pInterface->Input_IsKeyboardKeyDown(Elite::eScancode_Q))
//	{
//		ItemInfo info = {};
//		m_pInterface->Inventory_GetItem(m_InventorySlot, info);
//	}
//}

SteeringPlugin_Output SurvivalAgentPlugin::UpdateSteering(float dt)
{
	auto steering = SteeringPlugin_Output(Elite::Vector2{}, m_pInterface->Agent_GetInfo().MaxAngularSpeed, true, false);
	bool spin{};

	UpdateBlackboard(steering);

	m_BehaviourTree->Update();

	m_BehaviourTree->GetBlackboard()->GetData("Steering", steering);
	m_BehaviourTree->GetBlackboard()->GetData("Spin", spin);

	if (spin)
	{
		steering.AutoOrient = false;
	}

	if (m_GrabItem)
	{
		ItemInfo item;

		m_BehaviourTree->GetBlackboard()->GetData("TargetItem", item);

		if (m_pInterface->GrabItem(item))
		{
			m_pInterface->Inventory_AddItem(m_InventorySlot, item);
		}
	}

	return steering;
}

//This function should only be used for rendering debug elements
void SurvivalAgentPlugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	m_pInterface->Draw_SolidCircle(m_Target, .7f, { 0,0 }, { 1, 0, 0 });
	m_pInterface->Draw_Circle(m_pInterface->Agent_GetInfo().Position, 300.f, { 1.f, 0.f, 0 });
	m_pInterface->Draw_Circle(m_pInterface->Agent_GetInfo().Position, 100.f, { 1.f, 1.f, 0 });
}


