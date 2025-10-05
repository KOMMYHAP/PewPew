#pragma once
#include "resource_adapter.h"
#include "resource_registry.h"
#include "resource_url.h"

class DescRegistry
{
public:
  explicit DescRegistry(std::vector<std::filesystem::path> layers);

  ResourcesRegistry& ModifyIdStorage() { return _idStorage; }
  const ResourcesRegistry& GetIdStorage() const { return _idStorage; }

  template<class T, class... Args>
    requires std::constructible_from<T, Args...>
  std::expected<DescTypeId, ResourceError> Register(std::string_view type, Args&&... args)
  {
    return Register(type, std::make_unique<T>(std::forward<Args>(args)...));
  }

  std::optional<ResourceError> Load(DescTypeId type, ResourcePathId path);

private:
  std::expected<DescTypeId, ResourceError> Register(std::string_view type, std::unique_ptr<ResourceAdapter> adapter);

private:
  std::vector<std::filesystem::path> _layers; //< order is matter
  ResourcesRegistry _idStorage;
  std::map<std::string_view, DescTypeId> _resourceTypeToId;
  std::vector<std::unique_ptr<ResourceAdapter>> _resourceLoaders;
};