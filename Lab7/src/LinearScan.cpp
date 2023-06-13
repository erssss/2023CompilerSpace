#include "LinearScan.h"
#include <algorithm>
#include <iostream>
#include "LiveVariableAnalysis.h"
#include "MachineCode.h"

#define DEBUG_SWITCH_reg 0
#if DEBUG_SWITCH_reg
#define printReg(str) \
    std::cout << str << "\n";
#else
#define printReg(str) //
#endif

LinearScan::LinearScan(MachineUnit *unit, bool f)
{
    this->unit = unit;
    for (int i = 4; i < 11; i++)
    {
        regs.emplace_back(i);
    }
    if (f)
        for (int i = 5; i < 32; i++)
        {
            fregs.emplace_back(i + 16);
        }
}

void LinearScan::allocateRegisters()
{
    for (auto &f : unit->getFuncs())
    {
        func = f;
        bool success;
        success = false;
        while (!success)
        { // repeat until all vregs can be mapped
            computeLiveIntervals();
            success = linearScanRegisterAllocation();
            if (success) // all vregs can be mapped to real regs
                modifyCode();
            else // spill vregs that can'rrange be mapped to real regs
                genSpillCode();
        }
    }
}

void LinearScan::allocateRegisters_f()
{
    for (auto &f : unit->getFuncs())
    {
        func = f;
        bool success;
        success = false;
        while (!success)
        { // repeat until all vregs can be mapped
            computeLiveIntervals_f();
            success = linearScanRegisterAllocation_f();
            if (success) // all vregs can be mapped to real regs
                modifyCode();
            else // spill vregs that can'rrange be mapped to real regs
                genSpillCode_f();
        }
    }
}
void LinearScan::makeDuChains()
{
    LiveVariableAnalysis lives;
    lives.pass(func);
    du_chains.clear();
    int i = 0;
    std::map<MachineOperand, std::set<MachineOperand *>> liveVar;
    for (auto &bb : func->getBlocks())
    {
        liveVar.clear();
        for (auto &var : bb->getLiveOut())
        {
            liveVar[*var].insert(var);
        }
        int no;
        no = i = bb->getInsts().size() + i; // 这玩意儿为啥对性能有这么大影响………………
        for (auto instr = bb->getInsts().rbegin(); instr != bb->getInsts().rend(); instr++)
        {
            (*instr)->setNo(no--);
            for (auto &def : (*instr)->getDef())
            {
                if (def->isVReg())
                {
                    auto &uses = liveVar[*def];
                    du_chains[def].insert(uses.begin(), uses.end());
                    auto &kill = lives.getAllUses()[*def];
                    std::set<MachineOperand *> res;
                    std::set_difference(uses.begin(), uses.end(), kill.begin(),
                                        kill.end(), std::inserter(res, res.end()));
                    liveVar[*def] = res;
                }
            }
            for (auto &use : (*instr)->getUse())
            {
                if (use->isVReg())
                    liveVar[*use].insert(use);
            }
        }
    }
}

// 重载MachineOperand*的set排序
// bool operator < (MachineOperand* a,MachineOperand* b){
//     return a->getParent()->getNo() > b->getParent()->getNo();
// }

void LinearScan::computeLiveIntervals()
{
    makeDuChains();
    intervals.clear();

    std::vector<int> tmp_end;
    // std::vector<int> tmp_start;
    for (auto &du_chain : du_chains)
    {
        printReg("--------------------------");
        tmp_end.emplace_back(du_chain.first->getParent()->getNo());
        int maxno = -1;
        for (auto use : du_chain.second)
        {
            // min_no=std::min(min_no,use->getParent()->getNo());
            maxno = std::max(maxno, use->getParent()->getNo());
            if (use->getRange())
            {
                tmp_end.emplace_back(use->getParent()->getNo());
            }
        }
        tmp_end.emplace_back(maxno);
        if (tmp_end.size() <= 1)
        {
            printReg("start = " << du_chain.first->getParent()->getNo() << " end = " << maxno);
            Interval *interval = new Interval({du_chain.first->getParent()->getNo(),
                                               maxno,
                                               false,
                                               0,
                                               0,
                                               0,
                                               {du_chain.first},
                                               du_chain.second});
            intervals.emplace_back(interval);
        }
        else
        {
            printReg("else");
            std::vector<std::set<MachineOperand *>> ranges_uses(tmp_end.size());
            // std::sort(tmp_end.begin(),tmp_end.end());
            // std::set<MachineOperand *> tmp;
            for (auto use : du_chain.second)
            {
                for (size_t i = 0; i < tmp_end.size(); i++)
                {
                    if (use->getParent()->getNo() <= tmp_end[i])
                    {
                        //
                        ranges_uses[i].insert(use);
                    }
                }
            }
            // printReg("tmp_end.size() " << tmp_end.size());
            for (size_t i = 1; i < tmp_end.size(); i++)
            {
                printReg("start = " << tmp_end[i - 1] << " end = " << tmp_end[i]);
                if ((i != 1 && ranges_uses[i].size() <= 1))
                    continue;
                Interval *interval = new Interval({tmp_end[i - 1],
                                                   tmp_end[i],
                                                   false,
                                                   0,
                                                   0,
                                                   0,
                                                   {du_chain.first},
                                                   ranges_uses[i]});

                intervals.emplace_back(interval);
            }
            tmp_end.clear();
            ranges_uses.clear();
            // tmp_start.clear();
        }

        // int rangeCnt = (int)du_chain.first->getRange().size();
        // if(rangeCnt>1)
        // printReg("du_chain.first->getRange() = " << rangeCnt);
        // if (rangeCnt > 0)
        // {
        //     auto ranges = du_chain.first->getRange();
        //     std::set<MachineOperand *> *ranges_uses =
        //         new std::set<MachineOperand *>[rangeCnt];
        //     for (auto use : du_chain.second)
        //     {
        //         for (int i = 0; i < rangeCnt; i++)
        //         {
        //             if (ranges[i]->getNo() >= use->getParent()->getNo())
        //             {
        //                 ranges_uses[i].insert(use);
        //             }
        //         }
        //     }
        //     for (int i = 0; i < rangeCnt; i++)
        //     {
        //     // printReg("ranges[i]->getNo() = " << ranges[i]->getNo());
        //         Interval *interval = new Interval({du_chain.first->getParent()->getNo(),
        //                                            ranges[i]->getNo(),
        //                                            false,
        //                                            0,
        //                                            0,
        //                                            0,
        //                                            {du_chain.first},
        //                                            ranges_uses[i]});

        //         intervals.emplace_back(interval);
        //     }
        //     delete[] ranges_uses;
        // }
        // else
        // {
        //     int maxno = -1;
        //     for (auto &use : du_chain.second)
        //         maxno = std::max(maxno, use->getParent()->getNo());

        // Interval *interval = new Interval({du_chain.first->getParent()->getNo(),
        //                                    maxno,
        //                                    false,
        //                                    0,
        //                                    0,
        //                                    0,
        //                                    {du_chain.first},
        //                                    du_chain.second});

        // intervals.emplace_back(interval);
        // }
    }
    bool flag = 0;
    do 
    {
        flag=0;
        std::vector<Interval *> t(intervals.begin(), intervals.end());
        for (size_t i = 0; i < t.size(); i++)
            for (size_t j = i + 1; j < t.size(); j++)
            {
                Interval *w1 = t[i];
                Interval *w2 = t[j];
                if (**w1->defs.begin() == **w2->defs.begin())
                {
                    std::set<MachineOperand *> tmp;
                    set_intersection(w1->uses.begin(), w1->uses.end(),
                                     w2->uses.begin(), w2->uses.end(),
                                     inserter(tmp, tmp.end()));
                    if (!tmp.empty())
                    {
                        flag = 1;
                        w1->uses.insert(w2->uses.begin(), w2->uses.end());
                        w1->defs.insert(w2->defs.begin(), w2->defs.end());
                        w1->end = std::max(w1->end, w2->end);
                        w1->start = std::min(w1->start, w2->start);
                        auto it = std::find(intervals.begin(), intervals.end(), w2);
                        if (it != intervals.end())
                            intervals.erase(it);
                    }
                }
            }
    }while(flag);
    sort(intervals.begin(), intervals.end(), startSort);
}

void LinearScan::computeLiveIntervals_f()
{
    makeDuChains();
    intervals.clear();

    for (auto &du_chain : du_chains)
    {
        int maxno = -1;
        for (auto &use : du_chain.second)
        {
            maxno = std::max(maxno, use->getParent()->getNo());
        }
        Interval *interval = new Interval({du_chain.first->getParent()->getNo(),
                                           maxno,
                                           false,
                                           0,
                                           0,
                                           du_chain.first->isFloat(),
                                           {du_chain.first},
                                           du_chain.second});

        intervals.emplace_back(interval);
    }
    // std::vector<int> tmp_end;
    // // std::vector<int> tmp_start;
    // for (auto &du_chain : du_chains)
    // {
    //     printReg("--------------------------");
    //     tmp_end.emplace_back(du_chain.first->getParent()->getNo());
    //         int t = -1;
    //     for(auto use:du_chain.second){
    //         // min_no=std::min(min_no,use->getParent()->getNo());
    //         t = std::max(t, use->getParent()->getNo());
    //         if(use->getRange()){
    //             tmp_end.emplace_back(use->getParent()->getNo());

    //         }
    //     }
    //     tmp_end.emplace_back(t);
    //     if(tmp_end.size()<=1){
    //         for (auto& use : du_chain.second)
    //         printReg("start = "<<du_chain.first->getParent()->getNo()<<" end = "<<t);
    //         Interval* interval = new Interval({du_chain.first->getParent()->getNo(),
    //                                             t, false, 0, 0, 0,
    //                                            {du_chain.first},du_chain.second});
    //         auto ranges = du_chain.first->getRange();
    //         intervals.emplace_back(interval);
    //     }
    //     else{
    //         printReg("else");
    //     std::vector<std::set<MachineOperand *>> ranges_uses(tmp_end.size());
    //     // std::sort(tmp_end.begin(),tmp_end.end());
    //     // std::set<MachineOperand *> tmp;
    //     for(auto use:du_chain.second){
    //         for(size_t i=0;i<tmp_end.size();i++){
    //             if(use->getParent()->getNo()<=tmp_end[i]){
    //                 //
    //                 ranges_uses[i].insert(use);
    //             }
    //         }
    //     }
    //     // printReg("tmp_end.size() " << tmp_end.size());
    //     for(size_t i=1;i<tmp_end.size();i++){
    //         printReg("start = "<<tmp_end[i-1]<<" end = "<<tmp_end[i]);
    //         if(i!=1&&ranges_uses[i].size()<=1)continue;

    //         Interval *interval = new Interval({tmp_end[i-1],
    //                                            tmp_end[i],
    //                                            false, 0, 0, 0,
    //                                            {du_chain.first},
    //                                            ranges_uses[i]});

    //         intervals.emplace_back(interval);
    //     }
    //     tmp_end.clear();
    //     ranges_uses.clear();
    //     // tmp_start.clear();

    //     // Interval *interval = new Interval({du_chain.first->getParent()->getNo(),
    //     //                                    rrange,
    //     //                                    false,
    //     //                                    0,
    //     //                                    0,
    //     //                                    du_chain.first->isFloat(),
    //     //                                    {du_chain.first},
    //     //                                    du_chain.second});
    //     // intervals.emplace_back(interval);
    // }
    // }
    for (auto &interval : intervals)
    {
        auto uses = interval->uses;
        auto begin = interval->start;
        auto end = interval->end;
        for (auto block : func->getBlocks())
        {
            auto liveIn = block->getLiveIn();
            auto liveOut = block->getLiveOut();
            bool in = false;
            bool out = false;
            for (auto use : uses)
            {
                if (liveIn.count(use))
                {
                    in = true;
                    if (in && out)
                        break;
                }
                if (liveOut.count(use))
                {
                    out = true;
                    if (in && out)
                        break;
                }
            }

            if (in && out)
            {
                begin = std::min(begin, (*(block->begin()))->getNo());
                end = std::max(end, (*(block->rbegin()))->getNo());
            }
            else if (!in && out)
            {
                for (auto i : block->getInsts())
                    if (i->getDef().size() > 0 &&
                        i->getDef()[0] == *(uses.begin()))
                    {
                        begin = std::min(begin, i->getNo());
                        break;
                    }
                end = std::max(end, (*(block->rbegin()))->getNo());
            }
            else if (in && !out)
            {
                begin = std::min(begin, (*(block->begin()))->getNo());
                int tmp = 0;
                for (auto use : uses)
                {
                    if (use->getParent()->getParent() == block)
                        tmp = std::max(tmp, use->getParent()->getNo());
                }
                end = std::max(tmp, end);
            }
        }
        interval->start = begin;
        interval->end = end;
    }
    std::set<MachineOperand *> tmp;
    bool flag = 0;
    do
    {
        flag = 0;
        std::vector<Interval *> rrange(intervals.begin(), intervals.end());
        for (size_t i = 0; i < rrange.size(); i++)
            for (size_t j = i + 1; j < rrange.size(); j++)
            {
                Interval *r1 = rrange[i];
                Interval *r2 = rrange[j];
                if (**r1->defs.begin() == **r2->defs.begin())
                {
                    tmp.clear();
                    set_intersection(r1->uses.begin(), r1->uses.end(),
                                     r2->uses.begin(), r2->uses.end(),
                                     inserter(tmp, tmp.end()));
                    if (!tmp.empty())
                    {
                        flag = 1;
                        r1->defs.insert(r2->defs.begin(), r2->defs.end());
                        r1->uses.insert(r2->uses.begin(), r2->uses.end());
                        r1->start = std::min(std::min(r1->start, r1->end), std::min(r2->start, r2->end));
                        r1->end = std::max(std::max(r1->start, r1->end), std::max(r2->start, r2->end));
                        auto it = std::find(intervals.begin(), intervals.end(), r2);
                        if (it != intervals.end())
                        {
                            intervals.erase(it);
                        }
                    }
                }
            }
    } while (flag);
    sort(intervals.begin(), intervals.end(), startSort);
}
bool LinearScan::linearScanRegisterAllocation_f()
{
    bool flag = true;
    regs.clear();
    fregs.clear();
    active.clear();
    for (int i = 4; i < 11; i++){
        regs.emplace_back(i);
    }
    for (int i = 21; i < 48; i++)
    {
        fregs.emplace_back(i);
    }
    for (auto &intv : intervals)
    {
        expireOldIntervals_f(intv);
        if ((!intv->fpu && regs.empty()) || (intv->fpu && fregs.empty()))
        {
            spillAtInterval(intv);
            flag = false;
        }
        else
        {
            if (!intv->fpu)
            {
                intv->rreg = regs.front();
                regs.erase(regs.begin());
            }
            else
            {
                intv->rreg = fregs.front();
                fregs.erase(fregs.begin());
            }
            active.emplace_back(intv);
            sort(active.begin(), active.end(), endSort);
        }
    }
    return flag;
}
bool LinearScan::linearScanRegisterAllocation()
{
    active.clear();
    regs.clear();
    for (int i = 4; i < 11; i++){
        regs.emplace_back(i);
    }
    for (auto intv : intervals)
    {
        expireOldIntervals(intv);
        if (regs.empty())
        {
            spillAtInterval(intv);
            return 0;
        }
        else
        {
            intv->rreg = regs.front();
            active.emplace_back(intv);
            regs.erase(regs.begin());
            sort(active.begin(), active.end(), endSort);
        }
    }
    return 1;
}

void LinearScan::modifyCode()
{
    for (auto &interval : intervals)
    {
        func->addSavedRegs(interval->rreg);
        for (auto def : interval->defs)
            def->setReg(interval->rreg);
        for (auto use : interval->uses)
            use->setReg(interval->rreg);
    }
}

void LinearScan::genSpillCode()
{
    for (auto &interval : intervals)
    {
        if (!interval->spill)
            continue;
        // TODO
        /* HINT:
         * The vreg should be spilled to memory.
         * 1. insert ldr inst before the use of vreg
         * 2. insert str inst after the def of vreg
         */
        interval->disp = -func->AllocSpace(4);
        auto off = new MachineOperand(MachineOperand::IMM, interval->disp);
        auto fp = new MachineOperand(MachineOperand::REG, 11);
        for (auto use : interval->uses)
        {
            auto tmp = new MachineOperand(*use);
            MachineOperand *operand = nullptr;
            // 栈帧偏移大于255，重新load
            if (interval->disp > 255 || interval->disp < -255)
            {
                operand = new MachineOperand(MachineOperand::VREG,
                                             SymbolTable::getLabel());
                auto instr_ = new LoadMInstruction(use->getParent()->getParent(), LoadMInstruction::LDR,
                                                  operand, off);
                use->getParent()->insertBefore(instr_);
            }
            if (operand)
            {
                auto inst =
                    new LoadMInstruction(use->getParent()->getParent(), 
                            LoadMInstruction::LDR, tmp, fp, new MachineOperand(*operand));
                use->getParent()->insertBefore(inst);
            }
            else
            {
                auto inst = new LoadMInstruction(use->getParent()->getParent(), 
                        LoadMInstruction::LDR, tmp, fp, off);
                use->getParent()->insertBefore(inst);
            }
        }
        for (auto def : interval->defs)
        {
            MachineOperand *operand = nullptr;
            MachineInstruction *instr_ = nullptr, *inst = nullptr;
            auto tmp = new MachineOperand(*def);
            // 栈帧偏移大于255，重新load
            if (interval->disp > 255 || interval->disp < -255)
            {
                operand = new MachineOperand(MachineOperand::VREG,
                                             SymbolTable::getLabel());
                instr_ = new LoadMInstruction(def->getParent()->getParent(), 
                        LoadMInstruction::LDR, operand, off);
                def->getParent()->insertAfter(instr_);
            }
            if (operand)
                inst =
                    new StoreMInstruction(def->getParent()->getParent(), 
                            StoreMInstruction::STR, tmp, fp, new MachineOperand(*operand));
            else{
                inst = new StoreMInstruction(def->getParent()->getParent(), 
                        StoreMInstruction::STR, tmp, fp, off);
            }
            if (!instr_)
                def->getParent()->insertAfter(inst);
            else
                instr_->insertAfter(inst);
        }
    }
}
void LinearScan::genSpillCode_f()
{
    for (auto &interval : intervals)
    {
        if (!interval->spill)
            continue;
        /* HINT:
         * The vreg should be spilled to memory.
         * 1. insert ldr instr before the use of vreg
         * 2. insert str instr after the def of vreg
         */
        interval->disp = -func->AllocSpace(4);
        auto off = new MachineOperand(MachineOperand::IMM, interval->disp);
        auto fp = new MachineOperand(MachineOperand::REG, 11);
        for (auto use : interval->uses)
        {
            auto tmp = new MachineOperand(*use);
            MachineOperand *operand = nullptr;
            // 栈帧偏移大于255，重新load
            if (interval->disp > 255 || interval->disp < -255)
            {
                operand = new MachineOperand(MachineOperand::VREG,
                                             SymbolTable::getLabel());
                auto instr_ = new LoadMInstruction(use->getParent()->getParent(),
                                         LoadMInstruction::LDR, operand, off);
                use->getParent()->insertBefore(instr_);
            }
            if (operand)
            {
                if (!use->isFloat())
                {
                    auto instr = new LoadMInstruction(
                        use->getParent()->getParent(), LoadMInstruction::LDR,
                        tmp, fp, new MachineOperand(*operand));
                    use->getParent()->insertBefore(instr);
                }
                else
                {
                    auto reg = new MachineOperand(MachineOperand::VREG,
                                                  SymbolTable::getLabel());
                    MachineInstruction *instr = new BinaryMInstruction(
                        use->getParent()->getParent(), BinaryMInstruction::ADD,
                        reg, fp, new MachineOperand(*operand));
                    use->getParent()->insertBefore(instr);
                    instr = new LoadMInstruction(use->getParent()->getParent(),
                                                 LoadMInstruction::VLDR, tmp,
                                                 new MachineOperand(*reg));
                    use->getParent()->insertBefore(instr);
                }
            }
            else
            {
                if (!use->isFloat())
                {
                    auto instr = new LoadMInstruction(
                        use->getParent()->getParent(), LoadMInstruction::LDR,
                        tmp, fp, off);
                    use->getParent()->insertBefore(instr);
                }
                else
                {
                    auto instr = new LoadMInstruction(
                        use->getParent()->getParent(), LoadMInstruction::VLDR,
                        tmp, fp, off);
                    use->getParent()->insertBefore(instr);
                }
            }
        }
        for (auto def : interval->defs)
        {
            MachineOperand *operand = nullptr;
            MachineInstruction *instr = nullptr;
            MachineInstruction*instr_ = nullptr;
            MachineOperand* tmp = new MachineOperand(*def);
            // 栈帧偏移大于255，重新load
            if (interval->disp > 255 || interval->disp < -255)
            {
                operand = new MachineOperand(MachineOperand::VREG, SymbolTable::getLabel());
                instr_ = new LoadMInstruction(def->getParent()->getParent(),
                                         LoadMInstruction::LDR, operand, off);
                def->getParent()->insertAfter(instr_);
            }
            if (operand)
            {
                if (!def->isFloat())
                {
                    instr = new StoreMInstruction( def->getParent()->getParent(), 
                        StoreMInstruction::STR, tmp, fp, new MachineOperand(*operand));
                }
                else
                {
                    auto reg = new MachineOperand(MachineOperand::VREG,
                                                  SymbolTable::getLabel());
                    MachineInstruction *tmp_inst = new BinaryMInstruction(
                        def->getParent()->getParent(), BinaryMInstruction::ADD,
                        reg, fp, new MachineOperand(*operand));

                    instr_->insertAfter(tmp_inst);
                    instr_ = tmp_inst;

                    instr = new StoreMInstruction(def->getParent()->getParent(),
                                                  StoreMInstruction::VSTR, tmp,
                                                  new MachineOperand(*reg));
                }
            }
            else
            {
                if (!def->isFloat())
                {
                    instr = new StoreMInstruction(def->getParent()->getParent(),
                                                  StoreMInstruction::STR, tmp, fp, off);
                }
                else
                {
                    instr = new StoreMInstruction(def->getParent()->getParent(),
                                                  StoreMInstruction::VSTR, tmp, fp, off);
                }
            }
            if (!instr_)
                def->getParent()->insertAfter(instr);
            else
                instr_->insertAfter(instr);
        }
    }
}
void LinearScan::expireOldIntervals(Interval *interval)
{

    for(auto it = active.begin(); it != active.end();)
    {
        if ((*it)->end >= interval->start)
            return;
        regs.emplace_back((*it)->rreg);
        it = active.erase(find(active.begin(), active.end(), (*it)));
        sort(regs.begin(), regs.end());
    }
}
void LinearScan::expireOldIntervals_f(Interval *interval)
{
    for(auto it = active.begin(); it != active.end();){
        if((*it)->end >= interval->start){
            return;
        }
        if ((*it)->rreg < 11) {
            regs.emplace_back((*it)->rreg); // release
            it = active.erase(find(active.begin(), active.end(), *it));
            sort(regs.begin(), regs.end());
        }
        else{
            fregs.emplace_back((*it)->rreg); //release
            it = active.erase(find(active.begin(), active.end(), *it));
            sort(fregs.begin(), fregs.end()); 
        }
    }
}
void LinearScan::spillAtInterval(Interval *interval)
{

    Interval* intv_s = active.back();
    if (intv_s->end <= interval->end)
    {
        interval->spill = 1;
    }
    else
    {
        intv_s->spill = 1;
        interval->rreg = intv_s->rreg;
        active.emplace_back(interval);
        sort(active.begin(), active.end(), endSort);
    }
}
