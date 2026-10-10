#pragma once
static_assert(__cplusplus >= 202002L, "requires C++23 minimum version");

#include "FixedString.hpp"
#include "Capture.hpp"

#include <string_view>
#include <cassert>
#include <vector>


namespace imp {
    class Dictionary {
        struct NodeData {
            static constexpr size_t
                N   = sizeof(size_t) * 8 - 1;

            const char*
                lpcSegment  = nullptr;
            uhalfptr_t
                uLength     = 0,
                uWordID     = 0;

            NodeData() = default;

            NodeData(std::string_view strvData, uhalfptr_t uWordID = 0) :
                lpcSegment(strvData.data()),
                uLength((uhalfptr_t)strvData.size()),
                uWordID(uWordID)
            {
                assert(!strvData.empty() && strvData.data() != nullptr);
                assert(!(strvData.size() > uhalfptr_max));
            }

            std::pair<NodeData, NodeData>
            split(size_t uWhere) const {
                std::string_view
                    strvNode    = (std::string_view)*this,
                    strvPrefix  = strvNode.substr(0, uWhere),
                    strvPostfix = strvNode.substr(uWhere);
                return {
                    NodeData{ strvPrefix,   0               },
                    NodeData{ strvPostfix,  this->uWordID   }
                };
            }

            operator
            std::string_view() const noexcept {
                return { this->lpcSegment, this->uLength };
            }
        };

        struct Node {
            NodeData
                ndtData;
            std::vector<Node>
                vecChildren;
        };

    public:
        class DictMatch {
            friend class Dictionary;
        public:
            DictMatch() = default;

            DictMatch(const Node& refNode) :
                lpNode(&refNode),
                uMatchLen(1) {}

            bool
            AtSegmentEnd() const noexcept {
                assert(this->lpNode);
                const NodeData&
                    ndt = this->lpNode->ndtData;
                return ndt.uLength == this->uMatchLen;
            }

            bool
            IsLeafSegment() const noexcept {
                assert(this->lpNode);
                const NodeData&
                    ndt = this->lpNode->ndtData;
                return (ndt.uWordID != 0);
            }

            bool
            AtLeafSegmentEnd() const noexcept {
                assert(this->lpNode);
                const NodeData&
                    ndt = this->lpNode->ndtData;
                return (ndt.uWordID != 0) && (ndt.uLength == this->uMatchLen);
            }

            uhalfptr_t
            WordID() const noexcept {
                assert(this->lpNode);
                const NodeData&
                    ndt = this->lpNode->ndtData;
                return ndt.uWordID;
            }

            size_t
            Length() const noexcept {
                return this->uMatchLen;
            }

            bool
            Good() const noexcept {
                return (bool)this->lpNode;
            }

            operator
            bool() const noexcept {
                return this->Good();
            }

        private:
            const Node*
                lpNode      = nullptr;
            size_t
                uMatchLen   = 0;
        };

        template<size_t... n>
        Dictionary(const imp::FixedString<n>&... args) {
            uhalfptr_t
                uWordID = 1;
            ([this] (std::string_view strvInsert, uhalfptr_t uWordID) {
                this->insertImpl(this->vecRoot, strvInsert, uWordID);
            } ((std::string_view)args, uWordID++), ...);
        }

        DictMatch
        StartMatch() const {
            return DictMatch{};
        }

        DictMatch
        NextMatch(DictMatch m, char c) const {
            if (!m)
                return matchChar(this->vecRoot, c);

            const NodeData&
                ndt = m.lpNode->ndtData;
            if (m.uMatchLen < ndt.uLength) {
                if (ndt.lpcSegment[m.uMatchLen] == c) {
                    m.uMatchLen++;
                    return m;
                }
                else
                    return DictMatch{};
            }
            else {
                return matchChar(m.lpNode->vecChildren, c);
            }
        }

    private:
        static size_t
        commonPrefix(std::string_view strvA, std::string_view strvB) {
            size_t
                uLength = std::min(strvA.size(), strvB.size());
            for (size_t i = 0; i != uLength; ++i) {
                if (strvA[i] != strvB[i])
                    return i;
            }

            return uLength;
        }

        static DictMatch
        matchChar(const std::vector<Node>& vecNodes, char c) {
            for (const Node& refNode : vecNodes) {
                const NodeData&
                    ndt = refNode.ndtData;
                if (ndt.lpcSegment[0] == c)
                    return DictMatch(refNode);
            }

            return DictMatch{};
        }

        static void
        insertImpl(std::vector<Node>& vecNodes, std::string_view strvInsert, uhalfptr_t uWordID) {
            assert(uWordID != 0);

            for (Node& refNode : vecNodes) {
                std::string_view
                    strvNode    = refNode.ndtData;
                size_t
                    uPrefLen    = commonPrefix(strvNode, strvInsert);

                if (uPrefLen == strvNode.size()) {
                    if (strvInsert.size() == strvNode.size())
                        refNode.ndtData.uWordID = uWordID;
                    else
                        insertImpl(refNode.vecChildren, strvInsert.substr(uPrefLen), uWordID);

                    return;
                }
                else if (uPrefLen != 0) {
                    std::vector<Node>
                        vecCurChlidren  = std::move(refNode.vecChildren);
                    auto [ ndtPrefix, ndtPostfix] =
                        refNode.ndtData.split(uPrefLen);
                    NodeData
                        ndtInsert       = { strvInsert.substr(uPrefLen), 0 };

                    refNode.ndtData     = ndtPrefix;
                    refNode.vecChildren = std::vector<Node> {
                        Node{ ndtPostfix,   std::move(vecCurChlidren)   },
                        Node{ ndtInsert,    std::vector<Node>{}         }
                    };

                    return;
                }
            }

            NodeData
                ndtInsert   = { strvInsert, uWordID };
            vecNodes.emplace_back(
                ndtInsert, std::vector<Node>{});
        }

        std::vector<Node>
            vecRoot;
    };
}