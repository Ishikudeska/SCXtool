#pragma once

#include <array>
#include <cstdint>
#include <span>

namespace engine::crypto {

class Aes128
{
public:
    using Block = std::array<std::uint8_t, 16>;

    explicit Aes128(std::span<const std::uint8_t, 16> key);

    void encryptBlock(const std::uint8_t *in, std::uint8_t *out) const;
    void decryptBlock(const std::uint8_t *in, std::uint8_t *out) const;

private:
    std::array<std::uint8_t, 176> roundKeys_{};
};

void cbcDecrypt(const Aes128 &aes, Aes128::Block &iv, std::span<std::uint8_t> data);
void cbcEncrypt(const Aes128 &aes, Aes128::Block &iv, std::span<std::uint8_t> data);

}
