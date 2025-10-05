#pragma once
#include "desc.h"
#include "resource_id.h"

class ResourceURL
{
public:
  ResourceURL() = default;
  ResourceURL(DescTypeId resourceType, ResourcePathId pathId);

  DescTypeId GetType() const { return _resourceType; }

  ResourcePathId GetPath() const { return _resourcePath; }

  auto operator<=>(const ResourceURL&) const = default;

private:
  DescTypeId _resourceType{ DescTypeId::Invalid };
  ResourcePathId _resourcePath{ ResourcePathId::Invalid };
};