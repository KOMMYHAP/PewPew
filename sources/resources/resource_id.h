#pragma once

enum class ResourceTypeId : uint32_t
{
  Invalid = std::numeric_limits<std::underlying_type_t<ResourceTypeId>>::max()
};
enum class ResourcePathId : uint32_t
{
  Invalid = std::numeric_limits<std::underlying_type_t<ResourceTypeId>>::max()
};

class ResourceIdStorage
{
public:
  ResourceTypeId RegisterType(std::string_view path);
  ResourcePathId RegisterPath(std::string_view path);

  ResourceTypeId GetTypeId(std::string_view path) const;
  ResourcePathId GetPathId(std::string_view path) const;

  std::string_view GetType(ResourceTypeId typeId) const;
  std::string_view GetPath(ResourcePathId pathId) const;

  void Finalize();

private:
  bool _finalized{ false };
  std::unordered_map<std::string_view, ResourceTypeId> _resourceTypeToId;
  std::unordered_map<std::string_view, ResourcePathId> _resourcePathToId;
  std::vector<std::string_view> _resourceTypes;
  std::vector<std::string_view> _resourcePaths;
};