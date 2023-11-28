#include "clang-c/CXString.h"
#include "clang-c/Index.h"
#include "Log.h"
#include <filesystem>
#include <optional>
#include <string>
#include <regex>
#include <assert.h>
#include <sstream>
#include <vector>
#define INPUT_DIR(x) "../input/" x
#define OUTPUT_DIR(x) "../output/" x




std::vector<CXCursor> GetChildren(CXCursor node);
bool CheckPath();
CXTranslationUnit ParseTranslationUnit(CXIndex index,const std::string& path);
std::vector<std::string> ParseTargetName(const std::string& targetName);
std::string MoveClangString(CXString clangStr);
std::string GetDisplayName(CXCursor node);
std::optional<CXCursor> SearchTarget(CXCursor node,const std::string& targetName);
std::string GenIndent(size_t indent)
{
    static const std::string s = "    ";
    std::stringstream ss;
    for (size_t i = 0;i < indent;i++)
    {
        ss << s;
    }
    return ss.str();
}
void PrintNode(size_t indent, CXCursor node);
void PrintTree(size_t indent, CXCursor node);
std::vector<std::string> ScanTypeSpelling(const std::string& s)
{
    std::regex re("[A-Za-z_][A-Za-z_0-9]*|::|<|>|,");
    std::sregex_token_iterator iter(s.begin(), s.end(), re);
    std::sregex_token_iterator end;
    std::vector<std::string> tokens;
    while (iter != end)
    {
        tokens.push_back(*iter);
        iter++;
    }
    return tokens;
}
class TypeSpellingParser
{
public:
    struct Type
    {
        Type(const std::string& _name)
            :name(_name) {}
        Type() = default;
        Type(Type&&) = default;
        ~Type(){}
        std::string name="";
        std::unique_ptr<Type> elementType;
        bool IsArray() { return name.compare("std::vector") == 0; }
        bool IsMap() { return name.compare("std::unordererd_map") == 0; }
    };
    static std::string GetPrefix(const std::vector<std::string>& tokens,size_t& pos,size_t end)
    {
        
        std::stringstream ss;
        while(true)
        {
            ss<<tokens[pos];
            ss<<tokens[pos+1];
            if(pos+3<end)
            {
                if(tokens[pos+3].compare("::")==0)
                {
                    pos+=2;
                }
                else {
                    pos+=2;
                    return ss.str();
                }
            }
            else {
                pos+=2;
                return ss.str();
            }
            
        }
        
    }
    
    static std::unique_ptr<Type> Tokens2Type(const std::vector<std::string>& tokens,size_t begin,size_t end,const std::string prefix)
    {
        size_t pos=begin;
        if (pos + 1 == end)
        {
            return std::make_unique<Type>( tokens[pos]);
        }
        if(tokens[begin+1].compare("::")==0)
        {
            auto prefix=GetPrefix(tokens, pos,end);
            if(pos+1==end)
            {
                return std::make_unique<Type>(prefix+tokens[pos]);
            }
            auto type = Tokens2Type(tokens, pos, end,prefix);
            type->name = prefix + type->name;
            if (type->name.compare("std::vector") == 0)
            {
                type->name = "Array";
            }
            if (type->name.compare("std::unordered_map") == 0)
            {
                type->name = "Map";
            }
            return type;
        }
        if (tokens[begin + 1].compare("<") == 0)
        {
            assert(tokens[end - 1].compare(">") == 0);
            std::string t = prefix + tokens[begin];
            if (t.compare("std::unordered_map")==0)
            {
                auto type = std::make_unique<Type>(tokens[begin]);
                size_t pos = begin;
                while (tokens[pos].compare(",") != 0)pos++;
                type->elementType = Tokens2Type(tokens, pos+1, end - 1, "");
                return type;
            }
            auto type=std::make_unique<Type>(tokens[begin]);
            type->elementType = Tokens2Type(tokens, begin + 2, end - 1,"");
            return type;
        }

        assert(false);
        
    }
    //invalid input will make program crash
    //valid input: A B A::B std::vector<A> std::vector<std::vector<A>>
    static std::unique_ptr<Type> Parse(const std::string& typeSpelling)
    {
        auto tokens = ScanTypeSpelling(typeSpelling);
        return Tokens2Type(tokens, 0, tokens.size(),"");
    }
private:
    static bool IsPunctuation(const std::string& token)
    {
        if (token.compare("::") == 0)
        {
            return true;
        }
        if (token.compare("<") == 0)
        {
            return true;
        }
        if (token.compare(">") == 0)
        {
            return true;
        }
        return false;
    }
    static bool IsIdentifier(const std::string& token)
    {
        std::regex re("[A-Za-z_][A-Za-z0-9_]*");
        return std::regex_match(token, re);
    }
    
};


int main()
{
    {
        auto type = TypeSpellingParser::Parse("A::B::C");
        Log::Debug("name: {}", type->name);
    }
    {
        auto type = TypeSpellingParser::Parse("A");
        Log::Debug("name: {}", type->name);
    }
    {
        auto type = TypeSpellingParser::Parse("std::vector<A>");
        Log::Debug("name: {} elementName: {}", type->name,type->elementType->name);
    }
    {
        auto type = TypeSpellingParser::Parse("std::vector<std::vector<A>>");
        Log::Debug("name: {} elementName: {}", type->name, type->elementType->name);
    }
    {
        auto type = TypeSpellingParser::Parse("std::vector<std::vector<A::B>>");
        Log::Debug("name: {} elementName: {} element.elementName: {}", type->name, type->elementType->name, type->elementType->elementType->name);
    }
    {
        auto type = TypeSpellingParser::Parse("std::vector<std::vector<A::B>>");
        Log::Debug("name: {} elementName: {} element.elementName: {}", type->name, type->elementType->name, type->elementType->elementType->name);
    }
    {
        auto type = TypeSpellingParser::Parse("std::unordered_map<std::string,std::vector<A::B>>");
        Log::Debug("name: {} elementName: {} element.elementName: {}", type->name, type->elementType->name, type->elementType->elementType->name);
    }
   
    //std::string inputPath=INPUT_DIR("a.h");
    //std::string targetName="A";
    //CXIndex index;
    //CXTranslationUnit unit;
    //
    ///*parse file*/
    //if(!CheckPath())exit(-1);
    //index = clang_createIndex(0, 0);
	//unit = ParseTranslationUnit(index,inputPath);
    //if(unit==nullptr)
    //{
    //    Log::Error("failed to parse translation unit,path {}", inputPath);
    //    exit(-1);
    //}
    ///*search target*/
    //auto cursor=clang_getTranslationUnitCursor(unit);
    //auto opt_target=SearchTarget(cursor, targetName);
    //assert(opt_target);
    ///*print target*/
    //PrintTree(0, opt_target.value());
   
}
CXTranslationUnit ParseTranslationUnit(CXIndex index,const std::string& path)
{
    static const char* command_line_args[] = { "-x", "c++", 0 };
    return clang_parseTranslationUnit(
    index, path.c_str(), command_line_args,
    2, nullptr, 0,CXTranslationUnit_None);
}
bool CheckPath()
{
    if(!std::filesystem::exists(INPUT_DIR("")))
    {
        Log::Error("failed to find input dir,path: {}", INPUT_DIR(""));
        return false;
    }
    if(!std::filesystem::exists(OUTPUT_DIR("")))
    {
        Log::Error("failed to find output dir,path: {}", OUTPUT_DIR(""));
        return false;
    }
    return true;
}

std::vector<CXCursor> GetChildren(CXCursor node)
{
    std::vector<CXCursor> children;
    clang_visitChildren(node , [](CXCursor cur,CXCursor parent,CXClientData userdata)
    {
        (*((std::vector<CXCursor>*)userdata)).push_back(cur);
        return CXChildVisit_Continue;
    }, &children);
    return children;
}
std::vector<std::string> ParseTargetName(const std::string& targetName)
{
    std::regex re("[A-Za-z_][A-Za-z_0-9]*");
    std::sregex_token_iterator iter(targetName.begin(),targetName.end(),re);
    std::sregex_token_iterator end;
    std::vector<std::string> trace;
    while(iter!=end)
    {
        trace.push_back(*iter);
        iter++;
    }
    return trace;
}
std::string MoveClangString(CXString clangStr)
{
    std::string str=clang_getCString(clangStr);
    clang_disposeString(clangStr);
    return str;
}
std::string GetDisplayName(CXCursor node)
{
    return MoveClangString(clang_getCursorDisplayName(node));
}
std::optional<CXCursor> SearchTarget(CXCursor node,const std::string& targetName)
{
    auto trace=ParseTargetName(targetName);
    std::vector<CXCursor> curNodes;
    std::vector<CXCursor> nextNodes;
    curNodes.push_back(node);
    for(auto& name:trace)
    {
        for(auto& cur:curNodes)
        {
            auto children = GetChildren(cur);
            for (auto& child : children)
            {
                auto displayName = GetDisplayName(child);
                
                if (displayName.compare(name) == 0)
                {
                    nextNodes.push_back(child);
                }
            }
        }
        curNodes.clear();
        std::swap(curNodes, nextNodes);
    }
    if(curNodes.size()==0)
    {
        DEBUG_LOG_ERROR("name not find");
        return std::nullopt;
    }
    if(curNodes.size()>1)
    {
        DEBUG_LOG_ERROR("muti node find");
        return std::nullopt;
    }
    return curNodes[0];
}
void PrintNode(size_t indent, CXCursor node)
{
    std::string spelling = GetDisplayName(node);
    CXCursorKind kind = clang_getCursorKind(node);
    auto kindSpelling=MoveClangString(clang_getCursorKindSpelling(kind));
    auto type = clang_getCursorType(node);
    auto typeSpelling = MoveClangString(clang_getTypeSpelling(type));
    Log::Debug("{}spelling: {}", GenIndent(indent), spelling);
    Log::Debug("{}kind: {}", GenIndent(indent), kindSpelling);
    Log::Debug("{}type: {}", GenIndent(indent), typeSpelling);
    auto typeSpellingTokens = ScanTypeSpelling(typeSpelling);
    std::stringstream ss;
    for (auto& token : typeSpellingTokens)
    {
        ss << token << " " ;
    }
    Log::Debug("{}type spelling tokens: {}", GenIndent(indent), ss.str());
    
}
void PrintTree(size_t indent, CXCursor node)
{
    PrintNode(indent, node);
    Log::Debug("{}", GenIndent(indent));
    auto children = GetChildren(node);
    for (auto& child : children)
    {
        PrintTree(indent + 1, child);
    }
}