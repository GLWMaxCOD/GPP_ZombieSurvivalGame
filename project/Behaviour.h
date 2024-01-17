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
	BT::State SetTimer(Blackboard* pBlackboard, const std::string& timerName, bool doOnce);
	BT::State UnlockTimer(Blackboard* pBlackboard, const std::string& timerName);
	BT::State LockTimer(Blackboard* pBlackboard, const std::string& timerName);

	BT::State GoToDestination(Blackboard* pBlackboard);
	BT::State SetExpireDate(Blackboard* pBlackboard);
	BT::State EnableSpin(Blackboard* pBlackboard);
	BT::State DisableSpin(Blackboard* pBlackboard);
	BT::State FindClosestEdge(Blackboard* pBlackboard, int degree);

	BT::State SetZombieTarget(Blackboard* pBlackboard);
	BT::State AvoidingZombie(Blackboard* pBlackboard);
	BT::State RotateToZombie(Blackboard* pBlackboard);
	BT::State ReadyToShoot(Blackboard* pBlackboard, float minAngleDiff);
	BT::State Shoot(Blackboard* pBlackboard, eItemType type);

	BT::State SetItemAsTarget(Blackboard* pBlackboard);
	BT::State DestroyItemOnFloor(Blackboard* pBlackboard);
	BT::State PickUpItem(Blackboard* pBlackboard);
	BT::State SwapItem(Blackboard* pBlackboard);
	BT::State CheckItem(Blackboard* pBlackboard);
	BT::State UseItem(Blackboard* pBlackboard, eItemType type);

	BT::State FindRandomLocation(Blackboard* pBlackboard, float randomRadius);
	BT::State TryFindHouse(Blackboard* pBlackboard, float searchRadius, int degree);
	BT::State GetHouseAsTarget(Blackboard* pBlackboard, float maxTravelDistance);
	BT::State CheckHouses(Blackboard* pBlackboard);
	BT::State GetInsideTarget(Blackboard* pBlackboard, float offset);
}

namespace BT_Conditions
{
	bool CheckTimerLock(Blackboard* pBlackboard, const std::string& timerName);
	bool CheckTimer(Blackboard* pBlackboard, const std::string& timerName, bool doOnce);

	bool SeeZombie(Blackboard* pBlackboard);
	bool HasWeapon(Blackboard* pBlackboard);
	bool InRange(Blackboard* pBlackboard, float maxRange);

	bool InPurgeZone(Blackboard* pBlackboard);

	bool SeeItem(Blackboard* pBlackboard);
	bool IsTypeOfItem(Blackboard* pBlackboard, eItemType type);
	bool InvIsFull(Blackboard* pBlackboard);
	bool InvIsNotFull(Blackboard* pBlackboard);
	bool EmptyValue(Blackboard* pBlackboard);
	bool ItemInInv(Blackboard* pBlackboard, eItemType type);
	bool HpUnderThreshold(Blackboard* pBlackboard, float threshold);
	bool CheckMinNeededEnergy(Blackboard* pBlackboard);

	bool InsideTargetHouse(Blackboard* pBlackboard);
	bool NewHouse(Blackboard* pBlackboard);
	bool SeeHouse(Blackboard* pBlackboard);
	bool ReExploreHouse(Blackboard* pBlackboard);
}