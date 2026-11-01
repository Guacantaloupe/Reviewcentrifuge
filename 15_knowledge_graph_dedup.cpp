// ReviewCentrifuge v2: intentionally partial, verbatim source excerpts.
// Private headers and surrounding implementation are NOT included.
// Not independently compilable. Original: src/program_graph.cpp

// ---- original lines 142-196 ----
uint64_t ProgramKnowledgeGraph::addNode(
    KnowledgeNodeKind kind, const std::string& key, const std::string& name,
    uint64_t address, uint64_t size, double confidence) {
    const auto existing = keys_.find(key);
    if (existing != keys_.end()) {
        KnowledgeNode* current = node(existing->second);
        if (current) {
            if (current->name.empty()) current->name = name;
            if (!current->address) current->address = address;
            current->size = std::max(current->size, size);
            current->confidence = std::max(current->confidence, confidence);
        }
        return existing->second;
    }
    KnowledgeNode result;
    result.id = nextNode_++;
    result.kind = kind;
    result.key = key;
    result.name = name;
    result.address = address;
    result.size = size;
    result.confidence = confidence;
    keys_[key] = result.id;
    nodes_.push_back(std::move(result));
    return nodes_.back().id;
}

uint64_t ProgramKnowledgeGraph::addEdge(
    uint64_t from, uint64_t to, KnowledgeEdgeKind kind, double confidence,
    const char* reason) {
    if (!from || !to) return 0;
    const EdgeKey key{from, to, static_cast<uint32_t>(kind)};
    const auto existing = edgeIndex_.find(key);
    if (existing != edgeIndex_.end()) {
        KnowledgeEdge* current = nullptr;
        if (existing->second >= 1 &&
            existing->second <= edges_.size())
            current = &edges_[static_cast<size_t>(existing->second - 1)];
        if (current) {
            current->confidence = std::max(current->confidence, confidence);
            if (!current->reason || !*current->reason) current->reason = reason;
        }
        return existing->second;
    }
    KnowledgeEdge edge;
    edge.id = nextEdge_++;
    edge.from = from;
    edge.to = to;
    edge.kind = kind;
    edge.confidence = confidence;
    edge.reason = reason ? reason : "";
    edges_.push_back(edge);
    edgeIndex_[key] = edges_.back().id;
    return edges_.back().id;
}

