#pragma once
static_assert(__cplusplus >= 202002L, "requires C++23 minimum version");

#include "FixedString.hpp"
#include <string_view>
#include <vector>

namespace imp {
    class Dictionary {
        struct NodeData {
            static constexpr size_t
                N   = sizeof(size_t) * 8 - 1;

            const char*
                lpcWord     = nullptr;
            size_t
                uLength : N = 0,
                bIsEnd  : 1 = false;

            NodeData() = default;

            NodeData(std::string_view strvData, bool bIsEnd = false) :
                lpcWord(strvData.data()),
                uLength(strvData.size()),
                bIsEnd(bIsEnd) {}

            operator
            std::string_view() const noexcept {
                return { this->lpcWord, this->uLength };
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
                if (!this->lpNode)
                    return false;
                const NodeData&
                    ndt = this->lpNode->ndtData;
                return ndt.uLength == this->uMatchLen;
            }

            bool
            IsEndSegment() const noexcept {
                if (!this->lpNode)
                    return false;
                const NodeData&
                    ndt = this->lpNode->ndtData;
                return (bool)ndt.bIsEnd;
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
            ([this] (std::string_view strv) {
                this->Insert(strv);
            } ((std::string_view)args), ...);
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
                if (ndt.lpcWord[m.uMatchLen] == c) {
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

        void
        Insert(std::string_view strvInsert) {
            return insertImpl(this->vecRoot, strvInsert);
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
                if (ndt.lpcWord[0] == c)
                    return DictMatch(refNode);
            }

            return DictMatch{};
        }

        static void
        insertImpl(std::vector<Node>& vecNodes, std::string_view strvInsert) {
            for (Node& refNode : vecNodes) {
                std::string_view
                    strvNode    = refNode.ndtData;
                size_t
                    uPrefLen    = commonPrefix(strvNode, strvInsert);

                if (uPrefLen == strvNode.size()) {
                    if (strvInsert.size() == strvNode.size())
                        refNode.ndtData.bIsEnd = true;
                    else
                        insertImpl(refNode.vecChildren, strvInsert.substr(uPrefLen));

                    return;
                }
                else if (uPrefLen != 0) {
                    std::vector<Node>
                        vecCurChlidren  = std::move(refNode.vecChildren);
                    auto [ ndtPrefix, ndtPostfix] =
                        splitNodeData(refNode.ndtData, uPrefLen);
                    NodeData
                        ndtInsert       = { strvInsert.substr(uPrefLen), true };

                    refNode.ndtData     = ndtPrefix;
                    refNode.vecChildren = std::vector<Node> {
                        Node{ ndtPostfix,   std::move(vecCurChlidren)   },
                        Node{ ndtInsert,    std::vector<Node>{}         }
                    };

                    return;
                }
            }

            NodeData
                ndtInsert   = { strvInsert, true };
            vecNodes.emplace_back(
                ndtInsert, std::vector<Node>{});
        }

        static std::pair<NodeData, NodeData>
        splitNodeData(const NodeData& ndt, size_t uWhere) {
            std::string_view
                strvNode    = (std::string_view)ndt,
                strvPrefix  = strvNode.substr(0, uWhere),
                strvPostfix = strvNode.substr(uWhere);
            return {
                NodeData{ strvPrefix,   false       },
                NodeData{ strvPostfix,  ndt.bIsEnd  }
            };
        }

        std::vector<Node>
            vecRoot;
    };
}