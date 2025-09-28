#include "resource_error.h"

#include <system_error>

namespace
{

class ResourceAdapterErrorCategory final : public std::error_category
{
public:
  [[nodiscard]] const char* name() const noexcept override;
  [[nodiscard]] std::string message(int value) const override;
};


const char *ResourceAdapterErrorCategory::name() const noexcept
{
  return "ResourceAdapter";
}

[[nodiscard]] std::string ResourceAdapterErrorCategory::message(int value) const
{
  switch (static_cast<ResourceErrorCode>(value))
  {
    case ResourceErrorCode::InvalidContent:
      return "Invalid content";
    case ResourceErrorCode::UnsupportedOperation:
      return "Unsupported operation";
    default:
      return "Unrecognized error";
  }
}

constexpr ResourceAdapterErrorCategory g_resourceAdapterErrorCategoryInstance;

}


ResourceError::ResourceError(ResourceErrorCode code, std::string details)
  : error(std::error_code{std::to_underlying(code), g_resourceAdapterErrorCategoryInstance})
{
  if (!details.empty()) {
    this->details = std::move(details);
  }
}

std::string ResourceError::ToString() const
{
  if (!details) {
    return error.message();
  }
  return std::format("{} (details: {})", error.message(), details.value());
}