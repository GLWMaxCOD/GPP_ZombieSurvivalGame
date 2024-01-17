#pragma once

namespace BT
{
	class PartialSequence;
	class Sequence;
	class Selector;
}

namespace Branch
{
	BT::Sequence* PurgeZoneHandling();
	BT::Selector* ItemHandling();
	BT::PartialSequence* PickUpHandling();
	BT::Selector* HouseHandling();
}