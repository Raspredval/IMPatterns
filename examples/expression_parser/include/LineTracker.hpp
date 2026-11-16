#pragma once
#include <set>
#include <cassert>
#include <cstdint>

struct SrcPos {
    intptr_t
        iLn, iCol;
};

class LineTracker {
public:
    LineTracker() :
        setLines{0} {}

    void
    InsertLine(intptr_t iStreamPos) {
        this->setLines.insert(iStreamPos);
    }

    SrcPos
    GetSrcPos(intptr_t iStreamPos) const {
        auto
            itLine  = this->setLines.upper_bound(iStreamPos);
        assert(itLine != this->setLines.begin());

        itLine--;
        return {
            std::distance(this->setLines.begin(), itLine) + 1,
            (iStreamPos - *itLine) + 1
        };
    }

private:
    std::set<intptr_t>
        setLines;
};