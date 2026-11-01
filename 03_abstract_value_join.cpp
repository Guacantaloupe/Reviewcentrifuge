// ReviewCentrifuge — selected *verbatim* excerpts from private source
// Context and private headers intentionally omitted. NOT standalone translation units.
// Source file: src/midir.cpp

// ---- original lines 501-528 ----
ValueSet joinValueSet(const ValueSet& a, const ValueSet& b) {
    if (a.isUnknown() || b.isUnknown()) return ValueSet{};
    if (a.kind == ValueSetKind::CONSTANT && b.kind == ValueSetKind::CONSTANT) {
        if (a.constant == b.constant) return a;
        ValueSet set;
        set.kind = ValueSetKind::SET;
        set.values = {a.constant, b.constant};
        return set;
    }
    if (a.kind == ValueSetKind::INTERVAL && b.kind == ValueSetKind::INTERVAL) {
        ValueSet set;
        set.kind = ValueSetKind::INTERVAL;
        set.lo = std::min(a.lo, b.lo);
        set.hi = std::max(a.hi, b.hi);
        return set;
    }
    if (a.kind == ValueSetKind::SET && b.kind == ValueSetKind::SET) {
        ValueSet set;
        set.kind = ValueSetKind::SET;
        set.values = a.values;
        for (uint64_t v : b.values)
            if (std::find(set.values.begin(), set.values.end(), v) == set.values.end())
                set.values.push_back(v);
        if (set.values.size() > 8) return ValueSet{};
        return set;
    }
    return ValueSet{};
}
