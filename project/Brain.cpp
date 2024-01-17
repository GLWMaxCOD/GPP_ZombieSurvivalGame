#include "stdafx.h"
#include "Brain.h"
#include <algorithm>

std::vector<Brain::InventoryMemory>::iterator Brain::FindLeastValueItem(const eItemType& itemType)
{
	const auto checkItemType = [itemType](const InventoryMemory& memory)->bool { return memory.ItemInfo.Type == itemType; };

	const auto numItem =
		std::ranges::count_if(m_InventoryMemory, checkItemType);

	if (numItem <= 1)
		return std::ranges::find_if(m_InventoryMemory, checkItemType);

	std::ranges::partition(m_InventoryMemory, checkItemType);

	const auto minItem =
		std::ranges::min_element(m_InventoryMemory,
			[itemType](const InventoryMemory& lhs, const InventoryMemory& rhs)->bool
			{
				if (lhs.ItemInfo.Type == itemType && rhs.ItemInfo.Type == itemType)
				{
					return lhs.ItemInfo.Value < rhs.ItemInfo.Value;
				}
				return false;
			});

	return minItem;
}

bool Brain::IsInvNotFull() const
{
	return m_InventoryMemory.capacity() < m_MaxInventorySlots - 1;
}

bool Brain::IsItemInInv(const eItemType& itemType)
{
	return std::ranges::any_of(m_InventoryMemory,
		[itemType](const InventoryMemory& memory)->bool
			{ return memory.ItemInfo.Type == itemType && memory.ItemInfo.Value > 0; });
}

bool Brain::EmptyValue()
{
	return std::ranges::any_of(m_InventoryMemory,
		[](const InventoryMemory& memory)->bool
		{ return memory.ItemInfo.Value <= 0; });
}

int Brain::FindEmptyValue(const ItemInfo& item)
{
	const auto FirstItem =
		std::ranges::find_if(m_InventoryMemory,
			[](const InventoryMemory& memory)->bool
				{ return memory.ItemInfo.Value <= 0; });

	FirstItem->ItemInfo = item;

	return FirstItem->invIndex;
}

int Brain::AddItemToMemory(const ItemInfo& item)
{
	if (m_InventoryMemory.capacity() > m_MaxInventorySlots)
		return m_MaxInventorySlots - 1;

	const InventoryMemory itemToMemory{ item, m_InventoryMemory.capacity() };

	m_InventoryMemory.push_back(itemToMemory);

	return m_InventoryMemory.capacity();
}

int Brain::CheckItem(const ItemInfo& item)
{
	if (std::ranges::any_of(m_InventoryMemory,
		[item](const InventoryMemory& memory)->bool
			{ return memory.ItemInfo.Type == item.Type; }))
	{
		const auto minItem{ FindLeastValueItem(item.Type) };
		if (minItem->ItemInfo.Value <= item.Value)
		{
			minItem->ItemInfo = item;
			return minItem->invIndex;
		}
		else
		{
			return m_MaxInventorySlots - 1;
		}
	}
	else
	{
		std::ranges::sort(m_InventoryMemory,
			[](const InventoryMemory& lhs, const InventoryMemory& rhs)->bool
			{ return lhs.ItemInfo.Type < rhs.ItemInfo.Type; });
		const auto duplicate =
			std::ranges::adjacent_find(m_InventoryMemory,
				[](const InventoryMemory& lhs, const InventoryMemory& rhs)->bool
				{ return lhs.ItemInfo.Type == rhs.ItemInfo.Type; });

		const auto minItem =
			std::min_element(duplicate, duplicate + 1,
				[](const InventoryMemory& lhs, const InventoryMemory& rhs)->bool
				{ return lhs.ItemInfo.Value < rhs.ItemInfo.Value; });

		minItem->ItemInfo = item;
		return minItem->invIndex;
	}
}

bool Brain::CheckIfTargetIsInside(const HouseInfo& targetHouse, Elite::Vector2 playerPos)
{
	const Elite::Vector2 houseCenter{ targetHouse.Center };
	const Elite::Vector2 houseSize{ targetHouse.Size };

	const float minX{ houseCenter.x - houseSize.x / 2 };
	const float minY{ houseCenter.y - houseSize.y / 2 };

	const float maxX{ houseCenter.x + houseSize.x / 2 };
	const float maxY{ houseCenter.y + houseSize.y / 2 };

	return (minX < playerPos.x&& minY < playerPos.y&&
			maxX > playerPos.x&& maxY > playerPos.y);
}

bool Brain::CheckIfTargetIsExplored(Elite::Vector2 target, float offset) const
{
	return std::ranges::any_of(m_HousesMemory,
		[target, offset](const HouseMemory& house)->bool
		{
			const Elite::Vector2 houseCenter{ house.houseInfo.Center };
			const Elite::Vector2 houseSize{ house.houseInfo.Size };

			const float minX{ houseCenter.x - houseSize.x / 2 - offset };
			const float minY{ houseCenter.y - houseSize.y / 2 - offset };

			const float maxX{ houseCenter.x + houseSize.x / 2 + offset };
			const float maxY{ houseCenter.y + houseSize.y / 2 + offset };

			return (minX < target.x&& minY < target.y&&
					maxX > target.x&& maxY > target.y);
		});
}

bool Brain::NewHouseToExplore()
{
	if (m_HousesMemory.capacity() != 0)
	{
		if (std::ranges::any_of(m_HousesMemory,
			[](const HouseMemory& houseMemory)->bool
			{ return houseMemory.newHouse == true; }))
		{
			return true;
		}
	}
	return false;
}

bool Brain::HouseToReExplore()
{
	if (m_HousesMemory.capacity() != 0)
	{
		const std::chrono::steady_clock::time_point currentTime{ std::chrono::steady_clock::now() };

		if (std::ranges::any_of(m_HousesMemory,
			[=](const HouseMemory& houseMemory)->bool
			{
				const std::chrono::duration<float> elapsedSec{ currentTime - houseMemory.waitTimer };
				return elapsedSec.count() >= m_MaxWaitTimer;
			}))
		{
			return true;
		}
	}
	return false;
}

void Brain::SetTargetHouseExpireDate(const HouseInfo& targetHouse)
{
	// Ensuring there is capacity in the memory
	assert(m_HousesMemory.capacity() != 0);

	// Find the target house in memory
	const auto foundHouse = FindHouseInMemory(targetHouse);

	// Update the exploration timer and mark the house as not new
	foundHouse->waitTimer = std::chrono::steady_clock::now();
	foundHouse->newHouse = false;
}

HouseInfo Brain::CheckHouseValidTarget(Elite::Vector2 playerPos, float maxRadius) const
{
	// Initialize target house and distance
	HouseInfo targetHouse{};
	float closestHouse{ FLT_MAX };

	// Iterate through houses in memory
	for (auto house : m_HousesMemory)
	{
		// Calculate the squared distance between player and house center
		const float houseDistance{ house.houseInfo.Center.DistanceSquared(playerPos) };

		// Skip houses outside the specified radius
		if (houseDistance > maxRadius * maxRadius)
			continue;

		// Skip houses that are not new and are farther than the current target
		if (closestHouse < houseDistance)
			continue;
		
		// Update target house if current house is closer or not new but can be re-explored
		if (house.newHouse == true)
		{
			targetHouse = house.houseInfo;
			closestHouse = houseDistance;
		}
		else
		{
			const std::chrono::steady_clock::time_point currentTime{ std::chrono::steady_clock::now() };
			const std::chrono::duration<float> elapsedSec{ currentTime - house.waitTimer };

			if (elapsedSec.count() < m_MaxWaitTimer)
				continue;

			targetHouse = house.houseInfo;
			closestHouse = houseDistance;
		}
	}
	return targetHouse;
}

bool Brain::CheckHousesForMemory(const std::vector<HouseInfo>& FOVHouses)
{
	// Initialize result variable
	bool result{};

	// Iterate through houses in the field of view
	for (auto& newHouse : FOVHouses)
	{
		// Check if there are houses in memory
		if (m_HousesMemory.capacity() != 0)
		{
			// Skip houses already in memory
			if (std::ranges::any_of(m_HousesMemory,
				[newHouse](const HouseMemory& houseMemory)->bool
				{ return houseMemory.houseInfo == newHouse; }))
			{
				continue;
			}
		}

		// Create a memory entry for the new house
		HouseMemory houseToMemory{};
		houseToMemory.houseInfo = newHouse;

		// Add the new house to memory
		m_HousesMemory.push_back(houseToMemory);

		// Update result to indicate that a new house was added
		result = true;
	}
	return result;
}

std::vector<Brain::HouseMemory>::iterator Brain::FindHouseInMemory(const HouseInfo& targetHouse)
{
	return
		std::ranges::find_if(m_HousesMemory,
			[targetHouse](const HouseMemory& houseMemory)->bool
			{ return houseMemory.houseInfo == targetHouse; });
}
