#pragma once
#include "engine/common.hpp"
#include <format>
#include <spdlog/spdlog.h>

inline auto rhi_logger = spdlog::default_logger()->clone("RHI");

constexpr u8 MAX_SWAPCHAIN_FRAMES = 3;
constexpr u32 MAX_SAMPLER_DESCRIPTORS = 256;
constexpr u32 MAX_SAMPLED_IMAGE_DESCRIPTORS  = 125000;
constexpr u32 MAX_STORAGE_IMAGE_DESCRIPTORS  = 125000;
constexpr u32 MAX_STORAGE_BUFFER_DESCRIPTORS = 125000;
constexpr u32 MAX_UNIFORM_BUFFER_DESCRIPTORS = 125000;

#define RHI_INFO(...) rhi_logger->info(__VA_ARGS__);
#define RHI_WARN(...) rhi_logger->warn(__VA_ARGS__);
#define RHI_ERROR(...) rhi_logger->error(__VA_ARGS__);
#define RHI_DEBUG(...) rhi_logger->debug(__VA_ARGS__);
#define RHI_CRITICAL(...) rhi_logger->critical(__VA_ARGS__);
