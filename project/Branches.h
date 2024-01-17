#pragma once

namespace BT
{
	class PartialSequence;
	class Sequence;
	class Selector;
}

namespace Branch
{
	BT::Selector* ItemHandling();
	BT::Sequence* ZombieHandling();
	BT::Sequence* PurgeZoneHandling();
	BT::PartialSequence* PickUpHandling();
	BT::Selector* HouseHandling();
}