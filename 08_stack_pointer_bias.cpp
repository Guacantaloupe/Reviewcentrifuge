// ReviewCentrifuge v2: intentionally partial, verbatim source excerpts.
// Private headers and surrounding implementation are NOT included.
// Not independently compilable. Original: src/stack_recovery.cpp

// ---- original lines 55-109 ----
// Only uniques that are copied back into sp (or write sp directly) move the
// stack pointer; uniques that merely compute sp+const addressing do not.
// spUniq maps every sp-carrying unique to its entry-relative bias; biasOut
// is the bias after the instruction.
void processSpOps(const PcodeInsn& insn, uint64_t spOffset, int64_t biasIn,
                  int64_t& biasOut,
                  std::map<uint64_t, int64_t>& spUniq) {
    biasOut = biasIn;
    uint64_t spRegId = 0;
    for (const auto& kv : insn.varnodes)
        if (kv.second.kind == Varnode::REGISTER &&
            kv.second.offset == spOffset) {
            spRegId = kv.first;
            break;
        }
    if (!spRegId) return;
    // uniques copied back into the sp register are sp moves
    std::set<uint64_t> copiedToSp;
    for (const PcodeOp& op : insn.ops) {
        if (op.op != POp::COPY || op.out != spRegId || !op.in0) continue;
        const Varnode* v = insn.find(op.in0);
        if (v && v->kind == Varnode::UNIQUE) copiedToSp.insert(op.in0);
    }
    auto spBias = [&](uint64_t id) -> int64_t* {
        if (id == spRegId) return &biasOut;
        const auto found = spUniq.find(id);
        return found == spUniq.end() ? nullptr : &found->second;
    };
    for (const PcodeOp& op : insn.ops) {
        if (!op.out) continue;
        const Varnode* out = insn.find(op.out);
        const bool uniqueOut = out && out->kind == Varnode::UNIQUE;
        if (op.op == POp::INT_SUB || op.op == POp::INT_ADD) {
            int64_t* base = spBias(op.in0);
            if (!base) continue;
            const Varnode* c = insn.find(op.in1);
            if (!c || c->kind != Varnode::CONST) continue;
            const int64_t k = static_cast<int64_t>(c->offset);
            const int64_t newBias =
                *base + (op.op == POp::INT_SUB ? -k : k);
            if (uniqueOut) {
                if (!copiedToSp.count(op.out)) continue; // pure addressing
                spUniq[op.out] = newBias;
                biasOut = newBias;
            } else {
                biasOut = newBias; // direct sp register write
            }
            continue;
        }
        if (op.op == POp::COPY && op.out == spRegId) {
            const auto found = spUniq.find(op.in0);
            if (found != spUniq.end()) biasOut = found->second;
        }
    }
}

