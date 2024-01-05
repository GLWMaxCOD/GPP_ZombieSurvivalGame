#pragma once
#include <Exam_HelperStructs.h>

#include "Blackboard.h"

namespace BT
{
	enum class State;
}

class Blackboard;

namespace BT_Actions
{
	BT::State GoToDestination(Blackboard* pBlackboard);
	BT::State SetExpireDate(Blackboard* pBlackboard);
	BT::State EnableSpin(Blackboard* pBlackboard);
	BT::State DisableSpin(Blackboard* pBlackboard);

	BT::State SetItemAsTarget(Blackboard* pBlackboard);
	BT::State DestroyItemOnFloor(Blackboard* pBlackboard);
	BT::State PickUpItem(Blackboard* pBlackboard);

	BT::State TryFindHouse(Blackboard* pBlackboard, int searchRadius);
	BT::State GetHouseAsTarget(Blackboard* pBlackboard, float maxTravelDistance);
	BT::State CheckHouses(Blackboard* pBlackboard);
	BT::State GetOutsideTarget(Blackboard* pBlackboard, int offset);
}

namespace BT_Conditions
{
	bool SeeItem(Blackboard* pBlackboard);
	bool IsTypeOfItem(Blackboard* pBlackboard, eItemType typoToCheck);
	bool InvIsFull(Blackboard* pBlackboard);
	bool InvIsNotFull(Blackboard* pBlackboard);

	bool InsideHouse(Blackboard* pBlackboard);
	bool NewHouse(Blackboard* pBlackboard);
	bool SeeHouse(Blackboard* pBlackboard);
	bool ReExploreHouse(Blackboard* pBlackboard);
}