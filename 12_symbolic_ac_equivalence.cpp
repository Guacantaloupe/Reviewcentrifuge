// ReviewCentrifuge v2: intentionally partial, verbatim source excerpts.
// Private headers and surrounding implementation are NOT included.
// Not independently compilable. Original: src/symbolic.cpp

// ---- original lines 2068-2135 ----
bool sameSymbolicExpression(const Sym& left, const Sym& right) {
    std::set<std::pair<const Node*, const Node*>> active;
    const auto associativeCommutative = [](NK kind) {
        // All of these operations are associative and commutative in the
        // fixed-width bit-vector domain used by the executor.  Treating
        // their trees as multisets is important for obfuscated VM/object
        // addresses: a compiler or mutator may emit `base + index`,
        // `index + base`, or either expression with a different grouping.
        // No arithmetic identities are introduced here; only equivalent
        // reorderings of already-built operands are accepted.
        return kind == NK::Add || kind == NK::Mul || kind == NK::And ||
               kind == NK::Or || kind == NK::Xor;
    };
    std::function<bool(const Sym&, const Sym&)> equal =
        [&](const Sym& a, const Sym& b) -> bool {
        if (a.get() == b.get()) return true;
        if (!a || !b || a->kind != b->kind || a->size != b->size ||
            a->value != b->value || a->index != b->index ||
            a->aux != b->aux || a->pred != b->pred)
            return false;
        const auto key = std::make_pair(a.get(), b.get());
        if (!active.insert(key).second) return true;
        bool result = false;
        if (associativeCommutative(a->kind)) {
            std::vector<Sym> leftTerms;
            std::vector<Sym> rightTerms;
            std::function<void(const Sym&, std::vector<Sym>&)> flatten =
                [&](const Sym& value, std::vector<Sym>& terms) {
                    if (value && value->kind == a->kind &&
                        value->size == a->size) {
                        flatten(value->a, terms);
                        flatten(value->b, terms);
                        return;
                    }
                    terms.push_back(value);
                };
            flatten(a, leftTerms);
            flatten(b, rightTerms);
            if (leftTerms.size() == rightTerms.size()) {
                std::vector<bool> matched(rightTerms.size(), false);
                result = true;
                for (const Sym& term : leftTerms) {
                    bool found = false;
                    for (size_t i = 0; i < rightTerms.size(); ++i) {
                        if (matched[i] || !equal(term, rightTerms[i]))
                            continue;
                        matched[i] = true;
                        found = true;
                        break;
                    }
                    if (!found) {
                        result = false;
                        break;
                    }
                }
            }
        } else {
            result = equal(a->a, b->a) && equal(a->b, b->b) &&
                     equal(a->c, b->c);
        }
        // A failed comparison must not poison a later multiset candidate
        // with a stale active pair.  The trees are acyclic, so this also
        // preserves the cycle guard without weakening failed comparisons.
        active.erase(key);
        return result;
    };
    return equal(left, right);
}

