#include "stdafx.h"
#include "Behaviour.h"
#include "BehaviourTree.h"
#include <IExamInterface.h>
#include "Brain.h"

namespace BT_Actions
{
	BT::State SetTimer(Blackboard* pBlackboard, const std::string& timerName)
	{
		pBlackboard->ChangeData(timerName + "Timer", std::chrono::steady_clock::now());
		pBlackboard->ChangeData(timerName + "TimerLock", false);

		return BT::State::Success;
	}

	BT::State LockTimer(Blackboard* pBlackboard, const std::string& timerName)
	{
		pBlackboard->ChangeData(timerName + "TimerLock", true);

		return BT::State::Success;
	}

	BT::State FindHouse(Blackboard* pBlackboard, float radius)
	{
		IExamInterface* interfacePtr{};
		pBlackboard->GetData("Interface", interfacePtr);

		std::random_device rd;
		std::mt19937 seed(rd());
		std::uniform_int_distribution<> range(-radius, radius);

		const Elite::Vector2 randomLocation(range(seed), range(seed));
		const Elite::Vector2 target = interfacePtr->NavMesh_GetClosestPathPoint(randomLocation);

		if (randomLocation != target)
		{
			pBlackboard->ChangeData("Target", randomLocation);
			return BT::State::Success;
		}
		return BT::State::Running;
	}

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

		if (Distance(nextTargetPos, agentInfo.Position) < 2.f)
		{
			steering.LinearVelocity = Elite::ZeroVector2;

			std::cout << "target reached at " << target << "\n";

			return BT::State::Success;
		}

		pBlackboard->ChangeData("Steering", steering);

		return BT::State::Running;
	}

	BT::State GetHouseAsTarget(Blackboard* pBlackboard)
	{
		Brain* pBrain{};
		IExamInterface* pInterface{};
		float maxTravelDistance{};

		pBlackboard->GetData("Brain", pBrain);
		pBlackboard->GetData("Interface", pInterface);
		pBlackboard->GetData("MaxTravelDistance", maxTravelDistance);

		const HouseInfo targetHouse{ pBrain->CheckHouseTarget(pInterface->Agent_GetInfo().Position, maxTravelDistance) };

		if (targetHouse.Size.x <= 0)
		{
			return BT::State::Failure;
		}

		pBlackboard->ChangeData("Target", targetHouse.Center);

		std::cout << targetHouse.Center << std::endl;
		return BT::State::Success;
	}

	BT::State CheckHouses(Blackboard* pBlackboard)
	{
		Brain* pBrain{};
		IExamInterface* pInterface{};

		pBlackboard->GetData("Brain", pBrain);
		pBlackboard->GetData("Interface", pInterface);

		if (pBrain->CheckHouses(pInterface->GetHousesInFOV()))
		{
			return BT::State::Success;
		}

		return BT::State::Failure;
	}

}

namespace BT_Conditions
{
	bool SeeHouse(Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{};
		pBlackboard->GetData("Interface", pInterface);

		return pInterface->GetHousesInFOV().capacity() > 0;
	}

	bool TimerReached(Blackboard* pBlackboard, const std::string& timerName)
	{
		bool lock{};
		pBlackboard->GetData(timerName + "TimerLock", lock);

		if (lock)
			return false;

		std::chrono::steady_clock::time_point timer{};
		float maxTime{};

		pBlackboard->GetData(timerName + "Timer", timer);
		pBlackboard->GetData("max" + timerName + "Timer", maxTime);

		const std::chrono::steady_clock::time_point currentTime{ std::chrono::steady_clock::now() };
		const std::chrono::duration<float> elapsedSec{ currentTime - timer };

		const bool result = elapsedSec.count() > maxTime;
		std::cout << result << std::endl;
		return result;
	}

	bool NewHouse(Blackboard* pBlackboard)
	{
		Brain* pBrain{};
		pBlackboard->GetData("Brain", pBrain);

		return pBrain->NewHouseToExplore();
	}
}