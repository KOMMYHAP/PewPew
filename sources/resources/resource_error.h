#pragma once
#include <cstdint>
#include <optional>

enum class ResourceErrorCode : std::int32_t
{
  InvalidContent,
  TypeAlreadyRegistered,
  UnsupportedOperation
};


struct ResourceError
{
  explicit ResourceError(ResourceErrorCode code, std::string details = {});

  std::string ToString() const;

  std::error_code error;
  std::optional<std::string> details;
};