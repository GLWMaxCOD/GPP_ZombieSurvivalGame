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

	bool IsInvFull() const;
	int AddItemToMemory(const ItemInfo& itemInfo);

	bool NewHouseToExplore();
	bool HouseToReExplore();
	void SetTargetHouseExpireDate(const HouseInfo& targetHouse);
	HouseInfo CheckHouseValidTarget(Elite::Vector2 playerPos, float maxRadius) const;
	bool CheckHousesForMemory(const std::vector<HouseInfo>& FOVHouses);

private:
	struct HouseMemory
	{
		bool newHouse{ true };
		HouseInfo houseInfo;
		std::chrono::steady_clock::time_point unExploreTimer;
	};

	struct InvMemory
	{
		ItemInfo item;

	};

	std::vector<HouseMemory> m_HousesMemory{};
	const float m_MaxUnExplorable{ 60.f };

	std::vector<ItemInfo> m_InvMemory{};
	const size_t m_MaxInvSize{ 5 };
};