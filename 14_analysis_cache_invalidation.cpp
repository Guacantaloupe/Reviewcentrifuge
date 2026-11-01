// ReviewCentrifuge v2: intentionally partial, verbatim source excerpts.
// Private headers and surrounding implementation are NOT included.
// Not independently compilable. Original: src/analysis_cache.cpp

// ---- original lines 201-212 ----
std::string AnalysisCache::analyzerVersion() {
    // Bump on any analysis-pass behavior change.  The IR-feedback round
    // count changes what seedSignatureTypes does inside inferTypes, so it
    // participates in the version.
    const char* rounds = std::getenv("CENTRIFUGE_IR_FEEDBACK_ROUNDS");
    std::string v = "centrifuge-analysis-v18";
    if (rounds) {
        v += ":rounds=";
        v += rounds;
    }
    return v;
}

// ---- original lines 669-693 ----
bool AnalysisCache::load(const std::string& path) {
    records_.clear();
    enabled_ = false;
    path_ = path;
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    std::string data((std::istreambuf_iterator<char>(in)),
                     std::istreambuf_iterator<char>());
    Reader r{data.data(), data.size(), 0};
    uint32_t magic = 0, format = 0;
    if (!r.u32(magic) || !r.u32(format)) return false;
    if (magic != kMagic || format != kFormatVersion) return false;
    std::string version;
    if (!r.str(version) || version != analyzerVersion()) return false;
    uint32_t count = 0;
    if (!r.u32(count) || count > 1U << 24) return false;
    for (uint32_t i = 0; i < count; ++i) {
        uint64_t addr = 0, hash = 0;
        std::string payload;
        if (!r.u64(addr) || !r.u64(hash) || !r.str(payload)) return false;
        records_[addr] = Record{hash, std::move(payload)};
    }
    enabled_ = true;
    hits_ = 0;
    return true;

// ---- original lines 734-746 ----
bool AnalysisCache::lookup(uint64_t addr, uint64_t codeHash,
                           AnalyzedFunction& out) const {
    const auto it = records_.find(addr);
    if (it == records_.end() || it->second.codeHash != codeHash) return false;
    if (!deserializeAnalyzedFunction(it->second.payload, out)) return false;
    out.function.addr = addr;
    return true;
}

void AnalysisCache::store(uint64_t addr, uint64_t codeHash,
                          const AnalyzedFunction& function) {
    records_[addr] = Record{codeHash, serializeAnalyzedFunction(function)};
    dirty_ = true;

