#pragma once

#include <common/types.h>
#include <vu/vecu.h>
namespace cosmic::vu {
    // Just a communication interface between these two VUs
    class MacroModeCop2 {
    public:
        MacroModeCop2(Ref<vu::VectorUnit> vus[2]);
        void clearInterlock();
        bool checkInterlock();
        bool interlockCheck(bool isCop2);

        u32 cfc2(u32 special);
        void ctc2(u32 special, u32 value);

        Ref<vu::VectorUnit> v0;
        Ref<vu::VectorUnit> v1;
        bool cop2il,
            vuIl;
    };
}

