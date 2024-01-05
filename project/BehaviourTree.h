/*=============================================================================*/
// Copyright 2021-2022 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// EBehaviourTree.h: Implementation of a BehaviourTree and the components of a Behaviour Tree
/*=============================================================================*/

#include <Exam_HelperStructs.h>
#include "Blackboard.h"

namespace BT
{

enum class State
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
	virtual State Execute(Blackboard* blackboardPtr) = 0;

protected:
	State m_CurrentState = State::Failure;
};

#pragma region COMPOSITES
class Composite : public IBehaviour
{
public:
	explicit Composite(const std::vector<IBehaviour*>& childBehaviours);
	~Composite() override;

	State Execute(Blackboard* blackboardPtr) override = 0;

protected:
	std::vector<IBehaviour*> m_ChildBehaviours = {};
};

class Selector final : public Composite
{
public:
	inline explicit Selector(const std::vector<IBehaviour*>& childBehaviours) :
		Composite(std::move(childBehaviours)) {}
	~Selector() override = default;

	State Execute(Blackboard* blackboardPtr) override;
};

class Sequence : public Composite
{
public:
	inline explicit Sequence(const std::vector<IBehaviour*>& childBehaviours) :
		Composite(std::move(childBehaviours)) {}
	~Sequence() override = default;

	State Execute(Blackboard* blackboardPtr) override;
};

class PartialSequence final : public Sequence
{
public:
	inline explicit PartialSequence(const std::vector<IBehaviour*>& childBehaviours)
		: Sequence(std::move(childBehaviours)) {}
	~PartialSequence() override = default;

	State Execute(Blackboard* blackboardPtr) override;

private:
	unsigned int m_CurrentBehaviourIndex = 0;
};
#pragma endregion

class Conditional final : public IBehaviour
{
public:
	explicit Conditional(std::function<bool(Blackboard*)> fp)
		: m_ConditionalPtr(std::move(fp)) {}

	State Execute(Blackboard* blackboardPtr) override;

private:
	std::function<bool(Blackboard*)> m_ConditionalPtr = nullptr;
};

class Action final : public IBehaviour
{
public:
	explicit Action(std::function<State(Blackboard*)> fp) : m_ActionPtr(std::move(fp)) {}
	State Execute(Blackboard* blackboardPtr) override;

private:
	std::function<State(Blackboard*)> m_ActionPtr = nullptr;
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
	State m_CurrentState = State::Failure;
	Blackboard* m_BlackboardPtr = nullptr;
	IBehaviour* m_RootBehaviourPtr = nullptr;
};
}