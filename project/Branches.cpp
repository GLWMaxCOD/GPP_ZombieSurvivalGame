#include "stdafx.h"
#include "Branches.h"

#include "Behaviour.h"
#include "BehaviourTree.h"

namespace Branch
{
	BT::PartialSequence* ItemHandling()
	{
		return
			new BT::PartialSequence({
				new BT::Conditional(BT_Conditions::SeeItem),
				new BT::Action(BT_Actions::SetItemAsTarget),
				new BT::Action(BT_Actions::DisableSpin),
				new BT::Action(BT_Actions::GoToDestination),
				new BT::Selector({
					new BT::Sequence({
						new BT::Conditional(std::bind(BT_Conditions::IsTypeOfItem, std::placeholders::_1, eItemType::GARBAGE)),
						new BT::Action(BT_Actions::DestroyItemOnFloor)
					}),
					new BT::Sequence({
						new BT::Conditional(BT_Conditions::InvIsNotFull),
						new BT::Action(BT_Actions::PickUpItem)
					})
				})
				});
	}

	BT::Selector* HouseHandling()
	{
		constexpr float maxTravelDistance{ 200.f };
		constexpr int searchRadius{ 100 };
		constexpr int exitingOffset{ 10 };

		return
			new BT::Selector({
				new BT::Sequence({
					new BT::Conditional(BT_Conditions::SeeHouse),
					new BT::Action(BT_Actions::CheckHouses)
				}),
				new BT::Selector({
					new BT::PartialSequence({
						new BT::Conditional(BT_Conditions::NewHouse),
						new BT::Action(std::bind(BT_Actions::GetHouseAsTarget, std::placeholders::_1, maxTravelDistance)),
						new BT::Action(BT_Actions::EnableSpin),
						new BT::Action(BT_Actions::GoToDestination),
						new BT::Action(BT_Actions::SetExpireDate),
						new BT::PartialSequence({
							new BT::Action(std::bind(BT_Actions::GetOutsideTarget, std::placeholders::_1, exitingOffset)),
							new BT::Action(BT_Actions::GoToDestination)
						})
					}),
					new BT::PartialSequence({
						new BT::Conditional(BT_Conditions::ReExploreHouse),
						new BT::Action(std::bind(BT_Actions::GetHouseAsTarget, std::placeholders::_1, maxTravelDistance)),
						new BT::Action(BT_Actions::EnableSpin),
						new BT::Action(BT_Actions::GoToDestination),
						new BT::Action(BT_Actions::SetExpireDate),
						new BT::PartialSequence({
							new BT::Action(std::bind(BT_Actions::GetOutsideTarget, std::placeholders::_1, exitingOffset)),
							new BT::Action(BT_Actions::GoToDestination)
						})
					})
				}),
				new BT::PartialSequence({
					new BT::Action(std::bind(BT_Actions::TryFindHouse, std::placeholders::_1, searchRadius)),
					new BT::Action(BT_Actions::EnableSpin),
					new BT::Action(BT_Actions::GoToDestination)
				})
				});
	}
}