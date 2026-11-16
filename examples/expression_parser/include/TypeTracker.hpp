#pragma once
#include <set>
#include <string_view>

class TypeTracker {
public:
    TypeTracker() :
        setTypes{"int", "float", "bool"} {}

    bool
    HasType(std::string_view strvTypename) const noexcept {
        return this->setTypes.contains(strvTypename);
    }

private:
    std::set<std::string_view>
        setTypes;
};