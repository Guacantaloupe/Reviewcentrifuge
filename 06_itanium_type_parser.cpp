// ReviewCentrifuge — selected *verbatim* excerpts from private source
// Context and private headers intentionally omitted. NOT standalone translation units.
// Source file: src/cpp_recovery.cpp

// ---- original lines 23-175 ----
std::string itaniumTypeName(std::string encoding) {
    if (!encoding.empty() && encoding.front() == 'N' && encoding.back() == 'E')
        encoding = encoding.substr(1, encoding.size() - 2);
    std::string result;
    size_t at = 0;
    while (at < encoding.size()) {
        if (!std::isdigit(static_cast<unsigned char>(encoding[at]))) break;
        size_t digits = at;
        while (at < encoding.size() &&
               std::isdigit(static_cast<unsigned char>(encoding[at])))
            ++at;
        size_t length = 0;
        try {
            length = static_cast<size_t>(
                std::stoull(encoding.substr(digits, at - digits)));
        } catch (...) {
            break;
        }
        if (!length || length > encoding.size() - at) break;
        if (!result.empty()) result += "::";
        result += encoding.substr(at, length);
        at += length;
    }
    return result.empty() ? encoding : result;
}

std::string builtinTypeName(char code) {
    switch (code) {
    case 'v': return "void"; case 'b': return "bool";
    case 'c': return "char"; case 'a': return "signed char";
    case 'h': return "unsigned char"; case 's': return "short";
    case 't': return "unsigned short"; case 'i': return "int";
    case 'j': return "unsigned int"; case 'l': return "long";
    case 'm': return "unsigned long"; case 'x': return "long long";
    case 'y': return "unsigned long long"; case 'f': return "float";
    case 'd': return "double"; case 'e': return "long double";
    default: return {};
    }
}

std::string parseItaniumType(const std::string& encoding, size_t& at);

std::string parseItaniumComponent(const std::string& encoding, size_t& at) {
    // Itanium substitution codes are how libstdc++/libc++ keep common
    // namespaces and allocator types compact.  Treat them as components as
    // well as types so nested names such as NSt6vectorIiSaIiEEE are not
    // discarded before template recognition gets a chance to run.
    if (at < encoding.size() && encoding[at] == 'S')
        return parseItaniumType(encoding, at);
    if (at >= encoding.size() ||
        !std::isdigit(static_cast<unsigned char>(encoding[at]))) return {};
    size_t length = 0;
    while (at < encoding.size() &&
           std::isdigit(static_cast<unsigned char>(encoding[at]))) {
        length = length * 10 + static_cast<size_t>(encoding[at] - '0');
        ++at;
    }
    if (!length || length > encoding.size() - at) return {};
    std::string name = encoding.substr(at, length);
    at += length;
    if (at < encoding.size() && encoding[at] == 'I') {
        ++at;
        std::vector<std::string> arguments;
        while (at < encoding.size() && encoding[at] != 'E') {
            const size_t before = at;
            std::string argument = parseItaniumType(encoding, at);
            if (argument.empty() || at == before) break;
            arguments.push_back(std::move(argument));
        }
        if (at < encoding.size() && encoding[at] == 'E') ++at;
        name += "<";
        for (size_t index = 0; index < arguments.size(); ++index) {
            if (index) name += ", ";
            name += arguments[index];
        }
        name += ">";
    }
    return name;
}

std::string parseItaniumType(const std::string& encoding, size_t& at) {
    if (at >= encoding.size()) return {};
    const char code = encoding[at];
    if (const std::string builtin = builtinTypeName(code); !builtin.empty()) {
        ++at;
        return builtin;
    }
    if (code == 'P' || code == 'R' || code == 'O' || code == 'K') {
        ++at;
        std::string nested = parseItaniumType(encoding, at);
        if (code == 'P') return nested + "*";
        if (code == 'R') return nested + "&";
        if (code == 'O') return nested + "&&";
        return "const " + nested;
    }
    if (code == 'N') {
        ++at;
        std::string result;
        while (at < encoding.size() && encoding[at] != 'E') {
            std::string component = parseItaniumComponent(encoding, at);
            if (component.empty()) break;
            if (!result.empty()) result += "::";
            result += component;
        }
        if (at < encoding.size() && encoding[at] == 'E') ++at;
        return result;
    }
    if (code == 'S') {
        if (at + 1 >= encoding.size()) return {};
        const char substitution = encoding[at + 1];
        std::string result;
        if (substitution == 't') {
            at += 2;
            const std::string component = parseItaniumComponent(encoding, at);
            return component.empty() ? std::string{} : "std::" + component;
        }
        switch (substitution) {
        case 'a': result = "std::allocator"; break;
        case 'b': result = "std::basic_string"; break;
        case 's': result = "std::basic_string"; break;
        case 'i': result = "std::basic_istream"; break;
        case 'o': result = "std::basic_ostream"; break;
        case 'd': result = "std::basic_iostream"; break;
        default: return {};
        }
        at += 2;
        if (at < encoding.size() && encoding[at] == 'I') {
            ++at;
            std::vector<std::string> arguments;
            while (at < encoding.size() && encoding[at] != 'E') {
                const size_t before = at;
                std::string argument = parseItaniumType(encoding, at);
                if (argument.empty() || at == before) break;
                arguments.push_back(std::move(argument));
            }
            if (at < encoding.size() && encoding[at] == 'E') ++at;
            result += "<";
            for (size_t index = 0; index < arguments.size(); ++index) {
                if (index) result += ", ";
                result += arguments[index];
            }
            result += ">";
        }
        return result;
    }
    return parseItaniumComponent(encoding, at);
}

std::string demangleItaniumClass(const std::string& encoding) {
    size_t at = 0;
    const std::string parsed = parseItaniumType(encoding, at);
    return parsed.empty() ? itaniumTypeName(encoding) : parsed;
}
