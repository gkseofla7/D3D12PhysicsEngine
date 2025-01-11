#include "Resource.h"
namespace dengine {
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