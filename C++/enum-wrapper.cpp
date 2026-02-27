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

#include <string_view>
#include <array>
#include <algorithm>

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

    [[nodiscard]] static constexpr std::optional<Enum> parse(std::string_view input) {
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

    [[nodiscard]] static constexpr const std::string_view& to_string(enum Enum value) {
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
