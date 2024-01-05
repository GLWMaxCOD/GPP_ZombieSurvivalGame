#include "stdafx.h"
#include "Brain.h"
#include <algorithm>

bool Brain::IsInvFull() const
{
	return m_InvMemory.capacity() == m_MaxInvSize;
}

int Brain::AddItemToMemory(const ItemInfo& itemInfo)
{
	assert(m_InvMemory.capacity() < m_MaxInvSize);

	m_InvMemory.push_back(itemInfo);

	return static_cast<int>(m_InvMemory.capacity());
}

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

bool Brain::HouseToReExplore()
{
	if (m_HousesMemory.capacity() != 0)
	{
		// Get the current time
		const std::chrono::steady_clock::time_point currentTime{ std::chrono::steady_clock::now() };

		// Check if there are houses with elapsed exploration time exceeding the threshold
		if (std::any_of(std::begin(m_HousesMemory), std::end(m_HousesMemory),
			[=](const HouseMemory& houseMemory)->bool
			{
				const std::chrono::duration<float> elapsedSec{ currentTime - houseMemory.unExploreTimer };
				return elapsedSec.count() >= m_MaxUnExplorable;
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
	const auto foundHouse =
		std::find_if(std::begin(m_HousesMemory), std::end(m_HousesMemory),
			[targetHouse](const HouseMemory& houseMemory)->bool
			{ return houseMemory.houseInfo.Center == targetHouse.Center; });

	// Update the exploration timer and mark the house as not new
	foundHouse->unExploreTimer = std::chrono::steady_clock::now();
	foundHouse->newHouse = false;
}

HouseInfo Brain::CheckHouseValidTarget(Elite::Vector2 playerPos, float maxRadius) const
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

		// Skip houses that are not new and are farther than the current target
		if (targetDistance > houseDistance)
			continue;
		
		// Update target house if current house is closer or not new but can be re-explored
		if (house.newHouse == true)
		{
			targetHouse = house.houseInfo;
			targetDistance = houseDistance;
		}
		else
		{
			const std::chrono::steady_clock::time_point currentTime{ std::chrono::steady_clock::now() };
			const std::chrono::duration<float> elapsedSec{ currentTime - house.unExploreTimer };

			if (elapsedSec.count() < m_MaxUnExplorable)
				continue;

			targetHouse = house.houseInfo;
			targetDistance = houseDistance;
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
			if (std::any_of(std::begin(m_HousesMemory), std::end(m_HousesMemory),
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
