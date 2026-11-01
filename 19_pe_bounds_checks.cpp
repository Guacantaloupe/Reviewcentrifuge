// ReviewCentrifuge v2: intentionally partial, verbatim source excerpts.
// Private headers and surrounding implementation are NOT included.
// Not independently compilable. Original: src/loader_pe.cpp

// ---- original lines 53-68 ----
bool rangeInFile(uint64_t off, uint64_t size, size_t fileSize) {
    return off <= fileSize && size <= static_cast<uint64_t>(fileSize) - off;
}

bool addOk(uint64_t a, uint64_t b, uint64_t& out) {
    if (b > std::numeric_limits<uint64_t>::max() - a) return false;
    out = a + b;
    return true;
}

template <typename T>
bool rd(const std::vector<uint8_t>& d, size_t off, T& out) {
    if (off > d.size() || sizeof(T) > d.size() - off) return false;
    std::memcpy(&out, d.data() + off, sizeof(T));
    return true;
}

