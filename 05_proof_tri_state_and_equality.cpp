// ReviewCentrifuge — selected *verbatim* excerpts from private source
// Context and private headers intentionally omitted. NOT standalone translation units.
// Source file: src/semantics.cpp

// ---- original lines 2051-2075 ----
Semantics::Truth Semantics::proveCondition(uint32_t cond) {
    if (nodes_[cond].width != 1) return Truth::Unknown;
    std::vector<uint32_t> vars;
    if (!collectAtoms(*this, cond, vars)) return Truth::Unknown;
    std::map<uint32_t, uint32_t> atomBase;
    uint32_t base = 0;
    for (const uint32_t varId : vars) {
        atomBase.emplace(varId, base);
        base += nodes_[varId].width * 8;
    }
    std::map<uint64_t, Anf> memo;
    anfOpBudget_ = 4000000;
    const auto budgetMs = proofTimeBudget();
    proofDeadline_ = budgetMs.count() > 0
        ? std::chrono::steady_clock::now() + budgetMs
        : std::chrono::steady_clock::time_point{};
    proofClockChecks_ = 0;
    Anf result;
    if (!blast(*this, cond, 0, atomBase, memo, anfMonomialBudget, result))
        return Truth::Unknown;
    if (result.monos.empty()) return Truth::AlwaysFalse;
    if (result.monos.size() == 1 && result.monos[0] == 0)
        return Truth::AlwaysTrue;
    return Truth::Unknown;
}

// ---- original lines 2114-2125 ----
bool Semantics::proveEqual(uint32_t a, uint32_t b) {
    if (nodes_[a].width != nodes_[b].width) return false;
    // Cheap canonical-form equality first: the linear-algebra normal form
    // collapses MBA templates without any bit-blasting.
    if (simplify(a) == simplify(b)) return true;
    // Residual proofs (e.g. boolean identities the rewriter misses) go
    // through the ANF bit-blaster; carry-chain blow-up is budget-capped and
    // answers "unknown" (= false here, never a wrong proof).
    const uint32_t diff = binary(Xor, nodes_[a].width, a, b);
    const auto value = proveConstant(diff);
    return value.has_value() && *value == 0;
}
