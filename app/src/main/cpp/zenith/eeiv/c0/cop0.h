#pragma once

#include <common/types.h>

#include <mio/mmu_tlb.h>
#include <eeiv/c0/high_fast_cache.h>
namespace zenith::eeiv {
    class EEMipsCore;
}

namespace zenith::eeiv::c0 {
    static constexpr u8 cop0RegsCount{32};
    enum KSU : u8 {
        kernel,
        supervisor,
        user
    };

     union alignas(4) Cop0Status {
        u32 rawStatus{};
        struct {
            bool copUsable : 1;
            // Set when a level 1 exception occurs
            bool exception : 1;
            // Set when a level 2 exception occurs
            bool error : 1;
            KSU mode : 3;
            bool masterIE : 1;
            bool edi : 1;

            // If set (bev or dev), level 1 exceptions go to "bootstrap" vectors in BFC00xxx
            bool bev: 1;
            bool dev: 1;
        };
    };

    union Cop0Cause {
        u32 rawCause{};
        struct {
            u8 exCode: 4;
            bool timerIP;
            // Set when a level 1 exception occurs in a delay slot
            bool bd;
            bool bd2;
        };
    };

    class CoProcessor0 {
    public:
        static constexpr u8 countOfCacheLines{128};
        static constexpr auto invCacheBit{static_cast<u32>(1 << 31)};

        CoProcessor0();
        CoProcessor0(CoProcessor0&&) = delete;
        CoProcessor0(CoProcessor0&) = delete;
        ~CoProcessor0();

        union alignas(4) {
            // The codenamed pRid register determines in the very early boot process for the BIOS
            // which processor it is currently running on, whether it's on the EE or the PSX
            union {
                Cop0Status status;
                u32 count;
                u32 compare;
                Cop0Cause cause;
                // PC backup value used when returning to an exception handler
                u32 ePC;
                u32 pRid;
                u32 perfCounter;

                u32 errorPC;

                std::array<u32, cop0RegsCount> GPRs;
            };
        };
        u32 perf0, perf1;

        u8** mapVirtualTLB(std::shared_ptr<mio::TLBCache>& virtTable);
        void resetCoP();
        void rectifyTimer(u32 pulseCycles);

        bool isCacheHit(u32 address, u8 lane);
        EECacheLine* viewLine(u32 address);
        u32 readCache(u32 address);
        void fillCacheWay(EECacheLine* line, u32 tag);
        void loadCacheLine(u32 address, EEMipsCore& eeCore);

        void invIndexed(u32 address);
        void loadTlbValues(mio::TLBPageEntry& entry);
        void enableInt();
        void disableInt();

        bool isAHVector(u32 pcValue);
        bool haveAException();
        void mtc0(u8 reg, u32 code);

        bool isIntEnabled();
    private:
        void incPerfByEvent(u32 mask, u32 cycles, u8 perfEv);
        EECacheLine* eeNearCache;
    };

    static_assert(sizeof(u32) * cop0RegsCount == 128);
}

