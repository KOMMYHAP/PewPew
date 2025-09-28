#pragma once
#include "resource_error.h"

class ResourceAdapter
{
public:
    virtual ~ResourceAdapter() = default;

    virtual std::optional<ResourceError> DoResourceLoad(std::span<const std::byte> content) = 0;

    using ResourceSaveQueryContent = std::vector<std::byte>;
    virtual std::expected<ResourceSaveQueryContent, ResourceError> DoResourceSave() = 0;
};

class ResourceLoadingAdapter : public ResourceAdapter
{
    std::expected<ResourceSaveQueryContent, ResourceError> DoResourceSave() final;
};