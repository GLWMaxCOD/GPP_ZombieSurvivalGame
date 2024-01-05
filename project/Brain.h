#pragma once
#include <Exam_HelperStructs.h>
#include <chrono>

class Brain final
{
public:
	Brain() = default;

	~Brain() = default;
	Brain(const Brain&) = default;
	Brain& operator=(const Brain&) = default;
	Brain(Brain&&) = default;
	Brain& operator=(Brain&&) = default;

	void CheckIfNewHouse(const HouseInfo& newHouse);

private:
	struct HouseMemory
	{
		bool newHouse{ true };
		HouseInfo houseInfo;
		std::chrono::steady_clock::time_point whenExplored;
	};

	std::vector<HouseMemory> m_HousesMemory{};
};