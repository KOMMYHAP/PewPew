#include "resource_adapter.h"

std::expected<ResourceAdapter::ResourceSaveQueryContent, ResourceError> ResourceLoadingAdapter::DoResourceSave()
{
    ResourceError result{ResourceErrorCode::UnsupportedOperation, "Only loading operation is supported"};
    return std::unexpected(std::move(result));
}