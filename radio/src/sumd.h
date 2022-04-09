#pragma once
#include "opentx.h"
#include "trainer.h"
#include "switches.h"

#include <algorithm>
#include <limits>

void processSumdInput();

#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic error "-Wswitch" // unfortunately the project uses -Wnoswitch
#endif

namespace SumDV3 {
    static constexpr uint32_t baudrate{115200};
    
    struct Crc16 {
        inline void reset() {
            sum = 0;
        }
        inline void operator+=(const uint8_t v) {
            sum = sum ^ (((uint16_t)v) << 8);
            for(uint8_t i = 0; i < 8; ++i) {
                if (sum & 0x8000) {
                    sum = (sum << 1) ^ crc_polynome;
                }
                else {
                    sum = (sum << 1);
                }
            }
        }
        inline operator uint16_t() const {
            return sum;
        }
    private:
        static constexpr uint16_t crc_polynome = 0x1021;
        uint16_t sum{};
    };
    
    template<uint8_t Instance>    
    struct Servo {
        using SumDV3 = Trainer::Protocol::SumDV3;
        using MesgType = SumDV3::MesgType;
        using SwitchesType = SumDV3::SwitchesType;
        using Command_t = SumDV3::Command_t;
        
        template<uint8_t B, uint8_t E> struct range_t {};
        template<uint8_t N> using offset_t = std::integral_constant<uint8_t, N>;
        
        enum class State : uint8_t {Undefined, GotStart, StartV1, StartV3, V1ChannelDataH, V1ChannelDataL, CrcH, CrcL,
                                   V3ChannelDataH, V3ChannelDataL, V3FuncCode, V3LastValidPackage, V3ModeCmd, V3SubCmd};
        
        enum class Frame : uint8_t {Ch1to12 = 0x00, First = Ch1to12, 
                                    Ch1to8and13to16 = 0x01, Ch1to16 = 0x02, Ch1to8and17to24 = 0x03,
                                    Ch1to8and25to32 = 0x04, Ch1to12and64Switches = 0x05,
                                    Last = Ch1to12and64Switches,
                                    Undefined = 0xff};
                                    
        static inline void process(const uint8_t b, const std::function<void()> f) {
            ++mBytesCounter;
            switch(mState) { // enum-switch -> no default (intentional)
            case State::Undefined:
                if (b == SumDV3::start_code) {
                    csum.reset();
                    csum += b;
                    mState = State::GotStart;
                }
                break;
            case State::GotStart:
                csum += b;
                if ((b & 0x0f) == SumDV3::version_code1) {
                    mState = State::StartV1;
                }
                else if ((b & 0x0f) == SumDV3::version_code3) {
                    mState = State::StartV3;
                }
                else {
                    mState = State::Undefined;
                }
                break;
            case State::StartV1:
                if ((b >= 2) && (b <= 32)) {
                    csum += b;
                    nChannels = b;
                    mIndex = 0;
                    mState = State::V1ChannelDataH;
                }
                else {
                    mState = State::Undefined;
                }
                break;
            case State::V1ChannelDataH:
                csum += b;
                sumdFrame[mIndex].first = b;
                mState = State::V1ChannelDataL;
                break;
            case State::V1ChannelDataL:
                csum += b;
                sumdFrame[mIndex].second = b;
                ++mIndex;
                if (mIndex < nChannels) {
                    mState = State::V1ChannelDataH;
                }
                else {
                    mState = State::CrcH;
                    frame = Frame::Ch1to16;
                }
                break;
            case State::CrcH:
                crcH = b;
                mState = State::CrcL;
                break;
            case State::CrcL:
                if (((((uint16_t)crcH) << 8) | b) == csum) {
                    ++mPackagesCounter;
                    f();
                }
                mState = State::Undefined;
                break;
            case State::StartV3:
                if ((b >= 2) && (b <= 68)) {
                    csum += b;
                    nChannels = b - 2;
                    mIndex = 0;
                    mState = State::V3ChannelDataH;
                }
                else {
                    mState = State::Undefined;
                }
                break;
            case State::V3ChannelDataH:
                csum += b;
                sumdFrame[mIndex].first = b;
                mState = State::V3ChannelDataL;
                break;
            case State::V3ChannelDataL:
                csum += b;
                sumdFrame[mIndex].second = b;
                ++mIndex;
                if (mIndex < nChannels) {
                    mState = State::V3ChannelDataH;
                }
                else {
                    mState = State::V3FuncCode;
                }
                break;
            case State::V3FuncCode:
                csum += b;
                if ((b >= uint8_t(Frame::First)) && (b <= uint8_t(Frame::Last))) {
                    frame = Frame(b);
                }
                else {
                    frame = Frame::Undefined;
                }
                mState = State::V3LastValidPackage;
                break;
            case State::V3LastValidPackage:
                csum += b;
                reserved = b;
                mState = State::V3ModeCmd;
                break;
            case State::V3ModeCmd:
                csum += b;
                mode_cmd = b;
                mState = State::V3SubCmd;
                break;
            case State::V3SubCmd:
                csum += b;
                sub_cmd = b;
                if (!hasCommand()) {
                    mCommand = Command_t{mode_cmd, sub_cmd};
                }                
                mState = State::CrcH;
                break;
            }            
        }        

        template<uint8_t N>
        static inline void convert(int16_t (&pulses)[N]) {
            static_assert(N >= 16, "array too small");
            switch(frame) {
            case Frame::Ch1to12:
                extract(range_t<0, 11>{}, pulses);
                break;
            case Frame::Ch1to8and13to16:
                extract(range_t<0, 7>{}, pulses);
                extract(range_t<8, 11>{}, pulses, offset_t<12>{});
                break;
            case Frame::Ch1to8and17to24:
                extract(range_t<0, 7>{}, pulses);
                extract(range_t<8, 15>{}, pulses, offset_t<16>{}, std::integral_constant<bool, (N > 16) >{}); // no constepr-if in c++11
//                if constexpr(N > 16) {
//                    extract(range_t<8, 15>{}, pulses, offset_t<16>{});
//                }
                break;
            case Frame::Ch1to8and25to32:
                extract(range_t<0, 7>{}, pulses);
                extract(range_t<8, 15>{}, pulses, offset_t<24>{}, std::integral_constant<bool, (N > 16) >{}); // no constepr-if in c++11
//                if constexpr(N > 16) {
//                    extract(range_t<8, 15>{}, pulses, offset_t<24>{});
//                }
                break;
            case Frame::Ch1to16:
                extract(range_t<0, 15>{}, pulses);
                break;
            case Frame::Ch1to12and64Switches:
                extract(range_t<0, 11>{}, pulses);
                sumSwitches();
                break;
            case Frame::Undefined:
                break;
            }            
        }
        static inline uint16_t packages() {
            return mPackagesCounter;
        }
        static inline uint16_t getbytes() {
            return mBytesCounter;
        }
        static inline bool hasCommand() {
            return mCommand != Command_t{};
        }
        static inline Command_t command() {
            Command_t c{};
            using std::swap;
            swap(c, mCommand);
            return c;
        }
    private:
        // uses tag-dispatch because no constexpr-if in c++11
        template<uint8_t Begin, uint8_t End, uint8_t N, uint8_t Off = 0>
        static inline void extract(const range_t<Begin, End>&, int16_t (&dest)[N], offset_t<Off> = offset_t<0>{}, std::true_type = std::true_type{}) {
            static_assert((End - Begin) < (N - Off), "wrong range or target size");
            uint8_t out{Off};
            for(uint8_t i = Begin; i <= End; ++i) {
                uint16_t raw = (sumdFrame[i].first << 8) | sumdFrame[i].second;
                dest[out++] = convertSumdToPuls(raw);
            } 
        }
        template<uint8_t Begin, uint8_t End, uint8_t N, uint8_t Off = 0>
        static inline void extract(const range_t<Begin, End>&, int16_t (&dest)[N], offset_t<Off>, std::false_type) {
        }
        static inline void sumSwitches() {
            uint64_t sw = sumdFrame[12].first;
            sw = (sw << 8) | sumdFrame[12].second;
            sw = (sw << 8) | sumdFrame[13].first;
            sw = (sw << 8) | sumdFrame[13].second;
            sw = (sw << 8) | sumdFrame[14].first;
            sw = (sw << 8) | sumdFrame[14].second;
            sw = (sw << 8) | sumdFrame[15].first;
            sw = (sw << 8) | sumdFrame[15].second;
            
            for (uint8_t i = 0; i < MAX_LOGICAL_SWITCHES; ++i) {
                const uint64_t mask = (((uint64_t)0x01) << i);
                if (sw & mask) {
                    rawSetUnconnectedStickySwitch(i, true);
                }
                else {
                    rawSetUnconnectedStickySwitch(i, false);
                }
            }
        }
        static inline int16_t convertSumdToPuls(uint16_t const value) {
            const int32_t centered = value - SumDV3::CenterValue;
            return Trainer::clamp(((Trainer::MaxValue - Trainer::MinValue) * centered) / (SumDV3::MaxValue -SumDV3::MinValue));
        }
        static uint8_t nChannels;
        static Crc16 csum;
        static uint8_t crcH;
        static State mState;
        static MesgType sumdFrame;
        static uint8_t mIndex;
        static uint16_t mPackagesCounter;
        static uint16_t mBytesCounter;
        static Frame frame;
        static uint8_t reserved;
        static uint8_t mode_cmd;
        static uint8_t sub_cmd;
        static Command_t mCommand;
    };
    // inline static member definitions not until c++17
    template<uint8_t Instance>
    uint8_t Servo<Instance>::nChannels{};
    template<uint8_t Instance>
    Crc16 Servo<Instance>::csum{};
    template<uint8_t Instance>
    uint8_t Servo<Instance>::crcH{};
    
    template<uint8_t Instance>
    typename Servo<Instance>::State Servo<Instance>::mState{Servo::State::Undefined};
    template<uint8_t Instance>
    typename Servo<Instance>::MesgType Servo<Instance>::sumdFrame; 
    template<uint8_t Instance>
    uint8_t Servo<Instance>::mIndex{};

    template<uint8_t Instance>
    uint16_t Servo<Instance>::mPackagesCounter{};
    template<uint8_t Instance>
    uint16_t Servo<Instance>::mBytesCounter{};

    template<uint8_t Instance>
    typename Servo<Instance>::Frame Servo<Instance>::frame{Servo::Frame::Undefined};
    template<uint8_t Instance>
    uint8_t Servo<Instance>::reserved{};
    template<uint8_t Instance>
    uint8_t Servo<Instance>::mode_cmd{};
    template<uint8_t Instance>
    uint8_t Servo<Instance>::sub_cmd{};
    template<uint8_t Instance>
    typename Servo<Instance>::Command_t Servo<Instance>::mCommand{};
}

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif
