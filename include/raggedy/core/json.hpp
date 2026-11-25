#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace raggedy::core
{
    enum class JsonType
    {
        null_value,
        boolean,
        number,
        string,
        array,
        object
    };

    struct JsonValue
    {
        JsonType type = JsonType::null_value;
        bool boolean = false;
        double number = 0;
        std::string string;
        std::vector<JsonValue> array;
        std::map<std::string, JsonValue> object;

        static JsonValue null_value();
        static JsonValue from_bool(bool value);
        static JsonValue from_number(double value);
        static JsonValue from_string(std::string value);
        static JsonValue from_array(std::vector<JsonValue> values);
        static JsonValue from_object(std::map<std::string, JsonValue> values);

        bool as_bool(bool fallback = false) const;
        double as_number(double fallback = 0) const;
        std::string as_string(std::string fallback = {}) const;
        const JsonValue* get(std::string_view key) const;
        const JsonValue& at(std::size_t index) const;
        std::size_t size() const;
    };

    bool parse_json(std::string_view text, JsonValue& out);
    std::string write_json(const JsonValue& value);
}
