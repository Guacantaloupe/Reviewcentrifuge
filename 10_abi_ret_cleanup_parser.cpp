// ReviewCentrifuge v2: intentionally partial, verbatim source excerpts.
// Private headers and surrounding implementation are NOT included.
// Not independently compilable. Original: src/calling_convention.cpp

// ---- original lines 128-148 ----
// The Sleigh x86 constructors preserve the immediate on a near/far return in
// PcodeInsn::text (`ret 0x10`, `lret 0x8`). Keep this parser deliberately
// narrow: a missing or malformed operand is unknown evidence, not a guessed
// cleanup amount.
std::optional<uint32_t> returnStackCleanup(const std::string& text) {
    std::istringstream stream(text);
    std::string mnemonic;
    if (!(stream >> mnemonic)) return std::nullopt;
    if (mnemonic != "ret" && mnemonic != "retf" && mnemonic != "lret")
        return std::nullopt;
    std::string operand;
    if (!(stream >> operand)) return uint32_t{0};
    std::string trailing;
    if (stream >> trailing) return std::nullopt;
    if (operand.empty()) return std::nullopt;
    char* end = nullptr;
    const unsigned long value = std::strtoul(operand.c_str(), &end, 0);
    if (end == operand.c_str() || *end != '\0' || value > 0xffffUL)
        return std::nullopt;
    return static_cast<uint32_t>(value);
}

