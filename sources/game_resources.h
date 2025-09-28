#pragma once
#include "resource_system.h"

struct GameSettings
{
  int32_t windowSizeX{0};
  int32_t windowSizeY{0};

  int32_t playerPositionX{0};
  int32_t playerPositionY{0};
  int32_t playerSpeed{0};
  uint32_t playerColor{0};

  int32_t bulletSpeed{0};
  uint32_t bulletColor{0};
};

class GameResources
{
public:
  GameResources(std::filesystem::path resourceRoot);
  ~GameResources();

  GameSettings GetSettings() const;

private:
  struct Impl;

  ResourceSystem _resourceSystem;
  std::unique_ptr<Impl> _impl;
};
