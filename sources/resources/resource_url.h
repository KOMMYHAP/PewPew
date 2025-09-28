#pragma once
#include "resource_id.h"

class ResourceURL
{
public:
  ResourceURL() = default;

  ResourceTypeId GetType() const { return _type; }

  ResourcePathId GetPath() const { return _path; }

  auto operator<=>(const ResourceURL&) const = default;

private:
  ResourceURL(std::string_view type, std::string_view path);

  ResourceTypeId _type{ ResourceTypeId::Invalid };
  ResourcePathId _path{ ResourcePathId::Invalid };
};