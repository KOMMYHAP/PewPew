#pragma once

enum class DescTypeId : uint32_t
{
  Invalid = std::numeric_limits<std::underlying_type_t<DescTypeId>>::max()
};
enum class ResourcePathId : uint32_t
{
  Invalid = std::numeric_limits<std::underlying_type_t<DescTypeId>>::max()
};

class ResourceIdStorage
{
public:
  DescTypeId RegisterType(std::string_view path);
  ResourcePathId RegisterPath(std::string_view path);

  DescTypeId GetTypeId(std::string_view path) const;
  ResourcePathId GetPathId(std::string_view path) const;

  std::string_view GetType(DescTypeId typeId) const;
  std::string_view GetPath(ResourcePathId pathId) const;

  void Finalize();

private:
  bool _finalized{ false };
  std::unordered_map<std::string_view, DescTypeId> _resourceTypeToId;
  std::unordered_map<std::string_view, ResourcePathId> _resourcePathToId;
  std::vector<std::string_view> _resourceTypes;
  std::vector<std::string_view> _resourcePaths;
};