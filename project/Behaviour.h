#pragma once
#include <chrono>

#include "Blackboard.h"

namespace BT
{
	enum class State;
}

class Blackboard;

namespace BT_Actions
{
	BT::State SetTimer(Blackboard* pBlackboard, const std::string& timerName);
	BT::State LockTimer(Blackboard* pBlackboard, const std::string& timerName);

	BT::State GoToDestination(Blackboard* pBlackboard);

	BT::State GetHouseAsTarget(Blackboard* pBlackboard);
	BT::State FindHouse(Blackboard* pBlackboard, float radius);
	BT::State CheckHouses(Blackboard* pBlackboard);
}

namespace BT_Conditions
{
	bool TimerReached(Blackboard* pBlackboard, const std::string& timerName);

	bool NewHouse(Blackboard* pBlackboard);
	bool SeeHouse(Blackboard* pBlackboard);
}