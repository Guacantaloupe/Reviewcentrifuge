// ReviewCentrifuge v2: intentionally partial, verbatim source excerpts.
// Private headers and surrounding implementation are NOT included.
// Not independently compilable. Original: src/stack_recovery.cpp

// ---- original lines 470-507 ----
void StackFrameAnalysis::detectOverlaps() {
    std::vector<size_t> locals;
    for (size_t i = 0; i < model_.slots.size(); ++i)
        if (model_.slots[i].role == StackSlotRole::LOCAL)
            locals.push_back(i);
    for (size_t i = 0; i < locals.size(); ++i) {
        for (size_t j = i + 1; j < locals.size(); ++j) {
            const StackSlot& a = model_.slots[locals[i]];
            const StackSlot& b = model_.slots[locals[j]];
            if (a.offset == b.offset) continue; // same slot, not overlap
            const int64_t aEnd = a.offset + (a.size > 0 ? a.size : 1);
            const int64_t bEnd = b.offset + (b.size > 0 ? b.size : 1);
            if (a.offset < bEnd && b.offset < aEnd) {
                model_.slots[locals[i]].overlaps = true;
                model_.slots[locals[i]].overlapWith = b.offset;
                model_.slots[locals[j]].overlaps = true;
                model_.slots[locals[j]].overlapWith = a.offset;
            }
        }
    }
}

void StackFrameAnalysis::promoteVariables() {
    for (StackSlot& slot : model_.slots) {
        if (slot.role != StackSlotRole::LOCAL) continue;
        if (slot.addressTaken || slot.overlaps) continue;
        if (slot.widths.size() != 1) continue; // mixed-width: keep fallback
        const int size = *slot.widths.begin();
        if (size < 1 || size > 8 || (size & (size - 1)) != 0) continue;
        const std::string type = integerTypeName(size);
        if (type.empty()) continue;
        slot.promoted = true;
        slot.variableName = stackSlotName(slot.offset);
        slot.typeName = type;
        slot.size = size;
        ++model_.promotedCount;
    }
}

