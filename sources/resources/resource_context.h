#pragma once

#include "resource_id.h"

class ResourceContext
{
public:
  explicit ResourceContext(std::span<const std::byte> bytes);

  template<class... Args>
  void Emplace(ResourcePathId pathId, Args&&...args);

  template <class T>
  const T* Find(ResourcePathId pathId) const;

  template <class T>
  T Extract(ResourcePathId pathId);

  std::span<const std::byte> GetResourceBytes() const { return _content; }

private:
  std::unordered_map<ResourcePathId, std::any> _idToResource;
  std::span<const std::byte> _content;
};

#include "resource_context.hpp"