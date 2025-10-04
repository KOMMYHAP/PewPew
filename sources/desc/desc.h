#pragma once

enum class DescTypeId : uint32_t
{
  Int32,
  String,
  Array,
  Object,

  Invalid = std::numeric_limits<uint32_t>::max()
};

enum class DescFieldId : uint32_t
{
  Invalid = std::numeric_limits<uint32_t>::max()
};