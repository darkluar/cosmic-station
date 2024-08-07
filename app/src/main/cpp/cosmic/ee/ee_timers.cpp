#include <common/except.h>
#include <ee/ee_timers.h>
#include <ee/ee_intc.h>
#include <console/intc.h>

namespace cosmic::ee {
    EeTimers::EeTimers(std::shared_ptr<vm::Scheduler>& solver,
        std::shared_ptr<console::IntCInfra>& infra) :
            scheduler(solver), intc(infra) {
        std::memset(&timers, 0, sizeof(timers));

    }
    void EeTimers::resetTimerCounter(HwTimer& timer) {
        static const std::vector<u64> zero{0};
        timer.count = {};
        scheduler->modifyTimerSet(timer.callId, vm::Counter, zero);
    }
    bool EeTimers::isTimerEnabled(HwTimer& timer) {
        return !timer.gated && timer.isEnabled;
    }
    // PAL:  312 scanlines per frame (VBOFF: 286 | VBON: 26)
    // NTSC: 262 scanlines per frame (VBOFF: 240 | VBON: 22)

    // PAL:  9436 BUSCLK cycles per scanline
    // NTSC: 9370 BUSCLK cycles per scanline
    void EeTimers::sysCtrlGate(bool vb, bool high) {
        for (u8 timerId{}; timerId < timers.size(); timerId++) {
            auto& clocked{timers[timerId]};
            if (!clocked.isGateEnabled && clocked.withVbSync != vb)
                continue;

            switch (clocked.gateMode) {
            case ActivateGate:
                clocked.gated = high; break;
            case ResetGateWhenHigh:
                if (high)
                    resetTimerCounter(clocked);
                clocked.gated = {};
                break;
            case ResetGateWhenLow:
                if (!high)
                    resetTimerCounter(clocked);
                clocked.gated = true;
                break;
            case ResetGateWithDiffer:
                // 3=Reset counter for high<->low gate transitions
                if (clocked.gated == high)
                    break;
                resetTimerCounter(clocked);
                clocked.gated = high;
            }
            std::vector<u64> pause{isTimerEnabled(clocked)};
            scheduler->modifyTimerSet(clocked.callId, vm::Pause, pause);
        }
    }
    void EeTimers::resetTimers() {
        for (u8 chronos = {}; chronos != timers.size(); chronos++) {
            // Not necessary perhaps, it will depend on the implementation
            timers.at(chronos) = {};
        }
        raiseId = scheduler->createSchedTick(false,
            [this](u8 rice, bool over) {
                raiseClkTrigger(rice, over);
        });
        const u16 compareDiff{0xffff};
        for (u8 idx = {}; idx != timers.size(); idx++) {
            auto task{
                scheduler->placeTickedTask(raiseId, compareDiff, std::make_tuple(idx, false), false)
            };
            if (task) {
                timers[idx].callId = *task;
            }
        }

    }
    void EeTimers::raiseClkTrigger(u8 raised, bool overflow) {
        // This function is responsible for enabling the clock timer exception;
        // it should ensure that the 'overflow' flag is enabled
        static u8 base{};

        base = ee::T0 + raised;
        auto& ticked{timers.at(raised)};
        if (!overflow) {
            if (ticked.clearCountWithDiff) {
                ticked.count = {};
            }
            if (!ticked.compare) {
                ticked.compare = true;
                intc->trapIrq(console::EeInt, base);
            }
        } else if (!ticked.overflow) {
            ticked.overflow = true;
            intc->trapIrq(console::EeInt, base);
        }
    }
}

