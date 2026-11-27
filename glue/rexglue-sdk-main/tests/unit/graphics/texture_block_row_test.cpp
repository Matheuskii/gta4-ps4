#include <catch2/catch_test_macros.hpp>

#include <random>
#include <vector>

#include <rex/graphics/pipeline/texture/conversion.h>
#include <rex/graphics/pipeline/texture/util.h>

namespace {
using namespace rex::graphics;
using xenos::Endian;

TEST_CASE("batched texture rows match single-block copies", "[gta4-native][texture]") {
  std::vector<uint8_t> layer(128 * 64 * 8 * 16);
  std::mt19937 random(37);
  for (auto& byte : layer) byte = uint8_t(random());

  for (uint32_t log2 = 0; log2 <= 4; ++log2) {
    const uint32_t bytes = 1u << log2;
    for (auto endian : {Endian::kNone, Endian::k8in16, Endian::k8in32, Endian::k16in32}) {
      for (int layout = 0; layout < 3; ++layout) {
        for (uint32_t x : {0u, 1u, 6u, 17u, 31u}) {
          for (uint32_t y : {0u, 1u, 8u, 17u, 33u}) {
            for (uint32_t z : {0u, 3u, 5u}) {
              const auto offset = [&](uint32_t sx) -> int32_t {
                if (layout == 1) return texture_util::GetTiledOffset2D(sx, y, 128, log2);
                if (layout == 2) return texture_util::GetTiledOffset3D(sx, y, z, 128, 64, log2);
                return ((z * 64 + y) * 128 + sx) * bytes;
              };
              constexpr uint32_t width = 37;
              std::vector<uint8_t> expected(16 + width * bytes + 16, 0x5a);
              auto actual = expected;
              for (uint32_t i = 0; i < width; ++i) {
                texture_conversion::CopySwapBlock(endian, expected.data() + 16 + i * bytes,
                                                  layer.data() + offset(x + i), bytes);
              }
              REQUIRE(texture_conversion::CopySwapBlockRow(endian, actual.data() + 16,
                          layer.data(), x, width, bytes, layout != 0, offset));
              REQUIRE(actual == expected);
            }
          }
        }
      }
    }
  }
}

TEST_CASE("batched texture rows reject invalid offsets before copying the run",
          "[gta4-native][texture]") {
  uint8_t byte = 0;
  for (int32_t offset : {-1, INT32_MAX - 1}) {
    REQUIRE_FALSE(texture_conversion::CopySwapBlockRow(Endian::kNone, &byte, &byte, 0, 4, 2,
                                                       false, [=](uint32_t) { return offset; }));
  }
}
}  // namespace
