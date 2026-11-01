// ReviewCentrifuge — selected *verbatim* excerpts from private source
// Context and private headers intentionally omitted. NOT standalone translation units.
// Source file: src/midir.cpp

// ---- original lines 106-157 ----
MidIRVerification MidIR::verify() const {
    MidIRVerification result;
    std::set<uint64_t> blockAddresses;
    std::set<MidValueId> definitions;
    for (const auto& entry : values_) {
        if (!entry.first || entry.second.id != entry.first)
            result.errors.push_back("value table contains an invalid or mismatched id");
    }
    for (const MidBlock& block : blocks_) {
        if (!blockAddresses.insert(block.start).second)
            result.errors.push_back("duplicate MidIR block at " +
                                    std::to_string(block.start));
        auto check = [&](const MidInstruction& operation, bool expectPhi) {
            if (operation.phi != expectPhi)
                result.errors.push_back("phi placement mismatch at " +
                                        std::to_string(operation.address));
            if (operation.output) {
                if (!value(operation.output))
                    result.errors.push_back("operation defines a missing MidIR value");
                if (!definitions.insert(operation.output).second)
                    result.errors.push_back("MidIR value has multiple definitions");
            }
            for (MidValueId input : operation.inputs)
                if (input && !value(input))
                    result.errors.push_back("operation uses a missing MidIR value");
            if (expectPhi && operation.inputs.size() != block.predecessors.size())
                result.errors.push_back("phi input count does not match predecessor count");
            if ((operation.op == POp::LOAD || operation.op == POp::STORE ||
                 operation.op == POp::CALL || operation.op == POp::CALLIND ||
                 operation.op == POp::SYSCALL) &&
                !memoryPartitions_.count(operation.memoryPartition))
                result.errors.push_back("memory operation references a missing partition");
            if ((operation.op == POp::STORE || operation.op == POp::CALL ||
                 operation.op == POp::CALLIND || operation.op == POp::SYSCALL) &&
                !operation.memoryVersionOut)
                result.errors.push_back("memory-writing operation has no MemorySSA version");
        };
        for (const auto& memoryPhi : block.memoryPhis)
            if (!memoryPartitions_.count(memoryPhi.first) || !memoryPhi.second)
                result.errors.push_back("invalid partitioned MemorySSA phi");
        for (const MidInstruction& phi : block.phis) check(phi, true);
        for (const MidInstruction& operation : block.ops) check(operation, false);
    }
    for (const auto& entry : values_) {
        const MidValue& valueEntry = entry.second;
        const bool entryValue = valueEntry.storage == MidValue::CONSTANT ||
                                valueEntry.version == 0;
        if (!entryValue && !definitions.count(entry.first))
            result.errors.push_back("non-entry MidIR value has no definition");
    }
    return result;
}
