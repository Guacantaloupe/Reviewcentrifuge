// ReviewCentrifuge v2: intentionally partial, verbatim source excerpts.
// Private headers and surrounding implementation are NOT included.
// Not independently compilable. Original: src/cfg.cpp

// ---- original lines 149-159 ----
            // Jump-table hint: an indirect branch whose dispatch is covered by
            // a recovered (statically or symbolically) table branches to every
            // case target.  Enqueue them so the case bodies become regular
            // CFG blocks; addUnique keeps overlap with known targets cheap.
            if (jumpTables) {
                for (const JumpTable& table : *jumpTables) {
                    if (table.dispatchAddress != a) continue;
                    for (uint64_t target : table.targets)
                        enqueue(a, target, true);
                }
            }

