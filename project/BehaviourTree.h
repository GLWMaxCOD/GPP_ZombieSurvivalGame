/*=============================================================================*/
// Copyright 2021-2022 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// EBehaviourTree.h: Implementation of a BehaviourTree and the components of a Behaviour Tree
/*=============================================================================*/

#include "Blackboard.h"

namespace BT
{

	enum class BehaviourState
{
	Failure,
	Success,
	Running
};

class IBehaviour
{
public:
	IBehaviour() = default;
	virtual ~IBehaviour() = default;
	virtual BehaviourState Execute(Blackboard* blackboardPtr) = 0;

protected:
	BehaviourState m_CurrentState = BehaviourState::Failure;
};

#pragma region COMPOSITES
class Composite : public IBehaviour
{
public:
	explicit Composite(const std::vector<IBehaviour*>& childBehaviours);
	~Composite() override;

	BehaviourState Execute(Blackboard* blackboardPtr) override = 0;

protected:
	std::vector<IBehaviour*> m_ChildBehaviours = {};
};

class Selector final : public Composite
{
public:
	explicit Selector(std::vector<IBehaviour*> childBehaviours) :
		Composite(std::move(childBehaviours)) {}
	~Selector() override = default;

	BehaviourState Execute(Blackboard* blackboardPtr) override;
};

class Sequence : public Composite
{
public:
	inline explicit Sequence(std::vector<IBehaviour*> childBehaviours) :
		Composite(std::move(childBehaviours)) {}
	~Sequence() override = default;

	BehaviourState Execute(Blackboard* blackboardPtr) override;
};

class PartialSequence final : public Sequence
{
public:
	inline explicit PartialSequence(std::vector<IBehaviour*> childBehaviours)
		: Sequence(std::move(childBehaviours)) {}
	~PartialSequence() override = default;

	BehaviourState Execute(Blackboard* blackboardPtr) override;

private:
	unsigned int m_CurrentBehaviourIndex = 0;
};
#pragma endregion

class Conditional final : public IBehaviour
{
public:
	explicit Conditional(std::function<bool(Blackboard*)> fp)
		: m_ConditionalPtr(std::move(fp)) {}

	BehaviourState Execute(Blackboard* blackboardPtr) override;

private:
	std::function<bool(Blackboard*)> m_ConditionalPtr = nullptr;
};

class Action final : public IBehaviour
{
public:
	explicit Action(std::function<BehaviourState(Blackboard*)> fp) : m_ActionPtr(std::move(fp)) {}
	BehaviourState Execute(Blackboard* blackboardPtr) override;

private:
	std::function<BehaviourState(Blackboard*)> m_ActionPtr = nullptr;
};

class BehaviourTree final
{
public:
	inline explicit BehaviourTree(Blackboard* blackboardPtr, IBehaviour* pRootBehaviour)
		: m_BlackboardPtr(blackboardPtr),
		m_RootBehaviourPtr(pRootBehaviour) {}
	~BehaviourTree();

	void Update();

	Blackboard* GetBlackboard() const;

private:
	BehaviourState m_CurrentState = BehaviourState::Failure;
	Blackboard* m_BlackboardPtr = nullptr;
	IBehaviour* m_RootBehaviourPtr = nullptr;
};
}