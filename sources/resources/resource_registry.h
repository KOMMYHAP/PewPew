#pragma once
#include "resource_desc.h"

class DescRegistry;

#include "desc.h"
#include "resource_url.h"
#include "string_hash.h"

class ResourceNode;

struct ResourceNodeIterator
{
  using iterator_category = std::random_access_iterator_tag;
  using difference_type   = std::ptrdiff_t;
  using value_type        = const ResourceNode;
  using pointer           = const ResourceNode*;
  using reference         = const ResourceNode&;

  ResourceNodeIterator() = default;
  ResourceNodeIterator(const ResourceNode& node, size_t index);

  reference operator*() const;
  pointer operator->() const;

  ResourceNodeIterator& operator++();

  ResourceNodeIterator operator++(int);

  friend bool operator== (const ResourceNodeIterator& a, const ResourceNodeIterator& b);
  friend bool operator!= (const ResourceNodeIterator& a, const ResourceNodeIterator& b);

private:
  const ResourceNode*  _node{nullptr};
  size_t _childIndex{0};
};

class ResourceNode
{
  friend struct ResourceNodeIterator;
public:
  ResourceNode(const ResourceNode * parent, std::filesystem::path name, std::filesystem::path path);

  const std::filesystem::path& GetName() const { return _name; }
  const std::filesystem::path& GetPath() const { return _path; }
  const ResourceNode& GetParent() const { return *_parent; }
  bool HasParent() const { return _parent != nullptr; }

  ResourceNode & AddChild(std::filesystem::path name, std::filesystem::path path);

  using Iterator = const ResourceNode*;

  ResourceNodeIterator begin() const { return ResourceNodeIterator(*this, 0); }
  ResourceNodeIterator end() const { return ResourceNodeIterator(*this, _children.size()); }

private:
  std::filesystem::path _path; //< relative to the parent
  std::vector<std::unique_ptr<ResourceNode>> _children;
  const ResourceNode* _parent{nullptr};
};

enum class ResourceNodeId : uint16_t
{
  Invalid = std::numeric_limits<uint16_t>::max()
};

class ResourceNodeLink
{
  ResourceNodeId parent{ResourceNodeId::Invalid};
  ResourceNodeId node{ResourceNodeId::Invalid};
  std::vector<ResourceNodeId> children;
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
  using Visitor = std::move_only_function<VisitorStep(const ResourceNode& /*node*/)>;
  void VisitBreadthFirst(Visitor visitor) const;

  ResourceNode & AddChild(const std::filesystem::path& path, std::filesystem::path name);

  static std::filesystem::path MakePathToNode(const std::filesystem::path &root, const ResourceNode& node);

private:
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