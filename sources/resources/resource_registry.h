#pragma once

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
  ResourceNode(std::string name, std::string path);

  std::string_view GetName() const { return _name; }
  std::string_view GetPath() const { return _path; }

  void AddChild(std::string name, std::string path);

  using Iterator = const ResourceNode*;

  ResourceNodeIterator begin() const { return ResourceNodeIterator(*this, 0); }
  ResourceNodeIterator end() const { return ResourceNodeIterator(*this, _children.size()); }

private:
  std::string _name;
  std::string _path; //< relative to the parent
  std::vector<std::unique_ptr<ResourceNode>> _children;
};

class ResourceGraph
{
public:
  enum class VisitorStep
  {
    Continue,
    Stop
  };
  using Visitor = std::move_only_function<VisitorStep(const ResourceNode& /*node*/)>;
  void BreadthFirstSearch(Visitor visitor) const;


private:
  std::unique_ptr<ResourceNode> _root;
  std::string _rootPath;
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