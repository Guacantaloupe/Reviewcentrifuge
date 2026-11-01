// ReviewCentrifuge — selected *verbatim* excerpts from private source
// Context and private headers intentionally omitted. NOT standalone translation units.
// Source file: src/cfg.cpp

// ---- original lines 742-791 ----
std::set<uint64_t> CfgBuilder::predecessors(uint64_t blockStart) const {
    std::set<uint64_t> p;
    for (const auto& b : blocks_) {
        for (uint64_t s : b.succs)
            if (s == blockStart) p.insert(b.start);
        for (uint64_t s : b.exceptionSuccs)
            if (s == blockStart) p.insert(b.start);
    }
    return p;
}

std::set<uint64_t> CfgBuilder::dominators(uint64_t blockStart) const {
    if (!idx_.count(blockStart) || !idx_.count(entry_)) return {};
    // iterative dataflow: dom(b) = {b} ∪ ⋂ dom(p)
    std::set<uint64_t> all;
    for (const auto& b : blocks_) all.insert(b.start);

    std::map<uint64_t, std::set<uint64_t>> dom;
    for (const auto& b : blocks_) {
        if (b.start == entry_) dom[b.start] = {b.start};
        else dom[b.start] = all;
    }
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& b : blocks_) {
            if (b.start == entry_) continue;
            std::set<uint64_t> preds = predecessors(b.start);
            std::set<uint64_t> nd;
            bool first = true;
            for (uint64_t p : preds) {
                if (first) {
                    nd = dom[p];
                    first = false;
                } else {
                    std::set<uint64_t> inter;
                    for (uint64_t x : nd)
                        if (dom[p].count(x)) inter.insert(x);
                    nd.swap(inter);
                }
            }
            nd.insert(b.start);
            if (nd != dom[b.start]) {
                dom[b.start] = nd;
                changed = true;
            }
        }
    }
    return dom.at(blockStart);
}

// ---- original lines 793-926 ----
void CfgBuilder::findNaturalLoops() {
    loops_.clear();
    std::map<uint64_t, size_t> byHeader;

    // Batch dominator dataflow: computing dominators(block) per tail made
    // this pass O(B^4) (each call re-ran the full iterative fixpoint over
    // every block).  Large 5.x Blender functions (27KB+) blew up to minutes.
    // Run the fixpoint once for the whole graph and look results up.
    // dom(b) = {b} ∪ ⋂_{p in preds(b)} dom(p); entry = {entry}.
    // Precompute the predecessor table once; predecessors() scanned every
    // block per call, adding another O(B) factor to every back-edge walk.
    std::map<uint64_t, std::vector<uint64_t>> preds;
    for (const auto& b : blocks_) {
        for (uint64_t s : b.succs) preds[s].push_back(b.start);
        for (uint64_t s : b.exceptionSuccs) preds[s].push_back(b.start);
    }

    std::map<uint64_t, std::set<uint64_t>> dom;
    if (!blocks_.empty() && idx_.count(entry_)) {
        std::set<uint64_t> all;
        for (const auto& b : blocks_) all.insert(b.start);
        for (const auto& b : blocks_) {
            if (b.start == entry_) dom[b.start] = {b.start};
            else dom[b.start] = all;
        }
        bool changed = true;
        while (changed) {
            changed = false;
            for (const auto& b : blocks_) {
                if (b.start == entry_) continue;
                std::set<uint64_t> nd;
                bool first = true;
                const auto predIt = preds.find(b.start);
                if (predIt != preds.end()) {
                    for (uint64_t p : predIt->second) {
                        const auto dit = dom.find(p);
                        if (dit == dom.end()) continue;
                        if (first) {
                            nd = dit->second;
                            first = false;
                        } else {
                            std::set<uint64_t> inter;
                            for (uint64_t x : nd)
                                if (dit->second.count(x)) inter.insert(x);
                            nd.swap(inter);
                        }
                    }
                }
                nd.insert(b.start);
                if (nd != dom[b.start]) {
                    dom[b.start] = nd;
                    changed = true;
                }
            }
        }
    }

    for (const auto& tail : blocks_) {
        const auto tailDomIt = dom.find(tail.start);
        static const std::set<uint64_t> kEmptyDom;
        const std::set<uint64_t>& tailDom =
            tailDomIt == dom.end() ? kEmptyDom : tailDomIt->second;
        for (uint64_t header : tail.succs) {
            if (!tailDom.count(header)) continue; // not a back edge

            size_t loopIndex = 0;
            auto found = byHeader.find(header);
            if (found == byHeader.end()) {
                loopIndex = loops_.size();
                byHeader.emplace(header, loopIndex);
                NaturalLoop loop;
                loop.header = header;
                loop.blocks.insert(header);
                loops_.push_back(std::move(loop));
            } else {
                loopIndex = found->second;
            }

            NaturalLoop& loop = loops_[loopIndex];
            loop.backEdges.emplace_back(tail.start, header);
            std::vector<uint64_t> worklist;
            if (loop.blocks.insert(tail.start).second && tail.start != header)
                worklist.push_back(tail.start);
            while (!worklist.empty()) {
                const uint64_t block = worklist.back();
                worklist.pop_back();
                const auto predIt = preds.find(block);
                if (predIt == preds.end()) continue;
                for (uint64_t pred : predIt->second) {
                    if (loop.blocks.insert(pred).second && pred != header)
                        worklist.push_back(pred);
                }
            }
        }
    }

    for (auto& loop : loops_) {
        for (uint64_t blockAddr : loop.blocks) {
            const CfgBlock* block = blockAt(blockAddr);
            if (!block) continue;
            for (uint64_t succ : block->succs)
                if (!loop.blocks.count(succ))
                    loop.exits.emplace_back(blockAddr, succ);
        }
        std::sort(loop.backEdges.begin(), loop.backEdges.end());
        loop.backEdges.erase(
            std::unique(loop.backEdges.begin(), loop.backEdges.end()),
            loop.backEdges.end());
        std::sort(loop.exits.begin(), loop.exits.end());
        loop.exits.erase(std::unique(loop.exits.begin(), loop.exits.end()),
                         loop.exits.end());
    }

    // The smallest strict superset is the immediate parent loop.
    for (auto& child : loops_) {
        const NaturalLoop* parent = nullptr;
        for (const auto& candidate : loops_) {
            if (candidate.header == child.header ||
                candidate.blocks.size() <= child.blocks.size())
                continue;
            if (!std::includes(candidate.blocks.begin(), candidate.blocks.end(),
                               child.blocks.begin(), child.blocks.end()))
                continue;
            if (!parent || candidate.blocks.size() < parent->blocks.size())
                parent = &candidate;
        }
        if (parent) child.parentHeader = parent->header;
    }

    std::sort(loops_.begin(), loops_.end(),
              [](const NaturalLoop& a, const NaturalLoop& b) {
                  return a.header < b.header;
              });
}
