#include "resource_registry.h"

ResourceNodeIterator::ResourceNodeIterator(const ResourceGraph& graph, const ResourceNodeLink& link, size_t index)
  : _graph(&graph)
  , _link(&link)
  , _childIndex(index)
{
}

ResourceNodeIterator::reference ResourceNodeIterator::operator*() const
{
  const ResourceNodeId childId = _link->GetChildrenIds()[_childIndex];
  return _graph->GetNode(childId);
}

ResourceNodeIterator::pointer ResourceNodeIterator::operator->() const
{
  const ResourceNodeId childId = _link->GetChildrenIds()[_childIndex];
  return &_graph->GetNode(childId);
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

ResourceNodeLinkAdapter::ResourceNodeLinkAdapter(const ResourceGraph& graph, const ResourceNodeLink& link)
  : _graph(&graph)
  , _link(&link)
{
}

const ResourceNode& ResourceNodeLinkAdapter::GetAncestor() const
{
  return _graph->GetNode(GetAncestorId());
}

ResourceNodeId ResourceNodeLinkAdapter::GetAncestorId() const
{
  return _link->GetAncestorId();
}

const std::vector<ResourceNodeId>& ResourceNodeLinkAdapter::GetChildrenIds() const
{
  return _link->GetChildrenIds();
}

ResourceNodeIterator ResourceNodeLinkAdapter::begin() const
{
  return ResourceNodeIterator{ *_graph, *_link, 0 };
}

ResourceNodeIterator ResourceNodeLinkAdapter::end() const
{
  return ResourceNodeIterator{ *_graph, *_link, GetChildrenIds().size() };
}

bool operator==(const ResourceNodeIterator& a, const ResourceNodeIterator& b)
{
  return std::forward_as_tuple(a._graph, a._link, a._childIndex) == std::forward_as_tuple(b._graph, b._link, b._childIndex);
}

bool operator!=(const ResourceNodeIterator& a, const ResourceNodeIterator& b)
{
  return !(a == b);
}

ResourceNodeLink::ResourceNodeLink(ResourceNodeId parent)
  : _parent(parent)
{
}

void ResourceNodeLink::Add(ResourceNodeId child)
{
  _children.push_back(child);
}

ResourceNodeLinkAdapter ResourceNodeLink::Unwrap(const ResourceGraph& graph) const
{
  return ResourceNodeLinkAdapter{ graph, *this };
}


void ResourceGraph::VisitBreadthFirst(Visitor visitor) const
{
  std::queue<ResourceNodeId> nodesToVisit;
  nodesToVisit.push(_root);
  while (!nodesToVisit.empty()) {
    const ResourceNodeId id = nodesToVisit.front();
    nodesToVisit.pop();

    const VisitorStep step = visitor(id);
    if (step == VisitorStep::Stop) {
      break;
    }

    std::optional<ResourceNodeLinkAdapter> link = FindDependencyLink(id);
    if (!link.has_value()) {
      assert(false && "cannot find link for one of child node!");
      continue;
    }
    for (const ResourceNodeId& childId : link->GetChildrenIds()) {
      nodesToVisit.push(childId);
    }
  }
}

void ResourceGraph::LinkDependency(ResourceNodeId from, ResourceNodeId to)
{
  const ResourceNodeId nodeOwnerId = from;
  const ResourceNodeId requiredNodeId = to;

  ResourceNodeLink* nodeOwnerLink = nullptr;
  const auto it = std::ranges::find_if(_dependenciesGraph, [nodeOwnerId](const ResourceNodeLink& link) {
    return link.GetAncestorId() == nodeOwnerId;
  });
  if (it == _dependenciesGraph.end()) {
    nodeOwnerLink = &_dependenciesGraph.emplace_back(nodeOwnerId);
  } else {
    nodeOwnerLink = &(*it);
  }

  nodeOwnerLink->Add(requiredNodeId);
}

const ResourceNode& ResourceGraph::GetNode(ResourceNodeId id) const
{
  const size_t index = std::to_underlying(id);
  assert(index < _nodes.size() && "invalid node id!");
  return _nodes[index];
}

ResourceNode& ResourceGraph::ModifyNode(ResourceNodeId id)
{
  return _nodes[std::to_underlying(id)];
}

std::optional<ResourceNodeLinkAdapter> ResourceGraph::FindDependencyLink(ResourceNodeId id) const
{
  auto it = std::ranges::find_if(_dependenciesGraph, [id](const ResourceNodeLink& link) {
    return id == link.GetAncestorId();
  });
  if (it == _dependenciesGraph.end()) {
    return std::nullopt;
  }
  return it->Unwrap(*this);
}

ResourceNodeId ResourceGraph::CreateNode(const std::filesystem::path& path)
{
  const size_t nodeIndex = _nodes.size();
  const ResourceNodeId nextNodeId = static_cast<ResourceNodeId>(nodeIndex);
  auto [it, newPath] = _indexNodesByPath.emplace(path, nextNodeId);
  if (newPath) {
    _nodes.emplace_back(path);
  }
  return it->second;
}