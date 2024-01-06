#include "stdafx.h"
#include "Brain.h"
#include <algorithm>

bool Brain::IsInvFull() const
{
	bool a = m_InventoryMemory.capacity() > m_MaxInvSize;
	std::cout << m_InventoryMemory.capacity() << " > " << m_MaxInvSize << " = " << a << std::endl;
	return m_InventoryMemory.capacity() > m_MaxInvSize;
}

int Brain::AddItemToMemory(const ItemInfo& newItem)
{
	assert(m_InventoryMemory.capacity() < m_MaxInvSize);

	const InventoryMemory itemToMemory{ newItem, static_cast<int>(m_InventoryMemory.capacity()) };

	m_InventoryMemory.push_back(itemToMemory);

	return static_cast<int>(m_InventoryMemory.capacity());
}

int Brain::CheckAmountOfType(eItemType type)
{
	return std::count_if(std::begin(m_InventoryMemory), std::end(m_InventoryMemory),
		[type](const InventoryMemory& memory)->bool { return memory.ItemInfo.Type == type; });
}

//std::vector<Brain::InventoryMemory>::iterator Brain::CheckValueOfItem(const ItemInfo& item)
//{
//	return std::min_element(std::begin(m_InventoryMemory), std::end(m_InventoryMemory),
//		[](const InventoryMemory& lhs, const InventoryMemory& rhs)
//		{
//			return lhs.ItemInfo.Value < rhs.ItemInfo.Value;
//		});
//}

int Brain::CheckItem(const ItemInfo& newItem, int maxItems)
{
	const int pistolCount = CheckAmountOfType(eItemType::PISTOL);
	const int shotgun = CheckAmountOfType(eItemType::SHOTGUN);
	const int medkitCount = CheckAmountOfType(eItemType::MEDKIT);
	const int foodCount = CheckAmountOfType(eItemType::FOOD);

	switch (newItem.Type) {
		case eItemType::PISTOL:
		{
			if (pistolCount > 0)
			{

			}

			break;
		}
		case eItemType::SHOTGUN:
		{
			break;
		}
		case eItemType::MEDKIT:
		{
			break;
		}
		case eItemType::FOOD:
		{
			break;
		}
		default:
		{
			break;
		}
	}

	if (pistolCount >= 1)
	{

	}
	else if (shotgun >= 1)
	{

	}
	else if (medkitCount >= 1)
	{

	}
	else if (foodCount >= 2)
	{

	}

	return INT_MAX;
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
	return std::any_of(std::begin(m_HousesMemory), std::end(m_HousesMemory),
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
			const std::chrono::duration<float> elapsedSec{ currentTime - house.waitTimer };

			if (elapsedSec.count() < m_MaxWaitTimer)
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

std::vector<Brain::HouseMemory>::iterator Brain::FindHouseInMemory(const HouseInfo& targetHouse)
{
	return
		std::find_if(std::begin(m_HousesMemory), std::end(m_HousesMemory),
			[targetHouse](const HouseMemory& houseMemory)->bool
			{ return houseMemory.houseInfo == targetHouse; });
}
