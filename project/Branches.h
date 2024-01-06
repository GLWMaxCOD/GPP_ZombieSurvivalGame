#pragma once

namespace BT
{
	class PartialSequence;
	class Selector;
}

namespace Branch
{
	BT::Selector* ItemHandling();
	BT::PartialSequence* PickUpHandling();
	BT::Selector* HouseHandling();
}