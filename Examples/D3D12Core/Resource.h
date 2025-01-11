#pragma once
#include "EnginePch.h"
#include <shared_mutex>
namespace dengine {
class Texture;
class Resource : public std::enable_shared_from_this<Resource>
{
public:
	void SetLoadType(ELoadType inLoadType) { m_loadType = inLoadType; }
	const ELoadType GetLoadType() { return m_loadType; }

	bool IsLoaded() { return m_loadType == ELoadType::Loaded; }
	bool NeedLoading();

	virtual shared_ptr<Texture> GetTexture() { return nullptr; }
private:
	ELoadType m_loadType = ELoadType::NotLoaded;
	EResourceType m_resourceType = EResourceType::None;

	std::shared_mutex m_resMutex;
};
}
