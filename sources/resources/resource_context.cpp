#include "resource_context.h"

ResourceContext::ResourceContext(std::span<const std::byte> bytes)
  : _content(bytes)
{
}