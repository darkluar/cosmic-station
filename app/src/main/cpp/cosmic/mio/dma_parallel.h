#pragma once
#include <array>
#include <queue>

#include <common/types.h>
#include <mio/blocks.h>
namespace cosmic::mio {
    enum DirectChannels {
        Vif0, Vif1,
        Gif,
        IpuFrom, IpuTo,
        Sif0, Sif1, Sif2,
        SprFrom,
        SprTo
    };

    struct DmaChannel {
        bool request{false};
        u8 index;
    };
    struct DMARegister {
        u32 address;
        u32 value;
        bool writeBack{false};

        auto operator=(u32 vReg) {
            value = vReg;
            writeBack = true;
        }
        auto operator*() {
            return value;
        }
    };

    class DmaController {
    public:
        DmaController();

        void resetMA();
        void pulse(u32 cycles);
        u32 performRead(u32 address);
        void issueADmacRequest(DirectChannels channel);

        std::shared_ptr<GlobalMemory> memoryMapped;
    private:
        std::queue<DmaChannel> fifoChannels;
        u32 intStatus;
        union {
            std::array<DmaChannel, 0x9> channels;
            DMARegister priorityCtrl{0x1f8010f0};
        };
    };
}
