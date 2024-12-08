#include "parser.h"
#include "api.h"

namespace pbrt
{
    Loc* parserLoc;

    static std::string toString(string_view s)
    {
        return std::string(s.data(), s.size());
    }

    static char decodeEscaped(int ch)
    {
        switch (ch) {
        case EOF:
            exit(1);
        case 'b':
            return '\b';
        case 'f':
            return '\f';
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case 't':
            return '\t';
        case '\\':
            return '\\';
        case '\'':
            return '\'';
        case '\"':
            return '\"';
        default:
            exit(1);
        }
        return 0;
    }

    std::unique_ptr<Tokenizer> Tokenizer::CreateFromFile(const std::string& filename)
    {
        FILE* f;
        fopen_s(&f, filename.c_str(), "r");
        if (!f)
        {
            return nullptr;
        }

        std::string str;
        int ch;
        while ((ch = fgetc(f)) != EOF) str.push_back(char(ch));
        fclose(f);

        return std::unique_ptr<Tokenizer>(new Tokenizer(std::move(str)));
    }

    Tokenizer::Tokenizer(std::string str) : loc("<stdin>"), contents(std::move(str))
    {
        pos = contents.data();
        end = pos + contents.size();
    }

    string_view Tokenizer::Next()
    {
        while (true) {
            const char* tokenStart = pos;
            int ch = getChar();
            if (ch == EOF) return {};
            else if (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r') {
                // nothing
            }
            else if (ch == '"') {
                // scan to closing quote
                bool haveEscaped = false;
                while ((ch = getChar()) != '"') {
                    if (ch == EOF || ch == '\n')
                        return {};
                    else if (ch == '\\') {
                        haveEscaped = true;
                        if ((ch = getChar()) == EOF)
                            return {};
                    }
                }

                if (!haveEscaped)
                    return { tokenStart, size_t(pos - tokenStart) };
                else {
                    sEscaped.clear();
                    for (const char* p = tokenStart; p < pos; ++p) {
                        if (*p != '\\')
                            sEscaped.push_back(*p);
                        else {
                            ++p;
                            sEscaped.push_back(decodeEscaped(*p));
                        }
                    }
                    return { sEscaped.data(), sEscaped.size() };
                }
            }
            else if (ch == '[' || ch == ']')
                return { tokenStart, size_t(1) };
            else if (ch == '#') {
                while ((ch = getChar()) != EOF) {
                    if (ch == '\n' || ch == '\r') {
                        ungetChar();
                        break;
                    }
                }

                return { tokenStart, size_t(pos - tokenStart) };
            }
            else {
                while ((ch = getChar()) != EOF) {
                    if (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r' ||
                        ch == '"' || ch == '[' || ch == ']') {
                        ungetChar();
                        break;
                    }
                }
                return { tokenStart, size_t(pos - tokenStart) };
            }
        }
    }

    constexpr int TokenOptional = 0;
    constexpr int TokenRequired = 1;

    static void parse(std::unique_ptr<Tokenizer> t)
    {
        std::vector<std::unique_ptr<Tokenizer>> fileStack;
        fileStack.push_back(std::move(t));
        parserLoc = &fileStack.back()->loc;

        bool ungetTokenSet = false;
        std::string ungetTokenValue;

        std::function<string_view(int)> nextToken;
        nextToken = [&](int flags) -> string_view {
            if (ungetTokenSet) {
                ungetTokenSet = false;
                return string_view(ungetTokenValue.data(), ungetTokenValue.size());
            }

            if (fileStack.empty()) {
                if (flags & TokenRequired)
                    exit(1);
                parserLoc = nullptr;
                return {};
            }

            string_view tok = fileStack.back()->Next();

            if (tok.empty()) {
                fileStack.pop_back();
                if (!fileStack.empty()) parserLoc = &fileStack.back()->loc;
                return nextToken(flags);
            }
            else if (tok[0] == '#')
                return nextToken(flags);
            else
                return tok;
        };

        auto ungetToken = [&](string_view s) {
            ungetTokenValue = std::string(s.data(), s.size());
            ungetTokenSet = true;
        };

        auto syntaxError = [&](string_view tok) {
            exit(1);
        };

        while (true) {
            string_view tok = nextToken(TokenOptional);
            if (tok.empty()) break;

            switch (tok[0]) {
            case 'W':
                if (tok == "WorldBegin")
                    pbrtWorldBegin();
                else if (tok == "WorldEnd")
                    pbrtWorldEnd();
                else
                    syntaxError(tok);
                break;

            default:
                syntaxError(tok);
            }
        }
    }

    void pbrtParseFile(std::string filename)
    {
        std::unique_ptr<Tokenizer> t = Tokenizer::CreateFromFile(filename);
        if (!t) return;
        parse(std::move(t));
    }
}