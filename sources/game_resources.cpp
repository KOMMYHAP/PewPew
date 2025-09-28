#include "game_resources.h"

namespace {

constexpr std::string_view ResourceGraphType{ "resource_graph" };
constexpr std::string_view GameSettingsResourceType{ "game_settings" };
constexpr std::string_view InitialResourceGraphPath{ "root_graph.json" };

class GameSettingsAdapter final : public ResourceAdapter
{
public:
  explicit GameSettingsAdapter(GameSettings& gameSettings);

  std::optional<ResourceError> DoResourceLoad(std::span<const std::byte> content) override;
  std::expected<ResourceSaveQueryContent, ResourceError> DoResourceSave() override;

private:
  Ref<GameSettings> _gameSettings;
};

struct ResourceLoadingEntry
{
  ResourcePathId path{ ResourcePathId::Invalid };
  ResourceTypeId type{ ResourceTypeId::Invalid };
};

struct ResourceEntryList
{
  std::vector<ResourceLoadingEntry> entries;
};

class ResourceGraphAdapter final : public ResourceLoadingAdapter
{
public:
  ResourceGraphAdapter(ResourceIdStorage& storage, ResourceEntryList& loadingOrder);
  std::optional<ResourceError> DoResourceLoad(std::span<const std::byte> content) override;

private:
  Ref<ResourceIdStorage> _idStorage;
  Ref<ResourceEntryList> _loadingOrder;
};

GameSettingsAdapter::GameSettingsAdapter(GameSettings& gameSettings)
  : _gameSettings(&gameSettings)
{
}

std::optional<ResourceError> GameSettingsAdapter::DoResourceLoad(std::span<const std::byte> content)
{
  const Json root = Json::parse(content.data(), content.data() + content.size(), nullptr, false, true);
  if (!root.is_object()) {
    return ResourceError{ ResourceErrorCode::InvalidContent, "root must be object" };
  }

  if (root["window_size_x"].is_number_integer()) {
    return ResourceError{ ResourceErrorCode::InvalidContent, "\"window_size_x\" must be integer" };
  }
  if (root["window_size_y"].is_number_integer()) {
    return ResourceError{ ResourceErrorCode::InvalidContent, "\"window_size_y\" must be integer" };
  }

  if (root["player_position_x"].is_number_integer()) {
    return ResourceError{ ResourceErrorCode::InvalidContent, "\"player_position_x\" must be integer" };
  }
  if (root["player_position_y"].is_number_integer()) {
    return ResourceError{ ResourceErrorCode::InvalidContent, "\"player_position_y\" must be integer" };
  }
  if (root["player_speed"].is_number_integer()) {
    return ResourceError{ ResourceErrorCode::InvalidContent, "\"player_speed\" must be integer" };
  }
  if (root["player_color"].is_number_unsigned()) {
    return ResourceError{ ResourceErrorCode::InvalidContent, "\"player_color\" must be unsigned integer" };
  }

  if (root["bullet_speed"].is_number_integer()) {
    return ResourceError{ ResourceErrorCode::InvalidContent, "\"bullet_speed\" must be integer" };
  }
  if (root["bullet_color"].is_number_unsigned()) {
    return ResourceError{ ResourceErrorCode::InvalidContent, "\"bullet_color\" must be unsigned integer" };
  }

  root["window_size_x"].get_to(_gameSettings->windowSizeX);
  root["window_size_y"].get_to(_gameSettings->windowSizeY);
  root["player_position_x"].get_to(_gameSettings->playerPositionX);
  root["player_position_y"].get_to(_gameSettings->playerPositionY);
  root["player_speed"].get_to(_gameSettings->playerSpeed);
  root["player_color"].get_to(_gameSettings->playerColor);
  root["bullet_speed"].get_to(_gameSettings->bulletSpeed);
  root["bullet_color"].get_to(_gameSettings->bulletColor);

  return {};
}

ResourceGraphAdapter::ResourceGraphAdapter(ResourceIdStorage& storage, ResourceEntryList& loadingOrder)
  : _idStorage(&storage)
  , _loadingOrder(&loadingOrder)
{
}

std::optional<ResourceError> ResourceGraphAdapter::DoResourceLoad(std::span<const std::byte> content)
{
  const Json root = Json::parse(content.data(), content.data() + content.size(), nullptr, false, true);
  const Json resources = root["resources"];
  if (!resources) {
    return ResourceError{ ResourceErrorCode::InvalidContent, "key \"resources\" not found" };
  }
  if (!resources.is_array()) {
    return ResourceError{ ResourceErrorCode::InvalidContent, "key \"resources\" must be array" };
  }

  _loadingOrder->entries.reserve(resources.size());
  for (int index = 0; index < resources.size(); ++index) {
    const Json resource = resources[index];
    if (!resource.is_object()) {
      return ResourceError{ ResourceErrorCode::InvalidContent, std::format("entry #{} of \"resources\" must be object", index) };
    }

    const Json path = resource["path"];
    if (!path) {
      return ResourceError{ ResourceErrorCode::InvalidContent, "resource must have \"path\" key" };
    }
    if (!path.is_string()) {
      return ResourceError{ ResourceErrorCode::InvalidContent, "value by key \"path\" must be string" };
    }
    const Json type = resource["type"];
    if (!type) {
      return ResourceError{ ResourceErrorCode::InvalidContent, "resource must have \"type\" key" };
    }
    if (!type.is_string()) {
      return ResourceError{ ResourceErrorCode::InvalidContent, "value by key \"type\" must be string" };
    }

    ResourceLoadingEntry& entry = _loadingOrder->entries.emplace_back();
    entry.type = _idStorage->RegisterType(type.get<std::string_view>());
    entry.path = _idStorage->RegisterPath(path.get<std::string_view>());
  }

  return {};
}

}

struct GameResources::Impl
{
  ResourceEntryList resourcesOrderLoading;
  GameSettings gameSettings;
};

GameResources::GameResources(std::filesystem::path resourceRoot)
  : _resourceSystem{ std::vector{ std::move(resourceRoot) } }
  , _impl{ std::make_unique<Impl>() }
{
  ResourceIdStorage& idStorage = _resourceSystem.ModifyIdStorage();
  std::expected<ResourceTypeId, ResourceError> result = _resourceSystem.Register<ResourceGraphAdapter>(ResourceGraphType, idStorage, _impl->resourcesOrderLoading);
  if (!result) {
    std::println("Failed to register initial resource graph: {}", result.error().ToString());
    return;
  }
  if (std::expected<ResourceTypeId, ResourceError> r = _resourceSystem.Register<GameSettingsAdapter>(GameSettingsResourceType, _impl->gameSettings); !r) {
    std::println("Failed to register type \"{}\": {}", GameSettingsResourceType, r.error().ToString());
    return;
  }

  const ResourcePathId initialResourcePath = idStorage.RegisterPath(InitialResourceGraphPath);
  if (const std::optional<ResourceError> error = _resourceSystem.Load(result.value(), initialResourcePath); error.has_value()) {
    std::println("Failed to register initial resource graph: {}", error->ToString());
    return;
  }
}

GameResources::~GameResources() = default;

GameSettings GameResources::GetSettings() const
{
  return _impl->gameSettings;
}