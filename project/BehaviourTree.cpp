#include "stdafx.h"
#include "BehaviourTree.h"

using namespace BT;

#pragma region COMPOSITES
Composite::Composite(const std::vector<IBehavior*>& childBehaviors)
{
	m_ChildBehaviors = childBehaviors;
}

Composite::~Composite()
{
	for (auto pb : m_ChildBehaviors)
		SAFE_DELETE(pb)
		m_ChildBehaviors.clear();
}

//SELECTOR
BehaviorState Selector::Execute(Blackboard* blackboardPtr)
{
	// Loop over all children in m_ChildBehaviors
	for (IBehavior* pBeh : m_ChildBehaviors)
	{
		//Every Child: Execute and store the result in m_CurrentState
		m_CurrentState = pBeh->Execute(blackboardPtr);

		//Check the currentstate and apply the selector Logic:
		//if a child returns Success:
		if (m_CurrentState == BehaviorState::Success)
		{
			//stop looping over all children and return Success
			return m_CurrentState;
		}

		//if a child returns Running:
		if (m_CurrentState == BehaviorState::Running)
		{
			//Running: stop looping and return Running
			return m_CurrentState;
		}
		//The selector fails if all children failed.	
	}

	//All children failed
	m_CurrentState = BehaviorState::Failure;
	return m_CurrentState;
}
//SEQUENCE
BehaviorState Sequence::Execute(Blackboard* blackboardPtr)
{
	//Loop over all children in m_ChildBehaviors
	for (IBehavior* pBeh : m_ChildBehaviors)
	{
		//Every Child: Execute and store the result in m_CurrentState
		m_CurrentState = pBeh->Execute(blackboardPtr);

		//Check the currentstate and apply the sequence Logic:
		//if a child returns Failed:
		if (m_CurrentState == BehaviorState::Failure)
		{
			//stop looping over all children and return Failed
			return m_CurrentState;
		}

		//if a child returns Running:
		if (m_CurrentState == BehaviorState::Running)
		{
			//Running: stop looping and return Running
			return m_CurrentState;

		}

		//The selector succeeds if all children succeeded.
	}

	//All children succeeded 
	m_CurrentState = BehaviorState::Success;
	return m_CurrentState;
}

BehaviorState PartialSequence::Execute(Blackboard* blackboardPtr)
{
	while (m_CurrentBehaviorIndex < m_ChildBehaviors.size())
	{
		m_CurrentState = m_ChildBehaviors[m_CurrentBehaviorIndex]->Execute(blackboardPtr);
		switch (m_CurrentState)
		{
		case BehaviorState::Failure:
			m_CurrentBehaviorIndex = 0;
			return m_CurrentState;
		case BehaviorState::Success:
			++m_CurrentBehaviorIndex;
			m_CurrentState = BehaviorState::Running;
			return m_CurrentState;
		case BehaviorState::Running:
			return m_CurrentState;
		}
	}

	m_CurrentBehaviorIndex = 0;
	m_CurrentState = BehaviorState::Success;
	return m_CurrentState;
}
#pragma endregion


BehaviorState Conditional::Execute(Blackboard* blackboardPtr)
{
	if (m_ConditionalPtr == nullptr)
		return BehaviorState::Failure;

	if (m_ConditionalPtr(blackboardPtr))
	{
		m_CurrentState = BehaviorState::Success;
	}
	else
	{
		m_CurrentState = BehaviorState::Failure;
	}
	return m_CurrentState;
}

BehaviorState Action::Execute(Blackboard* blackboardPtr)
{
	if (m_ActionPtr == nullptr)
		return BehaviorState::Failure;

	m_CurrentState = m_ActionPtr(blackboardPtr);
	return m_CurrentState;
}

BehaviorTree::~BehaviorTree()
{
	SAFE_DELETE(m_RootBehaviorPtr)
		SAFE_DELETE(m_BlackboardPtr) //Takes ownership of passed blackboard!
}

void BehaviorTree::Update()
{
	if (m_RootBehaviorPtr == nullptr)
	{
		m_CurrentState = BehaviorState::Failure;
		return;
	}

	m_CurrentState = m_RootBehaviorPtr->Execute(m_BlackboardPtr);
}

Blackboard* BehaviorTree::GetBlackboard() const
{
	return m_BlackboardPtr;
}
