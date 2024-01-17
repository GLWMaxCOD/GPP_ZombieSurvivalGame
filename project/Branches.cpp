#include "stdafx.h"
#include "Branches.h"

#include "Behaviour.h"
#include "BehaviourTree.h"

using namespace std::placeholders;

namespace Branch
{
	BT::Sequence* PurgeZoneHandling()
	{
		constexpr int searchDegree{ 5 };
		return
			new BT::Sequence({
					new BT::Conditional(BT_Conditions::SeePurgeZone),
					new BT::Action(std::bind(BT_Actions::FindClosestEdge, _1, searchDegree)),
					new BT::Action(BT_Actions::GoToDestination)
			});
	}

	BT::Sequence* ZombieHandling()
	{
		constexpr float minShotgunAngleDiff{ .2f };
		constexpr float minPistolAngleDiff{ .1f };
		constexpr float maxShootRange{ 15.f };

		const std::string ShotgunTimer{ "Shotgun" };
		constexpr bool ShotgunDoOnce{ true };

		const std::string PistolTimer{ "Pistol" };
		constexpr bool PistolDoOnce{ true };

		return
			new BT::Sequence({
				new BT::Conditional(BT_Conditions::SeeZombie),
				new BT::Sequence({
					new BT::Action(BT_Actions::AvoidingZombie),
					new BT::Conditional(BT_Conditions::HasWeapon),
					new BT::Action(BT_Actions::SetZombieTarget),
					new BT::Sequence({
						new BT::Conditional(std::bind(BT_Conditions::InRange, _1, maxShootRange)),
						new BT::Action(BT_Actions::RotateToZombie),
						new BT::Selector({
							new BT::Sequence({
								new BT::Conditional(std::bind(BT_Conditions::ItemInInv, _1, eItemType::SHOTGUN)),
								new BT::Action(std::bind(BT_Actions::ReadyToShoot, _1, minShotgunAngleDiff)),
								new BT::Conditional(std::bind(BT_Conditions::CheckTimer, _1, ShotgunTimer, ShotgunDoOnce)),
								new BT::Action(std::bind(BT_Actions::Shoot, _1, eItemType::SHOTGUN)),
								new BT::Action(std::bind(BT_Actions::SetTimer, _1, ShotgunTimer, ShotgunDoOnce))
							}),
							new BT::Sequence({
								new BT::Action(std::bind(BT_Actions::ReadyToShoot, _1, minPistolAngleDiff)),
								new BT::Conditional(std::bind(BT_Conditions::CheckTimer, _1, PistolTimer, PistolDoOnce)),
								new BT::Action(std::bind(BT_Actions::Shoot, _1, eItemType::PISTOL)),
								new BT::Action(std::bind(BT_Actions::SetTimer, _1, PistolTimer, PistolDoOnce))
							})
						})
					})
				})
			});
	}

	BT::Selector* ItemHandling()
	{
		constexpr float HpThreshold{ 0.f };

		return
			new BT::Selector({
				new BT::Sequence({
					new BT::Conditional(std::bind(BT_Conditions::ItemInInv, _1, eItemType::MEDKIT)),
					new BT::Conditional(std::bind(BT_Conditions::HpUnderThreshold, _1, HpThreshold)),
					new BT::Action(std::bind(BT_Actions::UseItem, _1, eItemType::MEDKIT))
				}),
				new BT::Sequence({
					new BT::Conditional(std::bind(BT_Conditions::ItemInInv, _1, eItemType::FOOD)),
					new BT::Conditional(BT_Conditions::CheckMinNeededEnergy),
					new BT::Action(std::bind(BT_Actions::UseItem, _1, eItemType::FOOD))
				})
			});
	}

	BT::PartialSequence* PickUpHandling()
	{
		return
			new BT::PartialSequence({
				new BT::Conditional(BT_Conditions::SeeItem),
				new BT::Action(BT_Actions::DisableSpin),
				new BT::Action(BT_Actions::SetItemAsTarget),
				new BT::Action(BT_Actions::GoToDestination),
				new BT::Selector({
					new BT::Sequence({
						new BT::Conditional(std::bind(BT_Conditions::IsTypeOfItem, _1, eItemType::GARBAGE)),
						new BT::Action(BT_Actions::DestroyItemOnFloor)
					}),
					new BT::Sequence({
						new BT::Conditional(BT_Conditions::EmptyValue),
						new BT::Action(BT_Actions::SwapItem)
					}),
					new BT::Sequence({
						new BT::Conditional(BT_Conditions::InvIsNotFull),
						new BT::Action(BT_Actions::PickUpItem)
					}),
					new BT::Sequence({
						new BT::Conditional(BT_Conditions::InvIsFull),
						new BT::Selector({
							new BT::Sequence({
								new BT::Conditional(std::bind(BT_Conditions::IsTypeOfItem,_1, eItemType::FOOD)),
								new BT::Action(BT_Actions::CheckItem)
							}),
							new BT::Sequence({
								new BT::Conditional(std::bind(BT_Conditions::IsTypeOfItem, _1, eItemType::MEDKIT)),
								new BT::Action(BT_Actions::CheckItem)
							}),
							new BT::Sequence({
								new BT::Conditional(std::bind(BT_Conditions::IsTypeOfItem, _1, eItemType::SHOTGUN)),
								new BT::Action(BT_Actions::CheckItem)
							}),
							new BT::Sequence({
								new BT::Conditional(std::bind(BT_Conditions::IsTypeOfItem, _1, eItemType::PISTOL)),
								new BT::Action(BT_Actions::CheckItem)
							})
						})
					})
				})
			});
	}

	BT::Selector* HouseHandling()
	{
		constexpr float maxTravelDistance{ 250.f };
		constexpr int searchRadius{ 300 };
		constexpr int searchDegree{ 5 };
		constexpr float InsideOffset{ 5.f };

		const std::string BeforeLeavingTimer{ "BeforeLeaving" };
		constexpr bool BeforeLeavingDoOnce{ true };

		return
			new BT::Selector({
				new BT::PartialSequence({
					new BT::Conditional(BT_Conditions::InsideTargetHouse),
					new BT::Action(BT_Actions::SetExpireDate),
					new BT::Action(std::bind(BT_Actions::SetTimer, _1, BeforeLeavingTimer, BeforeLeavingDoOnce)),
					new BT::Selector({
						new BT::PartialSequence({
							new BT::Conditional(std::bind(BT_Conditions::CheckTimer, _1, BeforeLeavingTimer, BeforeLeavingDoOnce)),
							new BT::Selector({
								new BT::PartialSequence({
									new BT::Conditional(BT_Conditions::NewHouse),
									new BT::Action(std::bind(BT_Actions::GetHouseAsTarget, _1, maxTravelDistance)),
									new BT::Action(BT_Actions::EnableSpin),
									new BT::Action(BT_Actions::GoToDestination),
								}),
								new BT::PartialSequence({
									new BT::Conditional(BT_Conditions::ReExploreHouse),
									new BT::Action(std::bind(BT_Actions::GetHouseAsTarget, _1, maxTravelDistance)),
									new BT::Action(BT_Actions::EnableSpin),
									new BT::Action(BT_Actions::GoToDestination),
								}),
								new BT::PartialSequence({
									new BT::Action(std::bind(BT_Actions::TryFindHouse, _1, searchRadius, searchDegree)),
									new BT::Action(BT_Actions::EnableSpin),
									new BT::Action(BT_Actions::GoToDestination)
								})
							}),
						}),
						new BT::PartialSequence({
							new BT::Action(std::bind(BT_Actions::GetInsideTarget, _1, InsideOffset)),
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
						new BT::Action(std::bind(BT_Actions::GetHouseAsTarget, _1, maxTravelDistance)),
						new BT::Action(BT_Actions::EnableSpin),
						new BT::Action(BT_Actions::GoToDestination),
					}),
					new BT::PartialSequence({
						new BT::Conditional(BT_Conditions::ReExploreHouse),
						new BT::Action(std::bind(BT_Actions::GetHouseAsTarget, _1, maxTravelDistance)),
						new BT::Action(BT_Actions::EnableSpin),
						new BT::Action(BT_Actions::GoToDestination),
					})
				}),
				new BT::PartialSequence({
					new BT::Action(std::bind(BT_Actions::TryFindHouse, _1, searchRadius, searchDegree)),
					new BT::Action(BT_Actions::EnableSpin),
					new BT::Action(BT_Actions::GoToDestination)
				})
			});
	}
}