/**
 * linear scan register allocation
 */

#ifndef _LINEARSCAN_H__
#define _LINEARSCAN_H__
#include <list>
#include <map>
#include <set>
#include <vector>

class MachineUnit;
class MachineOperand;
class MachineFunction;

// TODO: allocate floating point registers

// extern bool* fcheck;
// struct range{
//     int r_start;
//     int r_end;
//     std::set<MachineOperand*> defs;
//     std::set<MachineOperand*> uses;
// };

class LinearScan {
   private:
    struct Interval {
        int start;
        int end;
        bool spill;  // whether this vreg should be spilled to memory
        int disp;    // displacement in stack
        int rreg;  // the real register mapped from virtual register if the vreg
                   // is not spilled to memory
        bool fpu;
        std::set<MachineOperand*> defs;
        std::set<MachineOperand*> uses;
        // std::vector<range> ranges;
    };
    MachineUnit* unit;
    MachineFunction* func;
    std::vector<int> fregs;
    std::vector<int> regs;
    std::map<MachineOperand*, std::set<MachineOperand*>> du_chains;
    std::vector<Interval*> intervals;
    std::vector<Interval*> active;
    static bool startSort(Interval* a, Interval* b){
        return a->start < b->start;
    };
    static bool endSort(Interval* a, Interval* b){
        return a->end < b->end;
    };
    void expireOldIntervals(Interval* interval);
    void expireOldIntervals_f(Interval* interval);
    void spillAtInterval(Interval* interval);
    void makeDuChains();
    void computeLiveIntervals();
    void computeLiveIntervals_f();
    bool linearScanRegisterAllocation();
    bool linearScanRegisterAllocation_f();
    void modifyCode();
    void genSpillCode();
    void genSpillCode_f();

   public:
    LinearScan(MachineUnit* unit,bool f);
    void allocateRegisters();
    void allocateRegisters_f();
};

#endif
