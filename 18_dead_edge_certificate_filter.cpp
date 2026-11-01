// ReviewCentrifuge v2: intentionally partial, verbatim source excerpts.
// Private headers and surrounding implementation are NOT included.
// Not independently compilable. Original: src/project_recovery.cpp

// ---- original lines 298-317 ----
std::vector<ProvenDeadEdge> provenDeadEdgesForProject(
    const ObfuscationAuditReport& audit) {
    std::vector<ProvenDeadEdge> edges;
    edges.reserve(audit.bogusControlFlow.size());
    for (const BogusControlFlowEvidence& evidence : audit.bogusControlFlow) {
        if (!evidence.proven || !evidence.keptSuccessor ||
            !evidence.deadSuccessor)
            continue;
        ProvenDeadEdge edge;
        edge.instructionAddress = evidence.address;
        edge.keptSuccessor = evidence.keptSuccessor;
        edge.deadSuccessor = evidence.deadSuccessor;
        edge.proof = evidence.certificate.method.empty()
            ? evidence.reason : evidence.certificate.method;
        edge.normalizedExpression = evidence.certificate.normalizedExpression;
        edge.witnessAddresses = evidence.certificate.witnessAddresses;
        edges.push_back(std::move(edge));
    }
    return edges;
}

