#include "Resource.h"
namespace dengine {

void Resource::SetLoadType(ELoadType inLoadType)
{
	m_loadType = inLoadType;
	if (m_loadType == ELoadType::Loaded)
	{
		if (static_cast<bool>(m_loadCallback))
		{
			m_loadCallback();
		}
	}
}
bool Resource::NeedLoading()
{
	std::unique_lock<std::shared_mutex> writeLock(m_resMutex);
	if (m_loadType != ELoadType::NotLoaded)
	{
		return false;
	}
	m_loadType = ELoadType::Loading;

	return true;
}
}