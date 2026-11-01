// ReviewCentrifuge v2: intentionally partial, verbatim source excerpts.
// Private headers and surrounding implementation are NOT included.
// Not independently compilable. Original: src/calling_convention.cpp

// ---- original lines 735-758 ----
        // Parameter/register agreement is the strongest evidence for a
        // detached raw function: the alternative convention simply cannot
        // name a value that the body reads in the other register bank.
        hypothesis.score += 2.0 * registerParameters;
        if (registerParameters) {
            hypothesis.evidence.push_back(
                std::to_string(registerParameters) +
                " live register parameter(s) match ABI order");
        }
        // A body that only consumes ABI positions 2 and 3 is more naturally
        // a four-register SysV tail than a Win64 two-argument function.  The
        // prefix term makes argument *order* part of the evidence rather than
        // treating every known register as an equally strong match.
        size_t prefix = 0;
        while (matchedRegisterPositions.count(prefix)) ++prefix;
        if (prefix) {
            hypothesis.score += 2.0 * prefix;
            hypothesis.evidence.push_back(
                std::to_string(prefix) + " contiguous leading ABI argument(s)");
        } else if (!matchedRegisterPositions.empty()) {
            hypothesis.score -=
                2.0 * static_cast<double>(*matchedRegisterPositions.begin());
            hypothesis.evidence.push_back("leading ABI argument gap");
        }

// ---- original lines 781-810 ----
        if (isX86_32Architecture(architecture) &&
            facts.calleeStackCleanupBytes) {
            const uint32_t cleanup = *facts.calleeStackCleanupBytes;
            if (cleanup != 0) {
                // A nonzero immediate is direct evidence that the callee
                // owns the outgoing stack area; cdecl is therefore a strong
                // negative.  The register/stack evidence above separates
                // stdcall from fastcall/thiscall when possible.
                if (candidate == "cdecl") {
                    hypothesis.score -= 6.0;
                    hypothesis.evidence.push_back(
                        "nonzero ret cleanup contradicts cdecl");
                } else {
                    hypothesis.score += 6.0;
                    hypothesis.evidence.push_back(
                        "nonzero ret cleanup matches callee-popped ABI");
                }
                if (candidate == "stdcall" && !facts.fixedRegisterArguments)
                    hypothesis.score += 2.0;
            } else {
                // A plain ret is the strongest easy distinction available for
                // a stack-only body: it rules in cdecl while leaving a
                // register-only fastcall/thiscall body viable.
                if (candidate == "cdecl") {
                    hypothesis.score += 3.0;
                    hypothesis.evidence.push_back("plain ret matches cdecl");
                } else if (!facts.fixedRegisterArguments) {
                    hypothesis.score -= 1.0;
                }
            }

