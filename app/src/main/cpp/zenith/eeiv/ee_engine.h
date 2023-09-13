#pragma once

#include <eeiv/cop0.h>
#include <os/neon_types.h>
#include <link/global_memory.h>

namespace zenith::eeiv {
    enum struct EEExecutionMode : u8 {
        // Increases instruction decoding speed through cache blocks, which is faster
        // than a simple interpreter
        CachedInterpreter,
        // JIT compiler, the fastest option but with various interpretation issues
        JitRe
    };

    class EEMipsCore {
        static constexpr u8 countOfGPRs{32};
    public:
        EEMipsCore(const std::shared_ptr<console::GlobalMemory>& memoryChips);
        ~EEMipsCore();

        void resetCore();
    private:
        EEExecutionMode m_execModel{EEExecutionMode::CachedInterpreter};

        std::shared_ptr<console::GlobalMemory> m_glbMemory;
        os::MappedMemory<u8> m_mainRamBlock;

        union eeRegister {
            eeRegister()
                : dw{0, 0}
                {}
            os::machVec128 qw;
            u64 dw[2];
            u32 words[4];
            u16 hw[8];
            u8 rwBytes[16];
        };
        eeRegister* m_gprs;

        u32 m_eePC{};
        cop::CoProcessor0 m_copCPU0{};
    };
}
