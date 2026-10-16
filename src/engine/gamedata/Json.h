#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace engine::gamedata::json {

class Writer
{
public:
    void beginObject();
    void endObject();
    void beginArray();
    void endArray();

    void key(std::string_view name);
    void value(std::string_view utf8);
    void value(const char *utf8) { value(std::string_view(utf8)); }
    void value(double v);
    void value(std::int32_t v);
    void null();

    template <class T> void field(std::string_view name, const T &v)
    {
        key(name);
        value(v);
    }
    template <class T> void field(std::string_view name, const std::optional<T> &v)
    {
        if (v) {
            key(name);
            value(*v);
        }
    }

    const std::string &text() const { return out_; }

private:
    void beforeValue();
    void newLine();

    std::string out_;
    std::vector<bool> hasItems_;
    bool afterKey_ = false;
};

struct Member;

struct Value
{
    enum class Type { Null, Bool, Number, String, Array, Object } type = Type::Null;
    bool boolean = false;
    double number = 0;
    std::string numberText;
    std::string string;
    std::vector<Value> array;
    std::vector<Member> object;

    const Value *find(std::string_view name) const;
};

struct Member
{
    std::string name;
    Value value;
};

std::optional<Value> parse(std::string_view text);

}
