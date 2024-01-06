#include "stdafx.h"
#include "Behaviour.h"
#include "BehaviourTree.h"
#include <IExamInterface.h>
#include "Brain.h"

#define TO_RAD(i) i * (M_PI / 180)

static int randNumRange(int minRange, int maxRange)
{
	std::random_device rd;
	std::mt19937 seed(rd());
	std::uniform_int_distribution<> range(minRange, maxRange);

	return range(seed);
}

namespace BT_Actions
{
	BT::State SetTimer(Blackboard* pBlackboard, const std::string& timerName, bool doOnce)
	{
		bool didOnce{};
		pBlackboard->GetData("Timer" + timerName + "DoOnce", didOnce);

		if (doOnce && didOnce)
			return BT::State::Success;

		if (doOnce)
			pBlackboard->ChangeData("Timer" + timerName + "DoOnce", true);

		pBlackboard->ChangeData("Timer" + timerName, std::chrono::steady_clock::now());

		return BT::State::Success;
	}

	BT::State UnlockTimer(Blackboard* pBlackboard, const std::string& timerName)
	{
		pBlackboard->ChangeData("TimerLock" + timerName, false);

		return BT::State::Success;
	}

	BT::State LockTimer(Blackboard* pBlackboard, const std::string& timerName)
	{
		pBlackboard->ChangeData("TimerLock" + timerName, true);

		return BT::State::Success;
	}

	BT::State GoToDestination(Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{};
		Elite::Vector2 target{};
		SteeringPlugin_Output steering{};

		std::chrono::steady_clock::time_point timer{};
		float maxTime{};
		bool doOnce{};

		pBlackboard->GetData("Interface", pInterface);
		pBlackboard->GetData("Target", target);
		pBlackboard->GetData("Steering", steering);

		pBlackboard->GetData("FailSafe", timer);
		pBlackboard->GetData("MaxFailSafe", maxTime);
		pBlackboard->GetData("FailSafeDoOnce", doOnce);

		if (!doOnce)
		{
			pBlackboard->ChangeData("FailSafe", std::chrono::steady_clock::now());
			pBlackboard->ChangeData("FailSafeDoOnce", true);
		}

		const auto agentInfo = pInterface->Agent_GetInfo();

		const auto nextTargetPos = pInterface->NavMesh_GetClosestPathPoint(target);

		steering.LinearVelocity = nextTargetPos - agentInfo.Position;
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= agentInfo.MaxLinearSpeed * 100;

		const std::chrono::steady_clock::time_point currentTime{ std::chrono::steady_clock::now() };
		const std::chrono::duration<float> elapsedSec{ currentTime - timer };

		if (elapsedSec.count() > maxTime)
		{
			pBlackboard->ChangeData("FailSafeDoOnce", false);
			return BT::State::Success;
		}

		if (Distance(target, agentInfo.Position) < 2.f)
		{
			pBlackboard->ChangeData("FailSafeDoOnce", false);

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

		if (pInterface->DestroyItem(targetItem))
			return BT::State::Success;

		return BT::State::Failure;
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

		if (pInterface->GrabItem(targetItem))
		{
			pInterface->Inventory_AddItem(freeSlot, targetItem);
			pBlackboard->ChangeData("NextFreeSlot", pBrain->AddItemToMemory(targetItem));
			return BT::State::Success;
		}

		return BT::State::Failure;
	}

	BT::State SwapItem(Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{};
		Brain* pBrain{};
		ItemInfo targetItem{};

		pBlackboard->GetData("Interface", pInterface);
		pBlackboard->GetData("Brain", pBrain);
		pBlackboard->GetData("TargetItem", targetItem);

		if (pInterface->GrabItem(targetItem))
		{
			const int slot = pBrain->FindEmptyValue(targetItem);
			pInterface->Inventory_RemoveItem(slot);
			pInterface->Inventory_AddItem(slot, targetItem);

			return BT::State::Success;
		}

		return BT::State::Failure;
	}

	BT::State CheckItem(Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{};
		Brain* pBrain{};
		ItemInfo targetItem{};

		pBlackboard->GetData("Interface", pInterface);
		pBlackboard->GetData("Brain", pBrain);
		pBlackboard->GetData("TargetItem", targetItem);

		const int slotIndex{ pBrain->CheckItem(targetItem) };

		if (slotIndex == pInterface->Inventory_GetCapacity() - 1)
		{
			if (targetItem.Type == eItemType::SHOTGUN || targetItem.Type == eItemType::PISTOL)
			{
				pInterface->DestroyItem(targetItem);
				return BT::State::Success;
			}

			if (pInterface->GrabItem(targetItem))
			{
				pInterface->Inventory_AddItem(slotIndex, targetItem);
				pInterface->Inventory_UseItem(slotIndex);
				pInterface->Inventory_RemoveItem(slotIndex);
				return BT::State::Success;
			}
		}
		else
		{
			if (!(targetItem.Type == eItemType::SHOTGUN || targetItem.Type == eItemType::PISTOL))
			{
				pInterface->Inventory_UseItem(slotIndex);
			}

			if (pInterface->GrabItem(targetItem))
			{
				pInterface->Inventory_RemoveItem(slotIndex);
				pInterface->Inventory_AddItem(slotIndex, targetItem);
				return BT::State::Success;
			}
		}

		return BT::State::Failure;
	}

	BT::State UseItem(Blackboard* pBlackboard, eItemType type)
	{
		IExamInterface* pInterface{};
		Brain* pBrain{};

		pBlackboard->GetData("Interface", pInterface);
		pBlackboard->GetData("Brain", pBrain);

		const auto item = pBrain->FindLeastValueItem(type);

		if (item->ItemInfo.Value <= 0)
			return BT::State::Failure;

		if (pInterface->Inventory_UseItem(item->invIndex))
		{
			item->ItemInfo.Value = 0;
			return BT::State::Success;
		}

		return BT::State::Failure;
	}

	BT::State TryFindHouse(Blackboard* pBlackboard, float searchRadius, int degree)
	{
		IExamInterface* pInterface{};
		Brain* pBrain{};

		pBlackboard->GetData("Interface", pInterface);
		pBlackboard->GetData("Brain", pBrain);

		pBlackboard->ChangeData("TargetHouse", HouseInfo{});

		const Elite::Vector2 playerPos{ pInterface->Agent_GetInfo().Position };

		float closestTarget{ FLT_MAX };
		Elite::Vector2 finalTarget{};

		constexpr int circleDegrees{ 360 };

		if (degree > circleDegrees)
		{
			degree = circleDegrees;
		}

		for (int i = 0; i <= circleDegrees; i += degree)
		{
			const Elite::Vector2 pointOnCircle{ playerPos.x + searchRadius * std::cosf(TO_RAD(i)), playerPos.y + searchRadius * std::sinf(TO_RAD(i)) };
			const Elite::Vector2 target = pInterface->NavMesh_GetClosestPathPoint(pointOnCircle);

			if (pointOnCircle != target)
			{
				constexpr float houseOffset{ 5.f };
				if (pBrain->CheckIfTargetIsExplored(target, houseOffset))
					continue;

				const float targetDistance{ playerPos.DistanceSquared(target) };

				if (closestTarget > targetDistance)
				{
					closestTarget = targetDistance;
					finalTarget = target;
				}
			}
		}

		if (finalTarget == Elite::Vector2{})
		{
			finalTarget = Elite::Vector2(randNumRange(-searchRadius, searchRadius),
										 randNumRange(-searchRadius, searchRadius));
		}

		pBlackboard->ChangeData("Target", finalTarget);

		return BT::State::Success;
	}

	BT::State GetHouseAsTarget(Blackboard* pBlackboard, float maxTravelDistance)
	{
		IExamInterface* pInterface{};
		Brain* pBrain{};

		pBlackboard->GetData("Interface", pInterface);
		pBlackboard->GetData("Brain", pBrain);

		const HouseInfo targetHouse{ pBrain->CheckHouseValidTarget(pInterface->Agent_GetInfo().Position, maxTravelDistance) };

		if (targetHouse.Size.x <= 0)
		{
			return BT::State::Failure;
		}

		pBlackboard->ChangeData("Target", targetHouse.Center);
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

	BT::State GetInsideTarget(Blackboard* pBlackboard, float offset)
	{
		HouseInfo targetHouse{};
		pBlackboard->GetData("TargetHouse", targetHouse);

		const Elite::Vector2 houseSize{ targetHouse.Size };
		const Elite::Vector2 houseCenter{ targetHouse.Center };

		const Elite::Vector2 targetLocation(randNumRange(int(houseCenter.x - houseSize.x / 2 + offset), int(houseCenter.x + houseSize.x / 2 - offset)),
											randNumRange(int(houseCenter.y - houseSize.y / 2 + offset), int(houseCenter.y + houseSize.y / 2 - offset)));

		pBlackboard->ChangeData("Target", targetLocation);

		return BT::State::Success;
	}
}

namespace BT_Conditions
{
	bool CheckTimerLock(Blackboard* pBlackboard, const std::string& timerName)
	{
		bool lock{};
		pBlackboard->GetData(timerName + "TimerLock", lock);

		return !lock;
	}

	bool CheckTimer(Blackboard* pBlackboard, const std::string& timerName, bool doOnce)
	{
		std::chrono::steady_clock::time_point timer{};
		float maxTime{};

		pBlackboard->GetData("Timer" + timerName, timer);
		pBlackboard->GetData("MaxTime" + timerName, maxTime);

		const std::chrono::steady_clock::time_point currentTime{ std::chrono::steady_clock::now() };
		const std::chrono::duration<float> elapsedSec{ currentTime - timer };

		if (elapsedSec.count() > maxTime)
		{
			if (doOnce)
				pBlackboard->ChangeData("Timer" + timerName + "DoOnce", false);

			return true;
		}

		return false;
	}

	bool SeeItem(Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{};
		pBlackboard->GetData("Interface", pInterface);

		return pInterface->GetItemsInFOV().capacity() > 0;
	}

	bool IsTypeOfItem(Blackboard* pBlackboard, eItemType type)
	{
		ItemInfo targetItem{};
		pBlackboard->GetData("TargetItem", targetItem);

		return targetItem.Type == type;
	}

	bool InvIsFull(Blackboard* pBlackboard)
	{
		return !InvIsNotFull(pBlackboard);
	}

	bool InvIsNotFull(Blackboard* pBlackboard)
	{
		Brain* pBrain{};
		pBlackboard->GetData("Brain", pBrain);
		return pBrain->IsInvNotFull();
	}

	bool EmptyValue(Blackboard* pBlackboard)
	{
		Brain* pBrain{};
		pBlackboard->GetData("Brain", pBrain);

		if (pBrain->EmptyValue())
		{
			std::cout << "empty item\n";
		}

		return pBrain->EmptyValue();
	}

	bool ItemInInv(Blackboard* pBlackboard, eItemType type)
	{
		Brain* pBrain{};
		pBlackboard->GetData("Brain", pBrain);

		return pBrain->IsItemInInv(type);
	}

	bool HpUnderThreshold(Blackboard* pBlackboard, float threshold)
	{
		IExamInterface* pInterface{};
		pBlackboard->GetData("Interface", pInterface);

		return pInterface->Agent_GetInfo().Health <= threshold;
	}

	bool CheckMinNeededEnergy(Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{};
		Brain* pBrain{};

		pBlackboard->GetData("Interface", pInterface);
		pBlackboard->GetData("Brain", pBrain);

		const auto item = pBrain->FindLeastValueItem(eItemType::FOOD);

		return pInterface->Agent_GetInfo().Energy <= 10.f - item->ItemInfo.Value;
	}

	bool InsideTargetHouse(Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{};
		Brain* pBrain{};
		HouseInfo targetHouse{};

		pBlackboard->GetData("Interface", pInterface);
		pBlackboard->GetData("Brain", pBrain);
		pBlackboard->GetData("TargetHouse", targetHouse);

		if (pInterface->Agent_GetInfo().IsInHouse)
		{
			return pBrain->CheckIfTargetIsInside(targetHouse, pInterface->Agent_GetInfo().Position);
		}

		return false;
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