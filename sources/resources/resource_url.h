#pragma once
#include "resource_id.h"

class ResourceURL
{
public:
  ResourceURL() = default;

  DescTypeId GetType() const { return _type; }

  ResourcePathId GetPath() const { return _path; }

  auto operator<=>(const ResourceURL&) const = default;

private:
  ResourceURL(std::string_view type, std::string_view path);

  DescTypeId _type{ DescTypeId::Invalid };
  ResourcePathId _path{ ResourcePathId::Invalid };
};