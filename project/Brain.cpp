#include "stdafx.h"
#include "Brain.h"
#include <algorithm>

void Brain::CheckIfNewHouse(const HouseInfo& newHouse)
{
	if (std::any_of(std::begin(m_HousesMemory), std::end(m_HousesMemory), [&newHouse](const HouseMemory& houseMemory)->bool {return houseMemory.houseInfo == newHouse; }))
	{
		return;
	}

	HouseMemory houseToMemory{};
	houseToMemory.houseInfo = newHouse;

	m_HousesMemory.push_back(houseToMemory);
}