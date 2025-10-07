#include "resource_registry.h"

ResourceNodeIterator::ResourceNodeIterator(const ResourceNode& node, size_t index)
  : _node(&node)
  , _childIndex(index)
{
}

ResourceNodeIterator::reference ResourceNodeIterator::operator*() const
{
  return *_node->_children[_childIndex];
}

ResourceNodeIterator::pointer ResourceNodeIterator::operator->() const
{
  return _node->_children[_childIndex].get();
}

ResourceNodeIterator& ResourceNodeIterator::operator++()
{
  _childIndex++;
  return *this;
}

ResourceNodeIterator ResourceNodeIterator::operator++(int)
{
  const ResourceNodeIterator tmp = *this;
  ++*this;
  return tmp;
}

ResourceNode::ResourceNode(const ResourceNode* parent, std::filesystem::path name, std::filesystem::path path)
  : _name(std::move(name))
  , _path(std::move(path))
  , _parent(parent)
{
}

ResourceNode& ResourceNode::AddChild(std::filesystem::path name, std::filesystem::path path)
{
  assert(name.has_filename());
  assert(!path.has_parent_path());
  _children.push_back(std::make_unique<ResourceNode>(this, std::move(name), std::move(path)));
  const std::unique_ptr<ResourceNode>& child = _children.back();
  return *child;
}

bool operator==(const ResourceNodeIterator& a, const ResourceNodeIterator& b)
{
  return std::forward_as_tuple(a._node, a._childIndex) == std::forward_as_tuple(b._node, b._childIndex);
}

bool operator!=(const ResourceNodeIterator& a, const ResourceNodeIterator& b)
{
  return !(a == b);
}

ResourceGraph::ResourceGraph(const std::filesystem::path& graphPath)
: _root(std::make_unique<ResourceNode>(nullptr, graphPath.parent_path(), graphPath.filename()))
{
}

void ResourceGraph::VisitBreadthFirst(Visitor visitor) const
{
  std::queue<const ResourceNode*> nodesToVisit;
  nodesToVisit.push(_root.get());
  while (!nodesToVisit.empty()) {
    const ResourceNode* node = nodesToVisit.front();
    nodesToVisit.pop();

    visitor(*node);

    for (const ResourceNode& child : *node) {
      nodesToVisit.push(&child);
    }
  }
}

ResourceNode& ResourceGraph::AddChild(const std::filesystem::path& path, std::filesystem::path name)
{
  std::filesystem::relative(_root, path, );
}

std::filesystem::path ResourceGraph::MakePathToNode(const std::filesystem::path& root, const ResourceNode& node)
{
  std::filesystem::path path = root;

  std::vector<const ResourceNode*> hierarchy;
  const ResourceNode* entry = &node;
  hierarchy.push_back(entry);
  while (entry->HasParent()) {
    entry = &entry->GetParent();
    hierarchy.push_back(entry);
  }

  for (const ResourceNode* parent : std::ranges::reverse_view(hierarchy)) {
    path /= parent->GetPath();
  }

  return path;
}