#include <cstdlib>
#include<sstream>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include<algorithm>
#include <iterator>
#include <typeinfo>
#include <queue>
#include <unistd.h>
#include <list>
// #include <tuple>

using namespace std;

const int MAX_FRAMES = 16;
const int MAX_VPAGES = 64;
int hand;
unsigned long ctx_switches, process_exits;
unsigned long long cost;



class PTE{
    public:
    unsigned int present:1;
    unsigned int referenced:1;
    unsigned int modified:1;
    unsigned int write_protect:1;
    unsigned int paged_out:1;
    unsigned int file_mapped:1;
    unsigned int accessed:1;
    unsigned int unmapped:1;
    unsigned int physical_frame:7;
    
    PTE();
};
PTE::PTE(){
    present = 0;
    referenced = 0;
    modified = 0; 
    write_protect = 0; 
    paged_out = 0;
    file_mapped = 0; 
    physical_frame = 0;
    accessed = 0;
    unmapped = 0;
}
class VMA{
    public:
    unsigned int start_page;
    unsigned int end_page;
    unsigned int write_protected:1;
    unsigned int filemapped:1;
    VMA(unsigned int, unsigned int, unsigned int, unsigned int);
};
VMA::VMA(unsigned int start, unsigned int end, unsigned int w_r, unsigned int f_mapped){
    start_page = start;
    end_page = end;
    write_protected = w_r;
    filemapped = f_mapped;
}
class Process{
    public:
    int pid;
    // vector <vector <int> > vma;
    vector <VMA*> vma;
    PTE page_table[MAX_VPAGES];
    unsigned long segv, segprot, maps, unmaps, ins, outs, zeros, fins, fouts;
    Process(int);
};
Process::Process(int id){
    pid = id;
    // segv, segprot, maps, unmaps, ins, outs, zeros, fins, fouts = 0, 0, 0, 0, 0, 0, 0, 0, 0;

}
class Frame{
    public:
    int frame_index;
    unsigned long virtual_address:32;
    unsigned long mapped:1;
    Process *p;
    Frame();
};
Frame::Frame(){
    virtual_address = 0;
    mapped = 0;
    p = nullptr;
}
Frame frame_table[MAX_FRAMES];
queue <Frame*> free_list;
// queue <FrameTable*> used_list;
Frame *allocate_frame (){
    // for (int i = 0; i < sizeof(frame_table_array)/sizeof(*frame_table_array); i++){
    //     if (frame_table_array[i].mapped == 0){
    //         return &frame_table_array[i];
    //     }

    // }
    Frame *f_ptr;
    if (!free_list.empty()){
        f_ptr  = free_list.front();
        free_list.pop();
        // used_list.push(f_ptr);
        return f_ptr;
    }
    
    return nullptr;
}
void unmap(PTE *pte, Process *ptr, int vpage){
    // for (int i = 0; i < ptr->vma.size(); i++){
    //     if (vpage >= ptr->vma[i]->start_page && vpage <= ptr->vma[i]->end_page ){

    //     }
    // }
    cost += 400;
    printf(" UNMAP %d:%d\n", ptr->pid, vpage);
    if (pte->modified){
        pte->paged_out = 1;
        if (pte -> file_mapped){
            cout << " FOUT" << endl;
            cost += 2400;
            ptr->fouts++;
        }
        else{
            cout<< " OUT" << endl;
            cost += 2700;
            ptr->outs++;
            
        }
        pte->modified = 0;
        pte -> referenced = 0;
        pte->physical_frame = 0;
        pte ->present = 0;
        pte->unmapped = 1;
        
    }
}
class Pager {
    public:
    virtual Frame* select_victim_frame() = 0; // virtual base class
};
class FIFO: public Pager{
    Frame  *select_victim_frame(){
        int flag = 0;
        
        if (hand >= sizeof(frame_table) / sizeof(*frame_table) - 1){
            hand = -1;
        }
        
        hand++;
        // for (int i = 0; i < sizeof(frame_table[hand].p->page_table) / sizeof(*frame_table[hand].p->page_table); i++){
        // for (int j = 0; j < sizeof(frame_table[hand].p->page_table) / sizeof (*frame_table[hand].p->page_table); j++){
        //     if ( frame_table[hand].p->page_table[j].physical_frame == hand ){
        //         frame_table[hand].p->page_table[j].mapped = 0;
        //         if (frame_table[hand].p->page_table[j].modified == 1){
        //             frame_table[hand].p->outs++;
        //             cout<< "OUT" << endl;
        //         }
        //         else if (frame_table[hand].p->page_table[j].referenced == 1){
        //             frame_table[hand].p->fouts++;
        //             cout << "FOUT" << endl;
        //         }
        //         frame_table[hand].p->page_table[j].referenced = 0;
        //         frame_table[hand].p->page_table[j].modified = 0;
        //         frame_table[hand].p->page_table[j].paged_out = 1; 
        //         frame_table[hand].p->page_table[j].write_protect = 0;
        //         frame_table[hand].p->page_table[j].present = 0;
        //         frame_table[hand].p->page_table[j].mapped = 0;
        //         frame_table[hand].p = nullptr;
        //         flag = 1;
        //         break;
                
        //     }
        // }
        //     if (flag == 1){
        //         break;
        //     }
        // }
        frame_table[hand].p->unmaps++;
        PTE *pte = &frame_table[hand].p->page_table[frame_table[hand].virtual_address];
        unmap(pte, frame_table[hand].p, frame_table[hand].virtual_address);
        
        frame_table[hand].p->page_table[frame_table[hand].virtual_address].present = 0;
        frame_table[hand].p = nullptr;
        frame_table[hand].virtual_address = 0;
        return &frame_table[hand];

    }
};


// void simulation(Pager *the_Pager){
 
// }
void page_fault(Process *ptr, PTE *pte, Pager *the_Pager, int vpage){
    Frame *frame;
    frame = allocate_frame();
    if (frame == nullptr){
        frame = the_Pager->select_victim_frame();
    }
    // pte->mapped = 1;
    pte->physical_frame = frame->frame_index;
    pte->present = 1;
    frame->p = ptr;
    frame->virtual_address = vpage;
    cout << " MAP " << frame->frame_index << endl;
}
// void mapping(PTE *pte, Process *ptr, Frame *frame){
    
// }
void set_accessed_bit(Process *ptr, int vpage){
    for (int j = 0; j < ptr->vma.size(); j++){
        if (vpage >= ptr->vma[j]->start_page && vpage <= ptr->vma[j]->end_page ){
            // return true;
            ptr -> page_table[vpage].accessed = 1;
        }
    }
    // return false;
}
void print_stats(vector <Process*> proc){
    for (int i = 0; i < proc.size(); i++){
        printf("PROC[%d]: U=%lu M=%lu I=%lu O=%lu FI=%lu FO=%lu Z=%lu SV=%lu SP=%lu\n",
                            proc[i]->pid,
                            proc[i]->unmaps, proc[i]->maps, proc[i]->ins, proc[i]->outs,
                            proc[i]->fins, proc[i]->fouts, proc[i]->zeros,
                            proc[i]->segv, proc[i]->segprot);
    }
    
}
void print_frame_t(){
    cout << "FT: " ;
    for (int i = 0; i < sizeof(frame_table)/sizeof(*frame_table); i++){
        if (frame_table[i].p == nullptr){
            cout << "* ";
        }
        else{
            printf("%d:%d ", frame_table[i].p->pid, frame_table[i].virtual_address);
        }
        
    }
    cout << endl;
}
void print_page_t(vector<Process*> process_ptr){
    for (int i = 0; i < process_ptr.size();  i++){
        printf("PT[%d]: ", i);
        for (int j =  0; j < sizeof(process_ptr[i]->page_table)/sizeof(*process_ptr[i]->page_table); j++){
            char ref;
            char swap;
            char modify;
            if (! process_ptr[i] -> page_table[j].present){
                if (process_ptr[i] -> page_table[j].paged_out){
                    cout << "# ";
                    continue;
                }
                else{
                    cout << "* ";
                    continue;
                }
            }
            else{
                if (process_ptr[i] -> page_table[j].modified){
                    modify = 'M';
                }
                else {
                    modify = '-';
                }
                if (process_ptr[i] -> page_table[j].referenced){
                    ref = 'R';
                }
                else{
                    ref = '-';
                }
                if (process_ptr[i] -> page_table[j].paged_out == 1){
                    swap = 'S';
                }
                else{
                    swap = '-';
                }
            }

            printf("%d:%c%c%c ", j, ref, modify, swap );
        }
        cout << endl;
    }
}
void set_bits(Process *ptr, unsigned int vpage){
    for (int j = 0; j < ptr->vma.size(); j++){
        if (vpage >= ptr->vma[j]->start_page && vpage <= ptr->vma[j]->end_page ){
            ptr -> page_table[vpage].accessed = 1;
            if (ptr->vma[j]->filemapped ){
                // return true;
                ptr -> page_table[vpage].file_mapped = 1;
            }
            if (ptr->vma[j]->write_protected ){
                ptr -> page_table[vpage].write_protect = 1;
            }
            break;

        }
    }
    // return false;

}
void set_write_protected_bit(Process *ptr, unsigned int vpage){
    for (int j = 0; j < ptr->vma.size(); j++){
        if (vpage >= ptr->vma[j]->start_page && vpage <= ptr->vma[j]->end_page ){
            if (ptr->vma[j]->write_protected ){
                // return true;
                ptr -> page_table[vpage].write_protect = 1;
            }
        }
    }
    // return false;

}

int main(int argc, char** argv){
    // MAX_FRAMES = 16;
    cout << sizeof(frame_table) / sizeof(*frame_table) - 1;
    hand = -1;
    vector <char> instruction_char;
    vector <int> instruction_int;
    ifstream file(argv[1]);
    string str;
    // vector < vector < int > > total_vma;
    vector <VMA*> total_vma;
    vector <int> vma;
    vector <Process*> process_ptr;
    int process_number= 0;
    int flag = 0;
    int num_p = -1;
    int num_vma = -1;

    while (getline(file, str))
    {
        // cout << "this " << endl;
        int n = str.length();
        char stoc [n + 1];
        strcpy(stoc, str.c_str());
        
        int success;
        // int a, b, c, d;
        int start, end, w_p, m;
        // if (success == 4){
        success = sscanf(stoc, "%u %u %u %u", &start, &end, &w_p, &m);
        // if (num_vma != 0){
        if (success == 4){

            
            VMA *v;
            v = new VMA(start, end, w_p, m);
            total_vma.push_back(v);
            // total_vma.push_back(vma);
            // vma.clear();
            num_vma --;

        }           
           
        // }        
        else if (success == 1){
            if (flag == 0){
                sscanf(stoc, "%d", &num_p);
                flag++;
            }
            else{
                sscanf(stoc, "%d", &num_vma);
                // cout << "num_vma " << num_vma << endl;
                 
            }
        }
        if ( process_number == num_p){
            char c;
            int i;
            sscanf(stoc, "%s %d", &c, &i);
            if (c == 'r' || c == 'c' || c == 'w' || c == 'e'){
                instruction_char.push_back(c);
                instruction_int.push_back(i);
            }
            
        }
        else if (num_vma == 0 && success == 4){
            // to do: build a new Process class
            // cout << "process _number" << process_number << endl;
            
            Process *pcb;
            pcb =  new Process(process_number);
            for (int i = 0; i < total_vma.size(); i++){
                pcb->vma.push_back(total_vma[i]);
            }
            process_ptr.push_back(pcb);
            // cout << pcb -> pid << endl;
            // for (int i = start; i < end; i ++){
            //     PTE p = new PTE();
            //     pcb->page_table.[i] = *p;
                
            // }
            process_number++;
            total_vma.clear();
            
        } 

    }
    // for (int i = 0; i < process_ptr.size(); i++){
    //     cout<< "pid: " <<process_ptr[i] -> pid << endl;
    //     for (int j = 0; j < process_ptr[i] -> vma.size(); j++){
    //         for (int k = 0; k < process_ptr[i] -> vma[j].size(); k++){
    //             cout << process_ptr[i] -> vma[j][k] << " ";
    //             // cout << "is mapped: " << process_ptr[i] -> page_table->mapped << endl;
    //         }
    //         cout<< endl;
    //     }
    // }
    cout << endl;
    for (int i = 0; i < sizeof(frame_table)/sizeof(*frame_table); i++){
        frame_table[i].frame_index = i;
        free_list.push(&frame_table[i]);
    }
    Pager *the_Pager = new FIFO;
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ simulation
    Process *current_process;
    // cost += instruction_char.size() - 1;
    for (int i = 0; i < instruction_char.size(); i++){
        printf("%d: ==> %c %d \n", i, instruction_char[i], instruction_int[i]);
        if (instruction_char[i] == 'c'){
            ctx_switches++;
            cost+= 130;
            for (int j = 0; j < process_ptr.size(); j++){
                if (process_ptr[j]->pid == instruction_int[i]){
                    current_process = process_ptr[j];
                    break;
                }
            }
            continue;
        }
        PTE *pte = &current_process->page_table[instruction_int[i]];
        if ((!pte->paged_out) && (!pte->present)){
            set_bits(current_process, instruction_int[i]);
        }
        if (!pte->accessed){
            cost += 340;
            cout<< " SEGV" <<endl;
            current_process->segv++;
            cost++;
            continue;
        }
        if ( ! pte->present) {
            Frame *frame;
            frame = allocate_frame();
            if (frame == nullptr){
                frame = the_Pager->select_victim_frame();
            }
            // pte->mapped = 1;
            pte->physical_frame = frame->frame_index;
            pte->present = 1;
            frame->p = current_process;
            frame->virtual_address = instruction_int[i];
            if((! pte->paged_out) && (! pte->file_mapped)){
                cout << " ZERO"<< endl;
                cost += 140;
                current_process->zeros++;
                
            }
            else if (pte ->paged_out && !pte ->file_mapped){
                cout << " IN" << endl;
                cost += 3100;

                current_process->ins++;
            }
            else if (pte ->file_mapped){
                cout << " FIN" << endl;
                cost += 2800;
                current_process->fins++;
            }
            // page_fault(current_process, pte, the_Pager, instruction_int[i]);
            cout << " MAP " << frame->frame_index << endl;
            cost += 300;
            current_process->maps++;
            // else if (pte -> mapped){
            //     cout << 
            // }
        }
        if (instruction_char[i] == 'r'){
            pte->referenced = 1;
            cost ++;
        }
        else if (instruction_char[i] == 'w' && !pte ->write_protect){
            pte -> referenced = 1;
            pte->modified = 1;
            cost ++;
        }
        else if (instruction_char[i] == 'w' && pte ->write_protect){
            pte -> referenced = 1;
            current_process -> segprot++;
            cout << " SEGPROT" << endl;
            cost ++;
            cost += 420;
            
        }

        
    }
    print_page_t(process_ptr);
    print_frame_t();
    print_stats(process_ptr);
    printf("TOTALCOST %lu %lu %lu %llu %lu\n",
              instruction_char.size(), ctx_switches, process_exits, cost, sizeof(process_ptr[0]->page_table) /MAX_VPAGES);
    // while (get_next_instruction(&operation, &vpage)) {
    // // handle special case of “c” and “e” instruction // now the real instructions for read and write pte_t *pte = &current_process->page_table[vpage]; if ( ! pte->present) {
    // // this in reality generates the page fault exception and now you execute
    // // verify this is actually a valid page in a vma if not raise error and next inst frame_t *newframe = get_frame();
    // //-> figure out if/what to do with old frame if it was mapped
    // // see general outline in MM-slides under Lab3 header and writeup below
    // // see whether and how to bring in the content of the access page.
    // }
    // check write protection
    // simulate instruction execution by hardware by updating the R/M PTE bits update_pte(read/modify) bits based on operations.


    return 0;
}