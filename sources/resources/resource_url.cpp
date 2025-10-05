#include "resource_url.h"

ResourceURL::ResourceURL(DescTypeId resourceType, ResourcePathId pathId)
  : _resourceType(resourceType)
  , _resourcePath(pathId)
{
}