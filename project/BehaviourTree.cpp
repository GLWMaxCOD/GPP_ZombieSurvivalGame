#include "stdafx.h"
#include "BehaviourTree.h"
#include "SurvivalAgentPlugin.h"

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
State Selector::Execute(Blackboard* pBlackboard)
{
	// Loop over all children in m_ChildBehaviours
	for (IBehaviour* pBeh : m_ChildBehaviours)
	{
		//Every Child: Execute and store the result in m_CurrentState
		m_CurrentState = pBeh->Execute(pBlackboard);

		//Check the currentstate and apply the selector Logic:
		//if a child returns Success:
		if (m_CurrentState == State::Success)
		{
			//stop looping over all children and return Success
			return m_CurrentState;
		}

		//if a child returns Running:
		if (m_CurrentState == State::Running)
		{
			//Running: stop looping and return Running
			return m_CurrentState;
		}
		//The selector fails if all children failed.	
	}

	//All children failed
	m_CurrentState = State::Failure;
	return m_CurrentState;
}
//SEQUENCE
State Sequence::Execute(Blackboard* pBlackboard)
{
	//Loop over all children in m_ChildBehaviours
	for (IBehaviour* pBeh : m_ChildBehaviours)
	{
		//Every Child: Execute and store the result in m_CurrentState
		m_CurrentState = pBeh->Execute(pBlackboard);

		//Check the currentstate and apply the sequence Logic:
		//if a child returns Failed:
		if (m_CurrentState == State::Failure)
		{
			//stop looping over all children and return Failed
			return m_CurrentState;
		}

		//if a child returns Running:
		if (m_CurrentState == State::Running)
		{
			//Running: stop looping and return Running
			return m_CurrentState;

		}

		//The selector succeeds if all children succeeded.
	}

	//All children succeeded 
	m_CurrentState = State::Success;
	return m_CurrentState;
}

State PartialSequence::Execute(Blackboard* pBlackboard)
{
	while (m_CurrentBehaviourIndex < m_ChildBehaviours.size())
	{
		m_CurrentState = m_ChildBehaviours[m_CurrentBehaviourIndex]->Execute(pBlackboard);
		switch (m_CurrentState)
		{
		case State::Failure:
			m_CurrentBehaviourIndex = 0;
			return m_CurrentState;
		case State::Success:
			++m_CurrentBehaviourIndex;
			m_CurrentState = State::Running;
			return m_CurrentState;
		case State::Running:
			return m_CurrentState;
		}
	}

	m_CurrentBehaviourIndex = 0;
	m_CurrentState = State::Success;
	return m_CurrentState;
}
#pragma endregion


State Conditional::Execute(Blackboard* pBlackboard)
{
	if (m_ConditionalPtr == nullptr)
		return State::Failure;

	if (m_ConditionalPtr(pBlackboard))
	{
		m_CurrentState = State::Success;
	}
	else
	{
		m_CurrentState = State::Failure;
	}
	return m_CurrentState;
}

State Action::Execute(Blackboard* pBlackboard)
{
	if (m_ActionPtr == nullptr)
		return State::Failure;

	m_CurrentState = m_ActionPtr(pBlackboard);
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
		m_CurrentState = State::Failure;
		return;
	}

	m_CurrentState = m_RootBehaviourPtr->Execute(m_BlackboardPtr);
}

Blackboard* BehaviourTree::GetBlackboard() const
{
	return m_BlackboardPtr;
}
