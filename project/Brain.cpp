#include "stdafx.h"
#include "Brain.h"
#include <algorithm>

bool Brain::NewHouseToExplore()
{
	// Check if there are houses in memory
	if (m_HousesMemory.capacity() != 0)
	{
		// Check if there is any new house in the memory
		if (std::any_of(std::begin(m_HousesMemory), std::end(m_HousesMemory),
			[](const HouseMemory& houseMemory)->bool
			{ return houseMemory.newHouse == true; }))
		{
			return true;
		}
	}
	return false;
}

HouseInfo Brain::CheckHouseTarget(Elite::Vector2 playerPos, float maxRadius) const
{
	// Initialize target house and distance
	HouseInfo targetHouse{};
	float targetDistance{};

	// Iterate through houses in memory
	for (auto house : m_HousesMemory)
	{
		// Calculate the squared distance between player and house center
		const float houseDistance{ house.houseInfo.Center.DistanceSquared(playerPos) };

		// Skip houses outside the specified radius
		if (houseDistance > maxRadius * maxRadius)
			continue;

		// Skip houses that are not new
		if (house.newHouse == false)
			continue;
		
		// Update target house if current house is closer
		if (targetDistance < houseDistance)
		{
			targetHouse = house.houseInfo;
			targetDistance = houseDistance;
		}
	}
	return targetHouse;
}

bool Brain::CheckHouses(const std::vector<HouseInfo>& FOVHouses)
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
			if (std::any_of(std::begin(m_HousesMemory), std::end(m_HousesMemory),
				[&newHouse](const HouseMemory& houseMemory)->bool
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

	// Display the houses in memory
	int count{};
	for (auto& house : m_HousesMemory)
	{
		std::cout << "House " << count << ": " << house.houseInfo.Center << std::endl;
		count++;
	}

	return result;
}
