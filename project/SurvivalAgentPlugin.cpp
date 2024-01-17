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
			Branch::ItemHandling(),
			Branch::PurgeZoneHandling(),
			Branch::ZombieHandling(),
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

SteeringPlugin_Output SurvivalAgentPlugin::UpdateSteering(float dt)
{
	auto steering = SteeringPlugin_Output(Elite::Vector2{}, m_pInterface->Agent_GetInfo().MaxAngularSpeed, true, false);
	bool spin{};

	UpdateBlackboard(steering);

	m_BehaviourTree->Update();

	m_BehaviourTree->GetBlackboard()->GetData("Steering", steering);
	m_BehaviourTree->GetBlackboard()->GetData("Spin", spin);

	if (m_pInterface->Agent_GetInfo().Stamina >= 10 || m_Running)
	{
		m_Running = true;
		steering.RunMode = true;
	}

	if (m_pInterface->Agent_GetInfo().Stamina <= 0.1f)
	{
		m_Running = false;
		steering.RunMode = false;
	}

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

	m_pInterface->Draw_Circle(m_pInterface->Agent_GetInfo().Position, 500.f, { 1.f, 0.f, 0 });
	m_pInterface->Draw_Circle(m_pInterface->Agent_GetInfo().Position, 400.f, { 1.f, 1.f, 0 });
	m_pInterface->Draw_Circle(m_pInterface->Agent_GetInfo().Position, 50.f, { 1.f, 1.f, 1.f });

	m_pInterface->Draw_Circle(m_pInterface->Agent_GetInfo().Position, 15.f, { 0.f, 1.f, 0.5f });
	m_pInterface->Draw_Circle(m_pInterface->Agent_GetInfo().Position, 7.5f, { 0.f, 0.5f, 1.f });
}


