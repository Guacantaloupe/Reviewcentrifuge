// ReviewCentrifuge v2: intentionally partial, verbatim source excerpts.
// Private headers and surrounding implementation are NOT included.
// Not independently compilable. Original: src/highir.cpp

// ---- original lines 362-373 ----
void setConditionEvidence(HighNode& node, const CfgBlock* block,
                          const ProvenDeadEdge* proof = nullptr) {
    if (proof) {
        node.conditionProof = proof->proof;
        node.conditionProofExpression = proof->normalizedExpression;
        node.conditionProofWitnesses = proof->witnessAddresses;
    }
    if (!block || block->succs.size() != 2 || !block->isCondBranch()) return;
    node.condition = branchCondition(*block);
    node.conditionFalseTarget = block->succs[0];
    node.conditionTrueTarget = block->succs[1];
}

