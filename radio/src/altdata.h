#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <tuple>

#include "debug.h"
#include "dataconstants.h"

#if defined(ALTDATA) 

namespace AlternateData {
    template<uint8_t N, typename S = uint8_t>
    struct Container {
        using value_t = S;
        constexpr uint8_t size() const {
            return N;
        }
        value_t& operator[](const uint8_t item) {
            assert(item < size());
            return mData[item];
        }
        const value_t& operator[](const uint8_t item) const {
            assert(item < size());
            return mData[item];
        }
        template<typename F>
        uint8_t pushNextChunk(const uint8_t numberOfValuesToPush, const F f) {
            const uint8_t start = mNextIndex;
            for(uint8_t i = 0; i < numberOfValuesToPush; ++i) {
                const uint8_t v = mData[mNextIndex];
                TRACE("pushNextData: %d = %d", mNextIndex, v);
                f(v);
                ++mNextIndex;
                if (mNextIndex >= N) {
                    mNextIndex = 0;
                }
            }
            return start;
        }
    private:
        uint8_t mNextIndex{0};
        std::array<S, N> mData{};
    };
    
    using container_t = Container<64>;
    extern container_t container;
}

#endif
