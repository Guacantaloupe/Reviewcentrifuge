// ReviewCentrifuge v2: intentionally partial, verbatim source excerpts.
// Private headers and surrounding implementation are NOT included.
// Not independently compilable. Original: src/symbolic.cpp

// ---- original lines 1207-1219 ----
// Unsigned interval of an expression; wraps clamp to full range (sound).
// Portable 65-bit arithmetic helpers (MSVC has no __uint128_t).
// addWide returns the carry out; a+b may need 65 bits.
inline uint64_t addWide(uint64_t a, uint64_t b, uint64_t& out) {
    out = a + b;
    return out < a ? 1 : 0;
}
// mulFits: true when a*b fits in 64 bits.
bool mulFits(uint64_t a, uint64_t b, uint64_t& out) {
    if (a != 0 && b > ~0ULL / a) return false;
    out = a * b;
    return true;
}

