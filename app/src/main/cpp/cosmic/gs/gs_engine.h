#pragma once

#include <common/types.h>
#include <os/neon_simd.h>
#include <os/mapped.h>
#include <gamedb/title_patches.h>
#include <mio/mem_pipe.h>
namespace cosmic::gs {
    struct GsPayloadDataPacket {
        GsPayloadDataPacket() = default;

        GsPayloadDataPacket(u64 bufferSize) :
            downloadBuffer(bufferSize) {}
        u32 qw128Count;
        os::MappedMemory<os::vec> downloadBuffer;
        u32 indexAddr;

        os::vec consume() {
            auto data{downloadBuffer[indexAddr]};
            indexAddr++;
            qw128Count--;
            return data;
        }
        operator bool() const {
            return downloadBuffer;
        }
    };
    enum GsRegisters {
        GsBusDir
    };
    enum RegDesc {
        Primitive,
        RgBaQ,
        StPos,
        UvPos,
        Xyz2,

        Fog = 0xa,
        Ad = 0xe,

        Nop
    };

    union RgBaQReg {
        u64 rainbow;
        struct {
            u8 r, g, b, a;
            f32 gsq;
        };
    };
    union CoordinatesXyz {
        u64 xyz;
        struct {
            u16 x, y;
            u32 z;
        };
    };

    class GsEngine {
    public:
        GsEngine(std::shared_ptr<mio::MemoryPipe>& memory) :
            shared(memory) {
        }
        ~GsEngine() {
        }

        void resetGraphics();
        std::tuple<bool, os::vec> readGsData();
        bool isStalled();
        u32 privileged(GsRegisters gsr) const;

        bool isSoftwareMode{};
        void gsWrite(u32 addr, u64 data);

        struct {
            // Must be set appropriately for GIF->VRAM and VRAM->GIF
            u8 busDir;
        } gsPrivateRegs;

        gamedb::SwitchPatches gswAddrAlias{};
        std::shared_ptr<mio::MemoryPipe> shared;
    private:
        GsPayloadDataPacket videoBuffer{};

        // Internal registers (accessible via GIF)
        u64 prim;
        RgBaQReg palette{};
        std::pair<f32, f32> st;
        std::pair<u16, u16> uv;
        CoordinatesXyz xyz2;
        u8 fog;
        u64 framesCount;

        void writePrimitive(u64 primitive);
    };
}
