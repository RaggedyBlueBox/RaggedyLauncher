#include "raggedy/core/json.hpp"

#include <array>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <system_error>
#include <utility>

namespace raggedy::core
{
    JsonValue JsonValue::null_value()
    {
        return JsonValue{};
    }

    JsonValue JsonValue::from_bool(bool value)
    {
        JsonValue result;
        result.type = JsonType::boolean;
        result.boolean = value;
        return result;
    }

    JsonValue JsonValue::from_number(double value)
    {
        JsonValue result;
        result.type = JsonType::number;
        result.number = value;
        return result;
    }

    JsonValue JsonValue::from_string(std::string value)
    {
        JsonValue result;
        result.type = JsonType::string;
        result.string = std::move(value);
        return result;
    }

    JsonValue JsonValue::from_array(std::vector<JsonValue> values)
    {
        JsonValue result;
        result.type = JsonType::array;
        result.array = std::move(values);
        return result;
    }

    JsonValue JsonValue::from_object(std::map<std::string, JsonValue> values)
    {
        JsonValue result;
        result.type = JsonType::object;
        result.object = std::move(values);
        return result;
    }

    bool JsonValue::as_bool(bool fallback) const
    {
        return type == JsonType::boolean ? boolean : fallback;
    }

    double JsonValue::as_number(double fallback) const
    {
        return type == JsonType::number ? number : fallback;
    }

    std::string JsonValue::as_string(std::string fallback) const
    {
        return type == JsonType::string ? string : std::move(fallback);
    }

    const JsonValue* JsonValue::get(std::string_view key) const
    {
        if (type != JsonType::object)
        {
            return nullptr;
        }

        const auto found = object.find(std::string(key));
        return found == object.end() ? nullptr : &found->second;
    }

    const JsonValue& JsonValue::at(std::size_t index) const
    {
        static const JsonValue empty = null_value();
        if (type == JsonType::array && index < array.size())
        {
            return array[index];
        }
        return empty;
    }

    std::size_t JsonValue::size() const
    {
        switch (type)
        {
        case JsonType::array:
            return array.size();
        case JsonType::object:
            return object.size();
        case JsonType::string:
            return string.size();
        default:
            return 0;
        }
    }

    namespace
    {
        bool is_digit(char c)
        {
            return c >= '0' && c <= '9';
        }

        int hex_value(char c)
        {
            if (c >= '0' && c <= '9')
            {
                return c - '0';
            }
            if (c >= 'a' && c <= 'f')
            {
                return c - 'a' + 10;
            }
            if (c >= 'A' && c <= 'F')
            {
                return c - 'A' + 10;
            }
            return -1;
        }

        bool append_code_point(std::string& out, std::uint32_t code_point)
        {
            if (code_point <= 0x7F)
            {
                out.push_back(static_cast<char>(code_point));
                return true;
            }
            if (code_point <= 0x7FF)
            {
                out.push_back(static_cast<char>(0xC0 | (code_point >> 6)));
                out.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
                return true;
            }
            if (code_point <= 0xFFFF)
            {
                out.push_back(static_cast<char>(0xE0 | (code_point >> 12)));
                out.push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
                return true;
            }
            if (code_point <= 0x10FFFF)
            {
                out.push_back(static_cast<char>(0xF0 | (code_point >> 18)));
                out.push_back(static_cast<char>(0x80 | ((code_point >> 12) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | ((code_point >> 6) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | (code_point & 0x3F)));
                return true;
            }
            return false;
        }

        class JsonParser
        {
        public:
            explicit JsonParser(std::string_view text)
                : text_(text)
            {
            }

            bool parse(JsonValue& out)
            {
                skip_whitespace();
                if (!parse_value(out))
                {
                    return false;
                }
                skip_whitespace();
                return position_ == text_.size();
            }

        private:
            std::string_view text_;
            std::size_t position_ = 0;

            void skip_whitespace()
            {
                while (position_ < text_.size())
                {
                    const char c = text_[position_];
                    if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
                    {
                        return;
                    }
                    ++position_;
                }
            }

            bool consume(char expected)
            {
                if (position_ >= text_.size() || text_[position_] != expected)
                {
                    return false;
                }
                ++position_;
                return true;
            }

            bool consume_literal(std::string_view literal)
            {
                if (text_.substr(position_, literal.size()) != literal)
                {
                    return false;
                }
                position_ += literal.size();
                return true;
            }

            bool parse_value(JsonValue& out)
            {
                skip_whitespace();
                if (position_ >= text_.size())
                {
                    return false;
                }

                const char c = text_[position_];
                switch (c)
                {
                case '{':
                    return parse_object(out);
                case '[':
                    return parse_array(out);
                case '"':
                {
                    std::string text;
                    if (!parse_string(text))
                    {
                        return false;
                    }
                    out = JsonValue::from_string(std::move(text));
                    return true;
                }
                case 't':
                    if (!consume_literal("true"))
                    {
                        return false;
                    }
                    out = JsonValue::from_bool(true);
                    return true;
                case 'f':
                    if (!consume_literal("false"))
                    {
                        return false;
                    }
                    out = JsonValue::from_bool(false);
                    return true;
                case 'n':
                    if (!consume_literal("null"))
                    {
                        return false;
                    }
                    out = JsonValue::null_value();
                    return true;
                default:
                    if (c == '-' || is_digit(c))
                    {
                        return parse_number(out);
                    }
                    return false;
                }
            }

            bool parse_object(JsonValue& out)
            {
                ++position_;
                JsonValue result;
                result.type = JsonType::object;

                skip_whitespace();
                if (consume('}'))
                {
                    out = std::move(result);
                    return true;
                }

                while (true)
                {
                    std::string key;
                    if (!parse_string(key))
                    {
                        return false;
                    }

                    skip_whitespace();
                    if (!consume(':'))
                    {
                        return false;
                    }

                    JsonValue value;
                    if (!parse_value(value))
                    {
                        return false;
                    }
                    result.object[key] = std::move(value);

                    skip_whitespace();
                    if (consume('}'))
                    {
                        out = std::move(result);
                        return true;
                    }
                    if (!consume(','))
                    {
                        return false;
                    }
                    skip_whitespace();
                }
            }

            bool parse_array(JsonValue& out)
            {
                ++position_;
                JsonValue result;
                result.type = JsonType::array;

                skip_whitespace();
                if (consume(']'))
                {
                    out = std::move(result);
                    return true;
                }

                while (true)
                {
                    JsonValue item;
                    if (!parse_value(item))
                    {
                        return false;
                    }
                    result.array.push_back(std::move(item));

                    skip_whitespace();
                    if (consume(']'))
                    {
                        out = std::move(result);
                        return true;
                    }
                    if (!consume(','))
                    {
                        return false;
                    }
                    skip_whitespace();
                }
            }

            bool parse_hex4(std::uint32_t& out)
            {
                if (position_ + 4 > text_.size())
                {
                    return false;
                }

                std::uint32_t value = 0;
                for (int i = 0; i < 4; ++i)
                {
                    const int digit = hex_value(text_[position_ + static_cast<std::size_t>(i)]);
                    if (digit < 0)
                    {
                        return false;
                    }
                    value = value * 16 + static_cast<std::uint32_t>(digit);
                }

                position_ += 4;
                out = value;
                return true;
            }

            bool parse_string(std::string& out)
            {
                out.clear();
                if (!consume('"'))
                {
                    return false;
                }

                while (position_ < text_.size())
                {
                    const char c = text_[position_++];
                    if (c == '"')
                    {
                        return true;
                    }

                    if (c == '\\')
                    {
                        if (position_ >= text_.size())
                        {
                            return false;
                        }
                        const char escape = text_[position_++];
                        switch (escape)
                        {
                        case '"':
                            out.push_back('"');
                            break;
                        case '\\':
                            out.push_back('\\');
                            break;
                        case '/':
                            out.push_back('/');
                            break;
                        case 'b':
                            out.push_back('\b');
                            break;
                        case 'f':
                            out.push_back('\f');
                            break;
                        case 'n':
                            out.push_back('\n');
                            break;
                        case 'r':
                            out.push_back('\r');
                            break;
                        case 't':
                            out.push_back('\t');
                            break;
                        case 'u':
                        {
                            std::uint32_t code_point = 0;
                            if (!parse_hex4(code_point))
                            {
                                return false;
                            }
                            if (code_point >= 0xD800 && code_point <= 0xDBFF)
                            {
                                std::uint32_t low = 0;
                                if (position_ + 1 >= text_.size()
                                    || text_[position_] != '\\'
                                    || text_[position_ + 1] != 'u')
                                {
                                    return false;
                                }
                                position_ += 2;
                                if (!parse_hex4(low) || low < 0xDC00 || low > 0xDFFF)
                                {
                                    return false;
                                }
                                code_point = 0x10000 + ((code_point - 0xD800) << 10) + (low - 0xDC00);
                            }
                            else if (code_point >= 0xDC00 && code_point <= 0xDFFF)
                            {
                                return false;
                            }
                            if (!append_code_point(out, code_point))
                            {
                                return false;
                            }
                            break;
                        }
                        default:
                            return false;
                        }
                        continue;
                    }

                    if (static_cast<unsigned char>(c) < 0x20)
                    {
                        return false;
                    }
                    out.push_back(c);
                }
                return false;
            }

            bool parse_number(JsonValue& out)
            {
                const std::size_t start = position_;
                if (text_[position_] == '-')
                {
                    ++position_;
                }

                if (position_ >= text_.size())
                {
                    return false;
                }

                if (text_[position_] == '0')
                {
                    ++position_;
                    if (position_ < text_.size() && is_digit(text_[position_]))
                    {
                        return false;
                    }
                }
                else if (text_[position_] >= '1' && text_[position_] <= '9')
                {
                    while (position_ < text_.size() && is_digit(text_[position_]))
                    {
                        ++position_;
                    }
                }
                else
                {
                    return false;
                }

                if (position_ < text_.size() && text_[position_] == '.')
                {
                    ++position_;
                    if (position_ >= text_.size() || !is_digit(text_[position_]))
                    {
                        return false;
                    }
                    while (position_ < text_.size() && is_digit(text_[position_]))
                    {
                        ++position_;
                    }
                }

                if (position_ < text_.size() && (text_[position_] == 'e' || text_[position_] == 'E'))
                {
                    ++position_;
                    if (position_ < text_.size() && (text_[position_] == '+' || text_[position_] == '-'))
                    {
                        ++position_;
                    }
                    if (position_ >= text_.size() || !is_digit(text_[position_]))
                    {
                        return false;
                    }
                    while (position_ < text_.size() && is_digit(text_[position_]))
                    {
                        ++position_;
                    }
                }

                const std::string_view token = text_.substr(start, position_ - start);
                double number = 0;
                const auto result = std::from_chars(token.data(), token.data() + token.size(), number);
                if (result.ec != std::errc{} || result.ptr != token.data() + token.size())
                {
                    return false;
                }

                out = JsonValue::from_number(number);
                return true;
            }
        };

        void write_string(std::string& out, const std::string& value)
        {
            static constexpr char digits[] = "0123456789abcdef";
            out.push_back('"');
            for (const char c : value)
            {
                switch (c)
                {
                case '"':
                    out += "\\\"";
                    break;
                case '\\':
                    out += "\\\\";
                    break;
                case '\b':
                    out += "\\b";
                    break;
                case '\f':
                    out += "\\f";
                    break;
                case '\n':
                    out += "\\n";
                    break;
                case '\r':
                    out += "\\r";
                    break;
                case '\t':
                    out += "\\t";
                    break;
                default:
                {
                    const unsigned char byte = static_cast<unsigned char>(c);
                    if (byte < 0x20)
                    {
                        out += "\\u00";
                        out.push_back(digits[(byte >> 4) & 0x0F]);
                        out.push_back(digits[byte & 0x0F]);
                    }
                    else
                    {
                        out.push_back(c);
                    }
                    break;
                }
                }
            }
            out.push_back('"');
        }

        void write_value(std::string& out, const JsonValue& value)
        {
            switch (value.type)
            {
            case JsonType::null_value:
                out += "null";
                break;
            case JsonType::boolean:
                out += value.boolean ? "true" : "false";
                break;
            case JsonType::number:
            {
                if (!std::isfinite(value.number))
                {
                    out.push_back('0');
                    break;
                }
                std::array<char, 64> buffer{};
                const auto result = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value.number);
                if (result.ec == std::errc{})
                {
                    out.append(buffer.data(), static_cast<std::size_t>(result.ptr - buffer.data()));
                }
                else
                {
                    out.push_back('0');
                }
                break;
            }
            case JsonType::string:
                write_string(out, value.string);
                break;
            case JsonType::array:
            {
                out.push_back('[');
                bool first = true;
                for (const JsonValue& item : value.array)
                {
                    if (!first)
                    {
                        out.push_back(',');
                    }
                    first = false;
                    write_value(out, item);
                }
                out.push_back(']');
                break;
            }
            case JsonType::object:
            {
                out.push_back('{');
                bool first = true;
                for (const auto& entry : value.object)
                {
                    if (!first)
                    {
                        out.push_back(',');
                    }
                    first = false;
                    write_string(out, entry.first);
                    out.push_back(':');
                    write_value(out, entry.second);
                }
                out.push_back('}');
                break;
            }
            }
        }
    }

    bool parse_json(std::string_view text, JsonValue& out)
    {
        JsonParser parser(text);
        JsonValue value;
        if (!parser.parse(value))
        {
            return false;
        }
        out = std::move(value);
        return true;
    }

    std::string write_json(const JsonValue& value)
    {
        std::string out;
        write_value(out, value);
        return out;
    }
}
