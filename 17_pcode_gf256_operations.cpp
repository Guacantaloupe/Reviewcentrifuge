// ReviewCentrifuge v2: intentionally partial, verbatim source excerpts.
// Private headers and surrounding implementation are NOT included.
// Not independently compilable. Original: src/pcode.cpp

// ---- original lines 278-306 ----
uint8_t gf256Multiply(uint8_t left, uint8_t right) {
    uint8_t result = 0;
    while (right) {
        if (right & 1U) result ^= left;
        const bool high = (left & 0x80U) != 0;
        left = static_cast<uint8_t>(left << 1);
        if (high) left ^= 0x1bU;
        right = static_cast<uint8_t>(right >> 1);
    }
    return result;
}

uint8_t gf256Inverse(uint8_t value) {
    if (value == 0) return 0;
    uint8_t result = 1;
    uint8_t base = value;
    unsigned exponent = 254;
    while (exponent) {
        if (exponent & 1U) result = gf256Multiply(result, base);
        base = gf256Multiply(base, base);
        exponent >>= 1;
    }
    return result;
}

uint8_t parity8(uint8_t value) {
    value ^= static_cast<uint8_t>(value >> 4);
    value &= 0x0fU;
    return static_cast<uint8_t>((0x6996U >> value) & 1U);

