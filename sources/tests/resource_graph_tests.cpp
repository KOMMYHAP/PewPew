#include "std_headers.h"

#include <gtest/gtest.h>

#include "resource_registry.h"

TEST(ResourceGraphTests, EmptyGraph)
{
  ResourceGraph graph;
  const ResourceNodeId id = graph.CreateNode("root/a.txt");
  ASSERT_FALSE(graph.FindDependencyLink(id).has_value());
}

TEST(ResourceGraphTests, OneLevelOfChildren)
{
  ResourceGraph graph;
  const ResourceNodeId rootId = graph.CreateNode("root/root.graph");
  const ResourceNodeId id1 = graph.CreateNode("root/a.txt");
  const ResourceNodeId id2 = graph.CreateNode("root/b.txt");
  graph.LinkDependency(rootId, id1);
  graph.LinkDependency(rootId, id2);

  const std::optional<ResourceNodeLinkAdapter> link =  graph.FindDependencyLink(rootId);
  ASSERT_TRUE(link.has_value());
  ASSERT_EQ(link->GetChildrenIds().size(), 2);
  ASSERT_EQ(link->GetAncestorId(), rootId);

  ASSERT_EQ(link->GetChildrenIds()[0], id1);
  ASSERT_EQ(link->GetChildrenIds()[1], id2);

  ASSERT_EQ(graph.GetNode(link->GetChildrenIds()[0]).path, "root/a.txt");
  ASSERT_EQ(graph.GetNode(link->GetChildrenIds()[1]).path, "root/b.txt");
}

TEST(ResourceGraphTests, DenendencyWalker)
{
  ResourceGraph graph;
  const ResourceNodeId id_a = graph.CreateNode("a");
  const ResourceNodeId id_b = graph.CreateNode("b");
  const ResourceNodeId id_c = graph.CreateNode("c");
  const ResourceNodeId id_d = graph.CreateNode("d");
  graph.LinkDependency(id_a, id_b);
  graph.LinkDependency(id_b, id_c);
  graph.LinkDependency(id_c, id_d);

  std::vector<ResourceNodeId> nodes;
  graph.VisitBreadthFirst([&nodes](const ResourceNodeId & id) {
    nodes.push_back(id);
    return ResourceGraph::VisitorStep::Continue;
  });

  ASSERT_EQ(nodes.size(), 4);
  ASSERT_EQ(nodes[0], id_a);
  ASSERT_EQ(nodes[1], id_b);
  ASSERT_EQ(nodes[2], id_c);
  ASSERT_EQ(nodes[3], id_d);
}