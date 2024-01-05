/*=============================================================================*/
// Copyright 2021-2022 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// EBehaviorTree.h: Implementation of a BehaviorTree and the components of a Behavior Tree
/*=============================================================================*/

#include "Blackboard.h"

namespace BT
{

	enum class BehaviorState
{
	Failure,
	Success,
	Running
};

class IBehavior
{
public:
	IBehavior() = default;
	virtual ~IBehavior() = default;
	virtual BehaviorState Execute(Blackboard* blackboardPtr) = 0;

protected:
	BehaviorState m_CurrentState = BehaviorState::Failure;
};

#pragma region COMPOSITES
class Composite : public IBehavior
{
public:
	explicit Composite(const std::vector<IBehavior*>& childBehaviors);
	~Composite() override;

	BehaviorState Execute(Blackboard* blackboardPtr) override = 0;

protected:
	std::vector<IBehavior*> m_ChildBehaviors = {};
};

class Selector final : public Composite
{
public:
	explicit Selector(std::vector<IBehavior*> childBehaviors) :
		Composite(std::move(childBehaviors)) {}
	~Selector() override = default;

	BehaviorState Execute(Blackboard* blackboardPtr) override;
};

class Sequence : public Composite
{
public:
	inline explicit Sequence(std::vector<IBehavior*> childBehaviors) :
		Composite(std::move(childBehaviors)) {}
	~Sequence() override = default;

	BehaviorState Execute(Blackboard* blackboardPtr) override;
};

class PartialSequence final : public Sequence
{
public:
	inline explicit PartialSequence(std::vector<IBehavior*> childBehaviors)
		: Sequence(std::move(childBehaviors)) {}
	~PartialSequence() override = default;

	BehaviorState Execute(Blackboard* blackboardPtr) override;

private:
	unsigned int m_CurrentBehaviorIndex = 0;
};
#pragma endregion

class Conditional final : public IBehavior
{
public:
	explicit Conditional(std::function<bool(Blackboard*)> fp)
		: m_ConditionalPtr(std::move(fp)) {}

	BehaviorState Execute(Blackboard* blackboardPtr) override;

private:
	std::function<bool(Blackboard*)> m_ConditionalPtr = nullptr;
};

class Action final : public IBehavior
{
public:
	explicit Action(std::function<BehaviorState(Blackboard*)> fp) : m_ActionPtr(std::move(fp)) {}
	BehaviorState Execute(Blackboard* blackboardPtr) override;

private:
	std::function<BehaviorState(Blackboard*)> m_ActionPtr = nullptr;
};

class BehaviorTree final
{
public:
	inline explicit BehaviorTree(Blackboard* blackboardPtr, IBehavior* pRootBehavior)
		: m_BlackboardPtr(blackboardPtr),
		m_RootBehaviorPtr(pRootBehavior) {}
	~BehaviorTree();

	void Update();

	Blackboard* GetBlackboard() const;

private:
	BehaviorState m_CurrentState = BehaviorState::Failure;
	Blackboard* m_BlackboardPtr = nullptr;
	IBehavior* m_RootBehaviorPtr = nullptr;
};
}