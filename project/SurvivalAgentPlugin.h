#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"

namespace BT
{
	class BehaviourTree;
}

class Blackboard;
class Brain;
class IBaseInterface;
class IExamInterface;

class SurvivalAgentPlugin final : public IExamPlugin
{
public:
	SurvivalAgentPlugin() = default;
	~SurvivalAgentPlugin() override;

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;

private:
	//Interface, used to request data from/perform actions with the AI Framework
	IExamInterface* m_pInterface = nullptr;

	Elite::Vector2 m_Target = {};
	bool m_CanRun = false; //Demo purpose
	bool m_GrabItem = false; //Demo purpose
	bool m_UseItem = false; //Demo purpose
	bool m_RemoveItem = false; //Demo purpose
	bool m_DestroyItemsInFOV = false;
	float m_AngSpeed = 5.f; //Demo purpose

	UINT m_InventorySlot = 0;

	Blackboard* CreateBlackboard() const;
	void UpdateBlackboard(const SteeringPlugin_Output& steering);
	BT::BehaviourTree* m_BehaviourTree;
	Brain* m_pBrain;

	bool m_Running{};
};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	inline __declspec (dllexport) IPluginBase* Register()
	{
		return new SurvivalAgentPlugin();
	}
}