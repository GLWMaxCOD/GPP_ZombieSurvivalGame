#include "stdafx.h"
#include "Behaviour.h"
#include "BehaviourTree.h"
#include <IExamInterface.h>

namespace BT_Actions
{
	BT::State FindAHouse(Blackboard* pBlackboard)
	{
		IExamInterface* pInterface{};
		pBlackboard->GetData("Interface", pInterface);

		const Elite::Vector2 worldDimensions{ pInterface->World_GetInfo().Dimensions };

		std::random_device rd; // obtain a random number from hardware
		std::mt19937 seed(rd()); // seed the generator
		std::uniform_int_distribution<> range(-worldDimensions.x, worldDimensions.x); // define the range

		const Elite::Vector2 randomLocation(range(seed), range(seed));
		const Elite::Vector2 target = pInterface->NavMesh_GetClosestPathPoint(randomLocation);

		if (randomLocation != target)
		{
			pBlackboard->ChangeData("Target", randomLocation);

			std::cout << "Data send " << randomLocation << "\n";

			return BT::State::Success;
		}

		return BT::State::Failure;
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
}