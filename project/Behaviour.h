#pragma once

namespace BT
{
	enum class State;
}

class Blackboard;

namespace BT_Actions
{
	BT::State FindAHouse(Blackboard* pBlackboard);
	BT::State GoToDestination(Blackboard* pBlackboard);
}

namespace BT_Conditions
{
}