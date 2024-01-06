#include "stdafx.h"
#include "Branches.h"

#include "Behaviour.h"
#include "BehaviourTree.h"

namespace Branch
{
	BT::PartialSequence* PickUpHandling()
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
						new BT::Conditional(BT_Conditions::InvIsFull),
						new BT::Selector({
							new BT::Sequence({
								new BT::Conditional(std::bind(BT_Conditions::IsTypeOfItem, std::placeholders::_1, eItemType::FOOD)),
								new BT::Action(BT_Actions::CheckItem)
							}),
							new BT::Sequence({
								new BT::Conditional(std::bind(BT_Conditions::IsTypeOfItem, std::placeholders::_1, eItemType::MEDKIT)),
								new BT::Action(BT_Actions::CheckItem)
							}),
							new BT::Sequence({
								new BT::Conditional(std::bind(BT_Conditions::IsTypeOfItem, std::placeholders::_1, eItemType::SHOTGUN)),
								new BT::Action(BT_Actions::CheckItem)
							}),
							new BT::Sequence({
								new BT::Conditional(std::bind(BT_Conditions::IsTypeOfItem, std::placeholders::_1, eItemType::PISTOL)),
								new BT::Action(BT_Actions::CheckItem)
							}),
						})
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
		constexpr float maxTravelDistance{ 100.f };
		constexpr int searchRadius{ 300 };
		constexpr int searchDegree{ 45 }; //TODO
		constexpr float InsideOffset{ 5.f };

		const std::string BeforeLeavingTimer{ "BeforeLeaving" };
		constexpr bool BeforeLeavingDoOnce{ true };

		return
			new BT::Selector({
				new BT::PartialSequence({
					new BT::Conditional(BT_Conditions::InsideTargetHouse),
					new BT::Action(BT_Actions::SetExpireDate),
					new BT::Action(std::bind(BT_Actions::SetTimer, std::placeholders::_1, BeforeLeavingTimer, BeforeLeavingDoOnce)),
					new BT::Selector({
						new BT::PartialSequence({
							new BT::Conditional(std::bind(BT_Conditions::CheckTimer, std::placeholders::_1, BeforeLeavingTimer, BeforeLeavingDoOnce)),
							new BT::Selector({
								new BT::PartialSequence({
									new BT::Conditional(BT_Conditions::NewHouse),
									new BT::Action(std::bind(BT_Actions::GetHouseAsTarget, std::placeholders::_1, maxTravelDistance)),
									new BT::Action(BT_Actions::EnableSpin),
									new BT::Action(BT_Actions::GoToDestination),
								}),
								new BT::PartialSequence({
									new BT::Conditional(BT_Conditions::ReExploreHouse),
									new BT::Action(std::bind(BT_Actions::GetHouseAsTarget, std::placeholders::_1, maxTravelDistance)),
									new BT::Action(BT_Actions::EnableSpin),
									new BT::Action(BT_Actions::GoToDestination),
								}),
								new BT::PartialSequence({
									new BT::Action(std::bind(BT_Actions::TryFindHouse, std::placeholders::_1, searchRadius, searchDegree)),
									new BT::Action(BT_Actions::EnableSpin),
									new BT::Action(BT_Actions::GoToDestination)
								})
							}),
						}),
						new BT::PartialSequence({
							new BT::Action(std::bind(BT_Actions::GetInsideTarget, std::placeholders::_1, InsideOffset)),
							new BT::Action(BT_Actions::EnableSpin),
							new BT::Action(BT_Actions::GoToDestination)
						}),
					})
				}),
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
					}),
					new BT::PartialSequence({
						new BT::Conditional(BT_Conditions::ReExploreHouse),
						new BT::Action(std::bind(BT_Actions::GetHouseAsTarget, std::placeholders::_1, maxTravelDistance)),
						new BT::Action(BT_Actions::EnableSpin),
						new BT::Action(BT_Actions::GoToDestination),
					})
				}),
				new BT::PartialSequence({
					new BT::Action(std::bind(BT_Actions::TryFindHouse, std::placeholders::_1, searchRadius, searchDegree)),
					new BT::Action(BT_Actions::EnableSpin),
					new BT::Action(BT_Actions::GoToDestination)
				})
				});
	}
}