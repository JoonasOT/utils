#include <string_view>
#include <array>
#include <algorithm>
#include <string>
#include <cmath>
#include <vector>
#include <variant>

#include <fmt/core.h>
#include <iostream>

namespace json {

#define EnumWrapperName JsonLiteral
#define EnumMappings \
    X(JsonTrue, "true") \
    X(JsonFalse, "false") \
    X(JsonNull, "null") \
    X(JsonDecimalSeparator, ".") \
    X(JsonMinus, "-") \
    X(JsonPlus, "+") \
    X(JsonExponent, "e") \
    X(JsonObjectOpen, "{") \
    X(JsonObjectClose, "}") \
    X(JsonArrayOpen, "[") \
    X(JsonArrayClose, "]") \
    X(JsonSeparator, ",") \
    X(JsonQuote, "\"") \
    X(JsonColon, ":")

// ============================================================================
// ||                          Enum Wrapper Utility                          ||
// ||                                                                        ||
// || Definition:                                                            ||
// ||   This utility creates a simple wrapper around an enum. The wrapper    ||
// ||   provides a simple constant time mapping between enum values and      ||
// ||   their string representations.                                        ||
// ||                                                                        ||
// || Requirements:                                                          ||
// ||   - macro EnumWrapperName - The name of the enum (class)               ||
// ||   - macro EnumMappings    - The enum values and their representations  ||
// ||                             as strings defined using the X-macro.      ||
// ||   - macro EnumSameString  - Optionally defined macro to indicate to    ||
// ||                             whether to use the same string repr for    ||
// ||                             the enum values (depends on EnumMappings)  ||
// ||                                                                        ||
// || Demo @ compiler explorer:                                              ||
// ||   https://godbolt.org/z/aofT796Yh                                      ||
// ============================================================================

class EnumWrapperName {
    static constinit const int N =
        #ifdef EnumSameString
            #define X(_) 1 +
                EnumMappings
            #undef X
        #else
            #define X(_, __) 1 +
                EnumMappings
            #undef X
        #endif
        0;

    static constexpr const std::array<std::string_view, EnumWrapperName::N> values = {
        #ifdef EnumSameString
            #define X(mapping) #mapping,
                EnumMappings
            #undef X
        #else
            #define X(_, mapping) mapping,
                EnumMappings
            #undef X
        #endif
    };

public:
    enum Enum : std::size_t {
        #ifdef EnumSameString
            #define X(value) value,
                EnumMappings
            #undef X
        #else
            #define X(value, _) value,
                EnumMappings
            #undef X
        #endif

    };

    static constexpr std::optional<Enum> parse(std::string_view input) {
        if (
            const auto it = std::find_if(
                values.cbegin(),
                values.cend(),
                [&input](auto value)->bool{ return input == value; }
            ); 
            it != values.cend()
        ) {
            return std::optional<Enum>{
                std::in_place,
                static_cast<Enum>(it - values.cbegin())
            };
        }
        return std::nullopt;
    };

    static constexpr const std::string_view& to_string(enum Enum value) {
        return values.at(static_cast<std::size_t>(value));
    };
};

#undef EnumMappings
#undef EnumWrapperName
#ifdef EnumSameString
#undef EnumSameString
#endif

// ============================================================================
// ||                                                                        ||
// ||                          Enum Wrapper Utility                          ||
// ||                             <--- END --->                              ||
// ||                                                                        ||
// ============================================================================

using Null = std::monostate;

struct PrimitiveType {
    std::size_t accumulated_chars;
    std::variant<Null, bool, float, std::string_view> value;
};

[[nodiscard]] constexpr std::optional<PrimitiveType> parse_null(std::string_view input) {
    const auto null_ = JsonLiteral::to_string(JsonLiteral::Enum::JsonNull);

    if (input.substr(0, null_.size()) != null_) return std::nullopt;
    return std::optional<PrimitiveType>{std::in_place, null_.size()};
}

[[nodiscard]] constexpr std::optional<PrimitiveType> parse_bool(std::string_view input) {
    const auto true_ = JsonLiteral::to_string(JsonLiteral::Enum::JsonTrue);
    const auto false_ = JsonLiteral::to_string(JsonLiteral::Enum::JsonFalse);

    if (input.substr(0, true_.size()) == true_) {
        return std::optional<PrimitiveType>{std::in_place, true_.size(), true};
    }
    if (input.substr(0, false_.size()) == false_) {
        return std::optional<PrimitiveType>{std::in_place, false_.size(), false};
    }
    return std::nullopt;
}

[[nodiscard]] constexpr std::optional<PrimitiveType> parse_string(std::string_view input) {
    const auto quote = JsonLiteral::to_string(JsonLiteral::Enum::JsonQuote)[0];
    if (!input.starts_with(quote)) return std::nullopt;
    const auto next = input.substr(1).find(quote, 1);
    if (next == input.npos) return std::nullopt;
    return std::optional<PrimitiveType>{std::in_place, next + 1, input.substr(1, next)};
}

[[nodiscard]] constexpr std::optional<std::string_view> parse_key(std::string_view input) {
    const auto colon = JsonLiteral::to_string(JsonLiteral::Enum::JsonColon)[0];
    const auto key_res = parse_string(input);
    if (
        !key_res.has_value() || 
        !std::holds_alternative<std::string_view>(key_res.value().value) ||
        !(input.at(key_res.value().accumulated_chars) != colon)
    ) return std::nullopt;

    return std::get<std::string_view>(key_res.value().value);
}

namespace number_details {

static const char exponent = JsonLiteral::to_string(JsonLiteral::Enum::JsonExponent)[0];
static const char decimal_sep = JsonLiteral::to_string(JsonLiteral::Enum::JsonDecimalSeparator)[0];
static const char plus = JsonLiteral::to_string(JsonLiteral::Enum::JsonPlus)[0];
static const char minus = JsonLiteral::to_string(JsonLiteral::Enum::JsonMinus)[0];

static constexpr const char valid_chars[] = {
    '0','1','2','3','4','5','6','7','8','9', 
    decimal_sep, 
    plus,
    minus,
    exponent
};

constexpr bool is_digit(char c) {
    return c <= '9' && c >= '0';
}

constexpr int stoi_impl(const char* str, int value = 0) {
    return *str ?
            is_digit(*str) ?
                stoi_impl(str + 1, (*str - '0') + value * 10)
                : value
            : value;
}

[[nodiscard]] constexpr auto parse_int(std::string_view input) { 
    return stoi_impl(input.data()); // TODO: Optimise
}

[[nodiscard]] constexpr float parse_float(std::string_view input) {
    if consteval {
        const auto offset = input.find(decimal_sep);
        const float base = parse_int(input.substr(0, offset));
        const float fraction = parse_int(input.substr(offset + 1)) / std::powf(10, input.size() - offset - 1);

        return base + fraction;
    } else {
        return std::stof(std::string{input});
    }
}

[[nodiscard]] constexpr std::optional<int> parse_exponent(std::string_view input) {    
    if (input.size() <= 0 || input.front() != exponent) return std::nullopt;

    return parse_int(input.substr(1));
}

}; // namespace details

[[nodiscard]] constexpr std::optional<PrimitiveType> parse_number(std::string_view input) { 
    const std::size_t number_offset = std::min(
        input.find_first_not_of(number_details::valid_chars, 0, sizeof(number_details::valid_chars)),
        input.size()
    );
    
    if (number_offset == 0) return std::nullopt;

    const auto numeric = input.substr(0, number_offset);


    const auto decimal = numeric.find(number_details::decimal_sep);
    const auto exponent_offset = numeric.find(number_details::exponent);
    const auto mantissa = numeric.substr(0, exponent_offset);
    const float base = 
        decimal != numeric.npos ?
            number_details::parse_float(mantissa) :
            number_details::parse_int(mantissa);

    const auto exponent =
        exponent_offset != numeric.npos ? 
        number_details::parse_exponent(numeric.substr(exponent_offset)) :
        std::nullopt;

    const auto number = exponent ? base * std::powf(10.0f, exponent.value()) : base;

    return std::optional<PrimitiveType>{
        std::in_place,
        number_offset,
        number
    };
}

[[nodiscard]] constexpr std::optional<PrimitiveType> parse_primitive(std::string_view input) {
    typedef std::optional<PrimitiveType> (*parser)(std::string_view);
    const parser parsers[] = { parse_null, parse_bool, parse_string, parse_number };
    for (std::size_t i = 0; i < sizeof(parsers); i++) {
        if (const auto res = parsers[i](input)) return res;
    }
    return std::nullopt;
}

}; // namespace json

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

int main() {
    static constexpr const std::string_view input{"123151.12412"};
    static constinit const auto parse_res = json::parse_primitive(input);
    std::visit(
        overloaded(
            [](const json::Null){ std::cout << "Null" << std::endl; },
            [](const auto res){ std::cout << fmt::format("{}", res) << std::endl; }
        ),
        parse_res.value().value
    );
    
}