#pragma once
#include <string>

struct ResourceDescNode
{
  std::string path;
};

struct ResourceDescGraph
{
  std::vector<ResourceDescNode> nodes;
};