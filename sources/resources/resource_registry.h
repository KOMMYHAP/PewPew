#pragma once
#include "resource_desc.h"


class ResourceNodeLink;
class ResourceGraph;
class DescRegistry;

#include "desc.h"
#include "resource_url.h"
#include "string_hash.h"

struct ResourceNode;


enum class ResourceNodeId : uint16_t
{
  Invalid = std::numeric_limits<uint16_t>::max()
};

struct ResourceNodeIterator
{
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = const ResourceNode;
  using pointer = const ResourceNode*;
  using reference = const ResourceNode&;

  ResourceNodeIterator() = default;
  ResourceNodeIterator(const ResourceGraph& graph, const ResourceNodeLink& link, size_t index);

  reference operator*() const;
  pointer operator->() const;

  ResourceNodeIterator& operator++();

  ResourceNodeIterator operator++(int);

  friend bool operator==(const ResourceNodeIterator& a, const ResourceNodeIterator& b);
  friend bool operator!=(const ResourceNodeIterator& a, const ResourceNodeIterator& b);

private:
  const ResourceGraph* _graph{ nullptr };
  const ResourceNodeLink* _link{ nullptr };
  size_t _childIndex{ 0 };
};

class ResourceNodeLinkAdapter
{
public:
  ResourceNodeLinkAdapter(const ResourceGraph& graph, const ResourceNodeLink& link);

  const ResourceNode& GetParent() const;
  ResourceNodeId GetParentId() const;
  const std::vector<ResourceNodeId>& GetChildrenIds() const;

  ResourceNodeIterator begin() const;
  ResourceNodeIterator end() const;

private:
  const ResourceGraph* _graph{ nullptr };
  const ResourceNodeLink* _link{ nullptr };
};

class ResourceNodeLink
{
public:
  ResourceNodeLink(ResourceNodeId parent);

  void Add(ResourceNodeId child);
  ResourceNodeLinkAdapter Unwrap(const ResourceGraph& graph) const;

  ResourceNodeId GetParentId() const { return _parent; }

  const std::vector<ResourceNodeId>& GetChildrenIds() const { return _children; }

private:
  ResourceNodeId _parent{ ResourceNodeId::Invalid };
  std::vector<ResourceNodeId> _children;
};

struct ResourceNode
{
  std::filesystem::path path; //< absolute path
};

class ResourceGraph
{
public:
  ResourceGraph(const std::filesystem::path& graphPath);

  enum class VisitorStep
  {
    Continue,
    Stop
  };
  using Visitor = std::move_only_function<VisitorStep(const ResourceNodeLinkAdapter&)>;
  void VisitBreadthFirst(Visitor visitor) const;

  void LinkDependency(const std::filesystem::path& nodeOwner, const std::filesystem::path& requiredNode);

  const ResourceNode& GetNode(ResourceNodeId id) const;
  ResourceNode& ModifyNode(ResourceNodeId id);

  std::optional<ResourceNodeLinkAdapter> FindDependencyLink(ResourceNodeId id) const;

private:
  ResourceNodeId RequireNode(const std::filesystem::path& path);

  ResourceNodeId _root{ ResourceNodeId::Invalid };
  std::vector<ResourceNode> _nodes;
  std::unordered_map<std::filesystem::path, ResourceNodeId> _indexNodesByPath;
  std::vector<ResourceNodeLink> _dependenciesGraph;
};

class ResourcesRegistry
{
public:
  ResourcesRegistry(DescRegistry& descRegistry);

  ResourceURL Register(std::string_view path);

  ResourcePathId FindPathId(std::string_view path) const;
  std::string_view GetPath(ResourcePathId pathId) const;

private:
  Ref<DescRegistry> descRegistry;
  std::unordered_map<std::string, ResourcePathId, string_hash, std::equal_to<>> _resourcePathToId;
  std::vector<std::string> _resourcePaths;
};