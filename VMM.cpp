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
int hand = 0;



class PTE{
    public:
    unsigned int present:1;
    unsigned int referenced:1;
    unsigned int modified:1;
    unsigned int write_protect:1;
    unsigned int paged_out:1;
    unsigned int mapped:1;
    unsigned int physical_frame:7;
    PTE();
};
PTE::PTE(){
    present = 0;
    referenced = 0;
    modified = 0; 
    write_protect = 0; 
    paged_out = 0;
    mapped = 0; 
    physical_frame = 0;
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
class FrameTable{
    public:
    unsigned long virtual_address:32;
    unsigned long mapped:1;
    Process *p;
    FrameTable();
};
FrameTable::FrameTable(){
    virtual_address = 0;
    mapped = 0;
    p = nullptr;
}
FrameTable frame_table_array[MAX_FRAMES];
queue <FrameTable*> free_list;
// queue <FrameTable*> used_list;
FrameTable *get_frame (){
    // for (int i = 0; i < sizeof(frame_table_array)/sizeof(*frame_table_array); i++){
    //     if (frame_table_array[i].mapped == 0){
    //         return &frame_table_array[i];
    //     }

    // }
    FrameTable *f_ptr;
    if (!free_list.empty()){
        f_ptr  = free_list.front();
        free_list.pop();
        // used_list.push(f_ptr);
        return f_ptr;
    }
    
    return nullptr;
}
class Pager {
    public:
    virtual FrameTable* select_victim_frame() = 0; // virtual base class
};
class FIFO: public Pager{
    FrameTable  *select_victim_frame(){
        int flag = 0;
        if (hand >= sizeof(frame_table_array) / sizeof(*frame_table_array)){
            hand = -1;
            // frame_table_array[hand].mapped = 0;
        }
        
        hand++;
        for (int i = 0; i < sizeof(frame_table_array[hand].p->page_table) / sizeof(*frame_table_array[hand].p->page_table); i++){
            for (int j = 0; j < sizeof(frame_table_array[hand].p->page_table) / sizeof (*frame_table_array[hand].p->page_table); j++){
                if ( frame_table_array[hand].p->page_table[j].physical_frame == hand ){
                    frame_table_array[hand].p->page_table[j].mapped = 0;
                    if (frame_table_array[hand].p->page_table[j].modified == 1){
                        frame_table_array[hand].p->outs++;
                        cout<< "OUT" << endl;
                    }
                    else if (frame_table_array[hand].p->page_table[j].referenced == 1){
                        frame_table_array[hand].p->fouts++;
                        cout << "FOUT" << endl;
                    }
                    frame_table_array[hand].p->page_table[j].referenced = 0;
                    frame_table_array[hand].p->page_table[j].modified = 0;
                    frame_table_array[hand].p->page_table[j].paged_out = 1; 
                    frame_table_array[hand].p->page_table[j].write_protect = 0;
                    frame_table_array[hand].p->page_table[j].present = 0;
                    frame_table_array[hand].p->page_table[j].mapped = 0;
                    frame_table_array[hand].p = nullptr;
                    flag = 1;
                    break;
                    
                }
            }
            if (flag == 1){
                break;
            }
        }
        return &frame_table_array[hand];

    }
};

// void simulation(Pager *the_Pager){
 
// }
bool can_vpage_accessed(Process *ptr, int vpage){
    for (int j = 0; j < ptr->vma.size(); j++){
        if (vpage >= ptr->vma[j]->start_page && vpage <= ptr->vma[j]->end_page ){
            return true;
        }
    }
    return false;
}
void print_stats(Process* proc){

    printf("PROC[%d]: U=%lu M=%lu I=%lu O=%lu FI=%lu FO=%lu Z=%lu SV=%lu SP=%lu\n",
                     proc->pid,
                     proc->unmaps, proc->maps, proc->ins, proc->outs,
                     proc->fins, proc->fouts, proc->zeros,
                     proc->segv, proc->segprot);
}
void print_frame_t(){
    for (int i = 0; i < sizeof(frame_table_array)/sizeof(*frame_table_array); i++){
        if (frame_table_array[i].mapped == 0){
            cout << "* ";
        }
        else{
            printf("%d:%d", frame_table_array[i].p->pid, frame_table_array[i].virtual_address);
        }
        
    }
}
void print_page_t(vector<Process*> process_ptr){
    for (int i = 0; i < process_ptr.size();  i++){
        printf("PT[%d]:", i);
        for (int j =  0; j < sizeof(process_ptr[i]->page_table)/sizeof(*process_ptr[i]->page_table); j++){
            char ref;
            char swap;
            char modify;
            if (process_ptr[i] -> page_table[j].present == 0 ){
                if (process_ptr[i] -> page_table[j].paged_out == 1){
                    cout << "# ";
                    continue;
                }
                else{
                    cout << "* ";
                    continue;
                }
            }
            if (process_ptr[i] -> page_table[j].modified == 1){
                modify = 'M';
            }
            else{
                modify = '-';
            }
            if (process_ptr[i] -> page_table[j].referenced == 1){
                modify = 'R';
            }
            else{
                modify = '-';
            }
            if (process_ptr[i] -> page_table[j].paged_out == 1){
                modify = 'S';
            }
            else{
                modify = '-';
            }

            printf("%d:%c%c%c", j, ref, modify, swap );
        }
        cout << endl;
    }
}
int main(int argc, char** argv){
    // MAX_FRAMES = 16;
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

            // vma.push_back(start);
            // vma.push_back(end);
            // vma.push_back(w_p);
            // vma.push_back(m);
            
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
    for (int i = 0; i < sizeof(frame_table_array)/sizeof(*frame_table_array); i++){
        free_list.push(&frame_table_array[i]);
    }
    // for (int i = 0; i < instruction_char.size(); i++){
    //     cout << instruction_char[i] << " " << instruction_int[i] << endl;
    // }

    // }
    // cout << "frame_table_array " << frame_table_array[0].pid << endl;
    // print_frame_t();
    Pager *the_Pager = new FIFO;
    // cout << "f_size " << sizeof(frame_table_array) / sizeof(*frame_table_array);
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ simulation
    FrameTable *frame = get_frame();
    if (frame == nullptr){
        frame = the_Pager->select_victim_frame();
    }
    Process *ptr;
    // cout << "instruction_char "<< instruction_char.size() << endl;
    for (int i = 0; i < instruction_char.size(); i++){
        printf("%d: ==> %c %d \n", i, instruction_char[i], instruction_int[i]);
        if (instruction_char[i] == 'c'){
            for (int j = 0; j < process_ptr.size(); j++){
                if (process_ptr[j]->pid == instruction_int[i]){
                    ptr = process_ptr[j];
                    break;
                }
            }
        }
        if (!can_vpage_accessed(ptr, instruction_int[i])){
            cout<< "SEGV" <<endl;
            ptr->segv++;
            continue;

        }
        
    }
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