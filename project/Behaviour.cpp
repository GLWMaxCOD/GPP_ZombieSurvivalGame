#include "stdafx.h"
#include "Behaviour.h"
#include "BehaviourTree.h"
#include <IExamInterface.h>
#include "Brain.h"

static int randNumRange(int minRange, int maxRange)
{
	std::random_device rd;
	std::mt19937 seed(rd());
	std::uniform_int_distribution<> range(minRange, maxRange);

	return range(seed);
}

namespace BT_Actions
{
	BT::State GoToDestination(Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{};
		Elite::Vector2 target{};
		SteeringPlugin_Output steering{};

		pBlackboard->GetData("Interface", pInterface);
		pBlackboard->GetData("Target", target);
		pBlackboard->GetData("Steering", steering);

		std::cout << "target received " << target << "\n";

		const auto agentInfo = pInterface->Agent_GetInfo();

		const auto nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(target);

		steering.LinearVelocity = nextTargetPos - agentInfo.Position;
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= agentInfo.MaxLinearSpeed;

		if (Distance(target, agentInfo.Position) < 2.f)
		{
			steering.LinearVelocity = Elite::ZeroVector2;

			std::cout << "target reached at " << target << "\n";

			return BT::State::Success;
		}

		pBlackboard->ChangeData("Steering", steering);

		return BT::State::Running;
	}

	BT::State EnableSpin(Blackboard* pBlackboard)
	{
		pBlackboard->ChangeData("Spin", true);

		return BT::State::Success;
	}

	BT::State DisableSpin(Blackboard* pBlackboard)
	{
		pBlackboard->ChangeData("Spin", false);

		return BT::State::Success;
	}

	BT::State SetItemAsTarget(Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{};
		pBlackboard->GetData("Interface", pInterface);

		if (pInterface->GetItemsInFOV().capacity() == 0)
		{
			return BT::State::Failure;
		}

		ItemInfo targetItem{};
		float closestItem{ FLT_MAX };

		for (const auto item : pInterface->GetItemsInFOV())
		{
			const float itemDistance{ pInterface->Agent_GetInfo().Position.DistanceSquared(item.Location) };

			if (closestItem < itemDistance)
				continue;

			closestItem = itemDistance;
			targetItem = item;
		}

		pBlackboard->ChangeData("TargetItem", targetItem);
		pBlackboard->ChangeData("Target", targetItem.Location);

		return BT::State::Success;
	}

	BT::State DestroyItemOnFloor(Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{};
		ItemInfo targetItem{};

		pBlackboard->GetData("Interface", pInterface);
		pBlackboard->GetData("TargetItem", targetItem);

		pInterface->DestroyItem(targetItem);

		return BT::State::Success;
	}

	BT::State PickUpItem(Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{};
		Brain* pBrain{};
		ItemInfo targetItem{};
		int freeSlot{};

		pBlackboard->GetData("Interface", pInterface);
		pBlackboard->GetData("Brain", pBrain);
		pBlackboard->GetData("TargetItem", targetItem);
		pBlackboard->GetData("NextFreeSlot", freeSlot);

		pInterface->GrabItem(targetItem);
		pInterface->Inventory_AddItem(freeSlot, targetItem);

		pBlackboard->ChangeData("NextFreeSlot", pBrain->AddItemToMemory(targetItem));

		return BT::State::Success;
	}

	BT::State TryFindHouse(Blackboard* pBlackboard, int searchRadius)
	{
		IExamInterface* pInterface{};

		pBlackboard->GetData("Interface", pInterface);

		const Elite::Vector2 randomLocation(randNumRange(-searchRadius, searchRadius),
			randNumRange(-searchRadius, searchRadius));

		const Elite::Vector2 target = pInterface->NavMesh_GetClosestPathPoint(randomLocation);

		if (randomLocation != target)
		{
			pBlackboard->ChangeData("Target", randomLocation);

			return BT::State::Success;
		}

		return BT::State::Running;
	}

	BT::State GetHouseAsTarget(Blackboard* pBlackboard, float maxTravelDistance)
	{
		Brain* pBrain{};
		IExamInterface* pInterface{};

		pBlackboard->GetData("Brain", pBrain);
		pBlackboard->GetData("Interface", pInterface);

		const HouseInfo targetHouse{ pBrain->CheckHouseValidTarget(pInterface->Agent_GetInfo().Position, maxTravelDistance) };

		if (targetHouse.Size.x <= 0)
		{
			return BT::State::Failure;
		}

		pBlackboard->ChangeData("TargetHouse", targetHouse);

		return BT::State::Success;
	}

	BT::State CheckHouses(Blackboard* pBlackboard)
	{
		Brain* pBrain{};
		IExamInterface* pInterface{};

		pBlackboard->GetData("Brain", pBrain);
		pBlackboard->GetData("Interface", pInterface);

		if (pBrain->CheckHousesForMemory(pInterface->GetHousesInFOV()))
		{
			return BT::State::Success;
		}

		return BT::State::Failure;
	}

	BT::State SetExpireDate(Blackboard* pBlackboard)
	{
		Brain* pBrain{};
		HouseInfo targetHouse{};

		pBlackboard->GetData("Brain", pBrain);
		pBlackboard->GetData("TargetHouse", targetHouse);

		pBrain->SetTargetHouseExpireDate(targetHouse);

		return BT::State::Success;
	}

	BT::State GetOutsideTarget(Blackboard* pBlackboard, int offset)
	{
		HouseInfo targetHouse{};
		pBlackboard->GetData("TargetHouse", targetHouse);

		const Elite::Vector2 houseSize{ targetHouse.Size };

		const Elite::Vector2 targetLocation(randNumRange(int(houseSize.x), int(houseSize.x + offset)),
			randNumRange(int(houseSize.y), int(houseSize.y + offset)));

		pBlackboard->ChangeData("Target", targetLocation);

		return BT::State::Success;
	}
}

namespace BT_Conditions
{
	bool SeeItem(Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{};
		pBlackboard->GetData("Interface", pInterface);

		return pInterface->GetItemsInFOV().capacity() > 0;
	}

	bool IsTypeOfItem(Blackboard* pBlackboard, eItemType typoToCheck)
	{
		ItemInfo targetItem{};
		pBlackboard->GetData("TargetItem", targetItem);

		return targetItem.Type == typoToCheck;
	}

	bool InvIsFull(Blackboard* pBlackboard)
	{
		Brain* pBrain{};
		pBlackboard->GetData("Brain", pBrain);

		return pBrain->IsInvFull();
	}

	bool InvIsNotFull(Blackboard* pBlackboard)
	{
		return !InvIsFull(pBlackboard);
	}

	bool InsideHouse(Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{};
		pBlackboard->GetData("Interface", pInterface);

		return pInterface->Agent_GetInfo().IsInHouse;
	}

	bool SeeHouse(Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{};
		pBlackboard->GetData("Interface", pInterface);

		return pInterface->GetHousesInFOV().capacity() > 0;
	}

	bool NewHouse(Blackboard* pBlackboard)
	{
		Brain* pBrain{};
		pBlackboard->GetData("Brain", pBrain);

		return pBrain->NewHouseToExplore();
	}

	bool ReExploreHouse(Blackboard* pBlackboard)
	{
		Brain* pBrain{};
		pBlackboard->GetData("Brain", pBrain);

		return pBrain->HouseToReExplore();
	}
}