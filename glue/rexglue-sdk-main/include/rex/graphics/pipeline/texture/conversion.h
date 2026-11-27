#pragma once
/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2014 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 *
 * @modified    Tom Clay, 2026 - Adapted for ReXGlue runtime
 */

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>

#include <rex/assert.h>
#include <rex/graphics/pipeline/texture/info.h>
#include <rex/graphics/xenos.h>

namespace rex::graphics::texture_conversion {

typedef std::function<void(xenos::Endian, void*, const void*, size_t)> CopyBlockCallback;

void CopySwapBlock(xenos::Endian endian, void* output, const void* input, size_t length);
// Copy one visible row of equal-size, power-of-two guest and host blocks.
// offset_at maps an absolute X in the packed mip to a byte offset in its layer.
template <typename OffsetAt>
bool CopySwapBlockRow(xenos::Endian endian, uint8_t* destination, const uint8_t* layer,
                      uint32_t source_x, uint32_t width, uint32_t block_bytes, bool tiled,
                      OffsetAt offset_at) {
  // Xenos 2D and 3D tiles keep at most eight adjacent blocks (16 bytes)
  // contiguous. Packed mips may begin in the middle of one such group.
  const uint32_t tile_group = std::clamp(16u / block_bytes, 1u, 8u);
  const uint32_t swap_bytes = endian == xenos::Endian::kNone ? 1u
                              : endian == xenos::Endian::k8in16 ? 2u : 4u;
  // Keep the scalar loop when neighboring blocks cannot share a copy.
  // CopySwapBlock only swaps complete words within each guest block.
  if ((tiled && tile_group == 1) || block_bytes < swap_bytes) {
    for (uint32_t x = 0; x < width; ++x) {
      const int32_t offset = offset_at(source_x + x);
      if (offset < 0) return false;
      CopySwapBlock(endian, destination + size_t(x) * block_bytes, layer + offset, block_bytes);
    }
    return true;
  }
  while (width) {
    const uint32_t count = tiled ? std::min(width, tile_group - (source_x & (tile_group - 1)))
                                 : width;
    const int32_t offset = offset_at(source_x);
    if (offset < 0 || uint64_t(offset) + uint64_t(count - 1) * block_bytes > INT32_MAX) {
      return false;
    }
    const size_t bytes = size_t(count) * block_bytes;
    CopySwapBlock(endian, destination, layer + offset, bytes);
    destination += bytes;
    source_x += count;
    width -= count;
  }
  return true;
}

void ConvertTexelCTX1ToR8G8(xenos::Endian endian, void* output, const void* input, size_t length);
void ConvertTexelDXNToR8G8(xenos::Endian endian, void* output, const void* input, size_t length);
void ConvertTexelDXT3AToDXT3(xenos::Endian endian, void* output, const void* input, size_t length);
void ConvertTexelDXT5AToR8(xenos::Endian endian, void* output, const void* input, size_t length);
// BC decoding for hosts without textureCompressionBC; `length` is the output row pitch.
void ConvertTexelDXT1ToR8G8B8A8(xenos::Endian endian, void* output, const void* input,
                                size_t length);
void ConvertTexelDXT1ToA1R5G5B5(xenos::Endian endian, void* output, const void* input,
                                size_t length);
void ConvertTexelDXT2_3ToR8G8B8A8(xenos::Endian endian, void* output, const void* input,
                                  size_t length);
void ConvertTexelDXT4_5ToR8G8B8A8(xenos::Endian endian, void* output, const void* input,
                                  size_t length);
void ConvertTexelDXT3AToR8G8B8A8(xenos::Endian endian, void* output, const void* input,
                                 size_t length);

typedef std::function<void(void*, const void*, size_t)> UntileCopyBlockCallback;

typedef struct UntileInfo {
  uint32_t offset_x;
  uint32_t offset_y;
  uint32_t width;
  uint32_t height;
  uint32_t input_pitch;
  uint32_t output_pitch;
  const FormatInfo* input_format_info;
  const FormatInfo* output_format_info;
  UntileCopyBlockCallback copy_callback;
} UntileInfo;

void Untile(uint8_t* output_buffer, const uint8_t* input_buffer, const UntileInfo* untile_info);

}  // namespace rex::graphics::texture_conversion
