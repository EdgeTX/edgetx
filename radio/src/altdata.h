#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <tuple>

#include "debug.h"
#include "dataconstants.h"

#if defined(ALTDATA) 

namespace AlternateData {
    template<uint8_t B>
    struct State {
        using value_t = uint8_t;
        static constexpr uint8_t mask = (1 << B) - 1;
        void operator=(const value_t v) {
            mValue = (v & mask);    
        }
        uint8_t toInt() const {
            return mValue;
        }
    private:
        value_t mValue{};
    };  
    
    template<uint8_t N, typename S>
    struct Container {
        using value_t = S;
        using chunk_t = std::array<S, 4>;
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
        uint8_t pushNextData(const uint8_t numberOfValuesToPush, const F f) {
            const uint8_t start = mNextIndex;
            for(uint8_t i = 0; i < numberOfValuesToPush; ++i) {
                const uint8_t v = mData[mNextIndex].toInt();
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
    
    using state_t = State<2>;
    using container_t = Container<64, state_t>;
    using chunk_t = container_t::chunk_t;
    
    extern container_t container;
}

#endif
