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
const int MAX_FRAMES = 128;
const int MAX_VPAGES = 64;

class FrameTable{
    public:
    unsigned long virtual_address:32;
    unsigned long mapped:1;
    int pid;
    FrameTable();
};
FrameTable::FrameTable(){
    virtual_address = 0;
    mapped = 0;
    pid = -1;
}
FrameTable frame_table_array[MAX_FRAMES];
queue <FrameTable*> free_list;

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

};
class Process{
    public:
    int pid;
    vector <vector <int> > vma;
    PTE page_table[MAX_VPAGES];
    unsigned long segv, segprot, maps, unmaps, ins, outs, zeros, fins, fouts;
    Process(int);
};
Process::Process(int id){
    pid = id;
    // segv, segprot, maps, unmaps, ins, outs, zeros, fins, fouts = 0, 0, 0, 0, 0, 0, 0, 0, 0;

}
FrameTable *allocate_from_free_list (){
    for (int i = 0; i < sizeof(frame_table_array)/sizeof(*frame_table_array); i++){
        if (frame_table_array[i].mapped == 0){
            return &frame_table_array[i];
        }

    }
    return nullptr;
}
// class Pager {
//     virtual frame_t* select_victim_frame() = 0; // virtual base class
// };
// void simulation(){
//     typedef struct { ... } pte_t; // can only be total of 32-bit size and will check on this typedef struct { ... } frame_t;
//     frame_t frame_table[MAX_FRAMES];
    
//     frame_t *get_frame() {
//     frame_t *frame = allocate_frame_from_free_list();
//     if (frame == NULL) frame = THE_PAGER->select_victim_frame();
//         return frame;
//     }
//     while (get_next_instruction(&operation, &vpage)) {
//     // handle special case of “c” and “e” instruction // now the real instructions for read and write pte_t *pte = &current_process->page_table[vpage]; if ( ! pte->present) {
//     // this in reality generates the page fault exception and now you execute
//     // verify this is actually a valid page in a vma if not raise error and next inst frame_t *newframe = get_frame();
//     //-> figure out if/what to do with old frame if it was mapped
//     // see general outline in MM-slides under Lab3 header and writeup below
//     // see whether and how to bring in the content of the access page.
//     }
//     // check write protection
//     // simulate instruction execution by hardware by updating the R/M PTE bits update_pte(read/modify) bits based on operations.
// }
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
            printf("%d:%d", frame_table_array[i].pid, frame_table_array[i].virtual_address);
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
    ifstream file(argv[1]);
    string str;
    vector < vector < int > > total_vma;
    vector <int> vma;
    vector <Process*> process_ptr;
    vector <char> instruction_char;
    vector <int> instruction_int;
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
        success = sscanf(stoc, "%d %d %d %d", &start, &end, &w_p, &m);
        // if (num_vma != 0){
        if (success == 4){

            vma.push_back(start);
            vma.push_back(end);
            vma.push_back(w_p);
            vma.push_back(m);
            total_vma.push_back(vma);
            
            // for (int i = 0; i < vma.size(); i++){
            //     cout<< vma[i] << " ";
            // }           
            // cout << endl;
            vma.clear();
            // for (int i = 0; i < total_vma.size(); i++){
            //     for (int j = 0; j < total_vma[i].size(); j++){
            //         cout<< total_vma[i][j] << " ";
            //     }
            //     cout<< endl;
            // }
            num_vma --;
            // cout << "num_vma " << num_vma << endl;

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

    return 0;
}