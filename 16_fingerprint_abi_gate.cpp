// ReviewCentrifuge v2: intentionally partial, verbatim source excerpts.
// Private headers and surrounding implementation are NOT included.
// Not independently compilable. Original: src/program_graph.cpp

// ---- original lines 423-450 ----
            // The fingerprint contains entry storage mappings, but retain an
            // explicit ABI/type compatibility gate here. This prevents an
            // incomplete prototype or malformed cache record from turning a
            // hash match into a source-level alias.
            if (representativeAnalysis->callingConvention !=
                    memberAnalysis->callingConvention ||
                representativeAnalysis->signature.parameters.size() !=
                    memberAnalysis->signature.parameters.size() ||
                representativeAnalysis->signature.returnType.kind !=
                    memberAnalysis->signature.returnType.kind)
                continue;
            bool parameterKindsCompatible = true;
            for (size_t parameter = 0;
                 parameter < representativeAnalysis->signature.parameters.size();
                 ++parameter) {
                if (representativeAnalysis->signature.parameters[parameter]
                            .type.kind !=
                    memberAnalysis->signature.parameters[parameter].type.kind) {
                    parameterKindsCompatible = false;
                    break;
                }
            }
            if (!parameterKindsCompatible) continue;
            graph.addEdge(
                representativeNode->second, memberNode->second,
                KnowledgeEdgeKind::SEMANTICALLY_EQUIVALENT, 0.91,
                "complete dual optimized SSA fingerprints and compatible ABI/type signature");
        }

