// ReviewCentrifuge — selected *verbatim* excerpts from private source
// Context and private headers intentionally omitted. NOT standalone translation units.
// Source file: src/ir.cpp

// ---- original lines 1603-1627 ----
uint64_t maskFor(int size) {
    return size <= 0 || size >= 8 ? ~0ULL : ((1ULL << (size * 8)) - 1);
}

std::optional<uint64_t> fold(POp op, uint64_t a, uint64_t b, int size) {
    const uint64_t mask = maskFor(size);
    switch (op) {
    case POp::INT_ADD: return (a + b) & mask;
    case POp::INT_SUB: return (a - b) & mask;
    case POp::INT_MULT: return (a * b) & mask;
    case POp::INT_AND: return (a & b) & mask;
    case POp::INT_OR: return (a | b) & mask;
    case POp::INT_XOR: return (a ^ b) & mask;
    case POp::INT_LEFT: return b < 64 ? (a << b) & mask : 0;
    case POp::INT_RIGHT: return b < 64 ? (a & mask) >> b : 0;
    case POp::INT_EQUAL: return (a & mask) == (b & mask);
    case POp::INT_NOTEQUAL: return (a & mask) != (b & mask);
    case POp::INT_LESS: return (a & mask) < (b & mask);
    case POp::INT_LESSEQUAL: return (a & mask) <= (b & mask);
    case POp::BOOL_AND: return (a != 0) && (b != 0);
    case POp::BOOL_OR: return (a != 0) || (b != 0);
    case POp::BOOL_XOR: return (a != 0) != (b != 0);
    default: return std::nullopt;
    }
}
