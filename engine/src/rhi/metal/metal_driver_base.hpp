#pragma once
#include "../sys_rhi.hpp"
#include "engine/common.hpp"
#include "engine/rhi/rhi.hpp"

#include <mutex>
#include <memory>

namespace core::rhi
{
    std::unique_ptr<Device> make_metal3_device(bool debug);
}
