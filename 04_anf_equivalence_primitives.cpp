// ReviewCentrifuge — selected *verbatim* excerpts from private source
// Context and private headers intentionally omitted. NOT standalone translation units.
// Source file: src/semantics.cpp

// ---- original lines 1569-1627 ----
bool Semantics::anfOpXor(Anf& a, const Anf& b, size_t budget) {
    const size_t work = a.monos.size() + b.monos.size();
    if (work > anfOpBudget_) return false;
    anfOpBudget_ -= work;
    std::vector<uint64_t> out;
    out.reserve(a.monos.size() + b.monos.size());
    size_t i = 0, j = 0;
    while (i < a.monos.size() || j < b.monos.size()) {
        if (j >= b.monos.size() ||
            (i < a.monos.size() && a.monos[i] < b.monos[j]))
            out.push_back(a.monos[i++]);
        else if (i >= a.monos.size() || b.monos[j] < a.monos[i])
            out.push_back(b.monos[j++]);
        else {
            ++i;
            ++j;  // cancel
        }
    }
    if (out.size() > budget) return false;
    a.monos = std::move(out);
    return true;
}

bool Semantics::anfOpAnd(Anf& a, const Anf& b, size_t budget) {
    // GF(2) polynomial product.  Collect every pairwise monomial union,
    // sort, and cancel even multiplicities (adjacent duplicates toggle).
    // A hard pair cap keeps the bailout itself cheap: a result anywhere near
    // the budget would be unusable by the carry chains anyway.
    const size_t pairs = a.monos.size() * b.monos.size();
    if (pairs > budget * 4) return false;
    if (pairs > anfOpBudget_) return false;
    anfOpBudget_ -= pairs;
    std::vector<uint64_t> prods;
    prods.reserve(pairs);
    for (const uint64_t ma : a.monos)
        for (const uint64_t mb : b.monos) prods.push_back(ma | mb);
    std::sort(prods.begin(), prods.end());
    std::vector<uint64_t> out;
    out.reserve(prods.size());
    for (size_t i = 0; i < prods.size();) {
        size_t j = i + 1;
        while (j < prods.size() && prods[j] == prods[i]) ++j;
        if ((j - i) & 1) out.push_back(prods[i]);
        i = j;
    }
    if (out.size() > budget) return false;
    a.monos = std::move(out);
    return true;
}

bool Semantics::anfOpOr(Anf& a, const Anf& b, size_t budget) {
    // a | b = a ^ b ^ (a & b)
    Anf ab = a;
    if (!anfOpAnd(ab, b, budget)) return false;
    if (!anfOpXor(ab, b, budget)) return false;
    if (!anfOpXor(ab, a, budget)) return false;
    a = std::move(ab);
    return true;
}
