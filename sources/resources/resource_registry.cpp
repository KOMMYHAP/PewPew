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
  ++(*this);
  return tmp;
}

bool operator==(const ResourceNodeIterator& a, const ResourceNodeIterator& b)
{
  return std::forward_as_tuple(a._node, a._childIndex) == std::forward_as_tuple(b._node, b._childIndex);
}

bool operator!=(const ResourceNodeIterator& a, const ResourceNodeIterator& b)
{
  return !(a == b);
}

void ResourceGraph::BreadthFirstSearch(Visitor visitor) const
{
  std::vector<const ResourceNode*> nodes;

  std::queue<const ResourceNode*> nodesToVisit;
  nodesToVisit.push(_root.get());
  while (!nodesToVisit.empty()) {
    const ResourceNode* node = nodesToVisit.front();
    nodesToVisit.pop();

    nodes.push_back(node);

    for (const ResourceNode & child : *node) {
      nodesToVisit.push(&child);
    }
  }

  for (const ResourceNode* node : std::ranges::reverse_view(nodes)) {
    visitor(*node);
  }
}