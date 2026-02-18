#include "Component.h"
#include "GameObject.h"

namespace dae
{
	Component::Component(GameObject* pOwner)
		: m_pOwner(pOwner)
	{
	}
}