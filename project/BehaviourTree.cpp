#include "stdafx.h"
#include "BehaviourTree.h"

using namespace BT;

#pragma region COMPOSITES
Composite::Composite(const std::vector<IBehaviour*>& childBehaviours)
{
	m_ChildBehaviours = childBehaviours;
}

Composite::~Composite()
{
	for (auto pb : m_ChildBehaviours)
		SAFE_DELETE(pb)
		m_ChildBehaviours.clear();
}

//SELECTOR
BehaviourState Selector::Execute(Blackboard* blackboardPtr)
{
	// Loop over all children in m_ChildBehaviours
	for (IBehaviour* pBeh : m_ChildBehaviours)
	{
		//Every Child: Execute and store the result in m_CurrentState
		m_CurrentState = pBeh->Execute(blackboardPtr);

		//Check the currentstate and apply the selector Logic:
		//if a child returns Success:
		if (m_CurrentState == BehaviourState::Success)
		{
			//stop looping over all children and return Success
			return m_CurrentState;
		}

		//if a child returns Running:
		if (m_CurrentState == BehaviourState::Running)
		{
			//Running: stop looping and return Running
			return m_CurrentState;
		}
		//The selector fails if all children failed.	
	}

	//All children failed
	m_CurrentState = BehaviourState::Failure;
	return m_CurrentState;
}
//SEQUENCE
BehaviourState Sequence::Execute(Blackboard* blackboardPtr)
{
	//Loop over all children in m_ChildBehaviours
	for (IBehaviour* pBeh : m_ChildBehaviours)
	{
		//Every Child: Execute and store the result in m_CurrentState
		m_CurrentState = pBeh->Execute(blackboardPtr);

		//Check the currentstate and apply the sequence Logic:
		//if a child returns Failed:
		if (m_CurrentState == BehaviourState::Failure)
		{
			//stop looping over all children and return Failed
			return m_CurrentState;
		}

		//if a child returns Running:
		if (m_CurrentState == BehaviourState::Running)
		{
			//Running: stop looping and return Running
			return m_CurrentState;

		}

		//The selector succeeds if all children succeeded.
	}

	//All children succeeded 
	m_CurrentState = BehaviourState::Success;
	return m_CurrentState;
}

BehaviourState PartialSequence::Execute(Blackboard* blackboardPtr)
{
	while (m_CurrentBehaviourIndex < m_ChildBehaviours.size())
	{
		m_CurrentState = m_ChildBehaviours[m_CurrentBehaviourIndex]->Execute(blackboardPtr);
		switch (m_CurrentState)
		{
		case BehaviourState::Failure:
			m_CurrentBehaviourIndex = 0;
			return m_CurrentState;
		case BehaviourState::Success:
			++m_CurrentBehaviourIndex;
			m_CurrentState = BehaviourState::Running;
			return m_CurrentState;
		case BehaviourState::Running:
			return m_CurrentState;
		}
	}

	m_CurrentBehaviourIndex = 0;
	m_CurrentState = BehaviourState::Success;
	return m_CurrentState;
}
#pragma endregion


BehaviourState Conditional::Execute(Blackboard* blackboardPtr)
{
	if (m_ConditionalPtr == nullptr)
		return BehaviourState::Failure;

	if (m_ConditionalPtr(blackboardPtr))
	{
		m_CurrentState = BehaviourState::Success;
	}
	else
	{
		m_CurrentState = BehaviourState::Failure;
	}
	return m_CurrentState;
}

BehaviourState Action::Execute(Blackboard* blackboardPtr)
{
	if (m_ActionPtr == nullptr)
		return BehaviourState::Failure;

	m_CurrentState = m_ActionPtr(blackboardPtr);
	return m_CurrentState;
}

BehaviourTree::~BehaviourTree()
{
	SAFE_DELETE(m_RootBehaviourPtr)
		SAFE_DELETE(m_BlackboardPtr) //Takes ownership of passed blackboard!
}

void BehaviourTree::Update()
{
	if (m_RootBehaviourPtr == nullptr)
	{
		m_CurrentState = BehaviourState::Failure;
		return;
	}

	m_CurrentState = m_RootBehaviourPtr->Execute(m_BlackboardPtr);
}

Blackboard* BehaviourTree::GetBlackboard() const
{
	return m_BlackboardPtr;
}
