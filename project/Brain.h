#pragma once
#include <Exam_HelperStructs.h>
#include <chrono>

class Brain final
{
private:
	struct HouseMemory
	{
		bool newHouse{ true };
		HouseInfo houseInfo;
		std::chrono::steady_clock::time_point waitTimer;
	};

	struct InventoryMemory
	{
		ItemInfo ItemInfo;
		int invIndex;
	};

public:
	Brain() = default;

	~Brain() = default;
	Brain(const Brain&) = default;
	Brain& operator=(const Brain&) = default;
	Brain(Brain&&) = default;
	Brain& operator=(Brain&&) = default;

	std::vector<InventoryMemory>::iterator FindLeastValueItem(const eItemType& itemType);
	bool IsInvNotFull() const;
	bool IsItemInInv(const eItemType& itemType);
	bool EmptyValue();
	int FindEmptyValue(const ItemInfo& item);
	int AddItemToMemory(const ItemInfo& item);
	int CheckItem(const ItemInfo& item);

	bool CheckIfTargetIsInside(const HouseInfo& targetHouse, Elite::Vector2 playerPos);
	bool CheckIfTargetIsExplored(Elite::Vector2 target, float offset) const;
	bool NewHouseToExplore();
	bool HouseToReExplore();
	void SetTargetHouseExpireDate(const HouseInfo& targetHouse);
	HouseInfo CheckHouseValidTarget(Elite::Vector2 playerPos, float maxRadius) const;
	bool CheckHousesForMemory(const std::vector<HouseInfo>& FOVHouses);

private:
	std::vector<HouseMemory> m_HousesMemory{};
	const float m_MaxWaitTimer{ 360.f };

	std::vector<InventoryMemory> m_InventoryMemory{};
	const size_t m_MaxInventorySlots{ 5 };

	std::vector<HouseMemory>::iterator FindHouseInMemory(const HouseInfo& targetHouse);
};