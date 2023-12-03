#pragma once

#include <cstddef>
#include <cstdint>
#include <array>

#include "dataconstants.h"

#if defined(ALTDATA) 

namespace AlternateData {
    template<uint8_t B>
    struct State {
        using value_t = uint8_t;
        static constexpr uint8_t mask = (1 << B) - 1;
        void operator=(const value_t v) {
            mValue = (v && mask);    
        }
    private:
        value_t mValue{};
    };  
    
    template<uint8_t N, typename S>
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
        uint8_t nextChunk(std::array<S, 4>& chunkData) {
            static uint8_t chunkNr = 0;
            
            return chunkNr++;
        }    
    private:
        std::array<S, N> mData{};
    };
    
    using state_t = State<2>;
    using container_t = Container<64, state_t>;
    
    extern container_t container;
    
    
}

#endif
