#pragma once
#include "resource_adapter.h"
#include "resource_id.h"
#include "resource_url.h"

class ResourceSystem
{
public:
  explicit ResourceSystem(std::vector<std::filesystem::path> layers);

  ResourceIdStorage& ModifyIdStorage() { return _idStorage; }
  const ResourceIdStorage& GetIdStorage() const { return _idStorage; }

  template<class T, class... Args>
    requires std::constructible_from<T, Args...>
  std::expected<ResourceTypeId, ResourceError> Register(std::string_view type, Args&&... args)
  {
    return Register(type, std::make_unique<T>(std::forward<Args>(args)...));
  }

  std::optional<ResourceError> Load(ResourceTypeId type, ResourcePathId path);

private:
  std::expected<ResourceTypeId, ResourceError> Register(std::string_view type, std::unique_ptr<ResourceAdapter> adapter);

private:
  std::vector<std::filesystem::path> _layers; //< order is matter
  ResourceIdStorage _idStorage;
  std::map<std::string_view, ResourceTypeId> _resourceTypeToId;
  std::vector<std::unique_ptr<ResourceAdapter>> _resourceLoaders;
};