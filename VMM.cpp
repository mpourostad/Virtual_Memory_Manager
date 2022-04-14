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
#include <limits>
// #include <tuple>

using namespace std;

const int MAX_FRAMES = 128;
const int MAX_VPAGES = 64;
int frame_size;
int hand;
unsigned long ctx_switches, process_exits;
unsigned long long cost;
int count_inst;
vector <string> randvals;
int ofs;
bool ohno;




class PTE{
    public:
    unsigned int present:1;
    unsigned int referenced:1;
    unsigned int modified:1;
    unsigned int write_protect:1;
    unsigned int paged_out:1;
    unsigned int file_mapped:1;
    unsigned int accessed:1;
    unsigned int mapped:1;
    unsigned int setBits_flag:1;
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
    mapped = 0;
    setBits_flag = 0;
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
    unsigned long segv;
    unsigned long segprot;
    unsigned long maps;
    unsigned long unmaps;
    unsigned long ins;
    unsigned long outs;
    unsigned long zeros;
    unsigned long fins;
    unsigned long fouts;
    Process(int);
};
Process::Process(int id){
    pid = id;
    segv = 0;
    segprot = 0;
    maps = 0;
    unmaps = 0;
    ins = 0;
    outs = 0;
    zeros = 0;
    fins = 0;
    fouts = 0;

}
class Frame{
    public:
    int frame_index;
    unsigned long virtual_address:32;
    unsigned long mapped:1;
    unsigned int age;
    unsigned long current_time;
    Process *p;
    Frame();
};
Frame::Frame(){
    virtual_address = 0;
    mapped = 0;
    age = 0;
    current_time = 0;
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
    if (ohno){
        printf(" UNMAP %d:%d\n", ptr->pid, vpage);
    }
    
    if (pte->modified){
        // pte->paged_out = 1;
        if (pte -> file_mapped){
            if (ohno){
                 cout << " FOUT" << endl;
            }
           
            cost += 2400;
            ptr->fouts++;
        }
        else{
            pte->paged_out = 1;
            if (ohno){
                cout<< " OUT" << endl;
            }
            
            cost += 2700;
            ptr->outs++;
            
        }
        
    }
    pte->modified = 0;
    pte -> referenced = 0;
    pte->physical_frame = 0;
    pte ->present = 0;
    pte->mapped = 0;
}
void unmap_exit(PTE *pte, Process *ptr, int vpage){
    cost += 400;
    if (ohno){
        printf(" UNMAP %d:%d\n", ptr->pid, vpage);
    }
    
    if (pte->modified){
        // pte->paged_out = 1;
        if (pte -> file_mapped){
            if (ohno){
                cout << " FOUT" << endl;
            }
            
            cost += 2400;
            ptr->fouts++;

        }
        
    }
    pte ->paged_out = 0;
    // pte->modified = 0;
    pte -> referenced = 0;
    pte->physical_frame = 0;
    pte ->present = 0;
    pte->mapped = 0;
    
}
class Pager {
    public:
    virtual Frame* select_victim_frame() = 0; 
    virtual void reset_age(Frame *frame){}

};
class FIFO: public Pager{
    Frame  *select_victim_frame(){
        
        if (hand >= frame_size - 1){
            hand = -1;
        }
        
        hand++;
        frame_table[hand].p->unmaps++;
        PTE *pte = &frame_table[hand].p->page_table[frame_table[hand].virtual_address];
        unmap(pte, frame_table[hand].p, frame_table[hand].virtual_address);
        
        // frame_table[hand].p->page_table[frame_table[hand].virtual_address].present = 0;
        frame_table[hand].p = nullptr;
        frame_table[hand].virtual_address = 0;
        return &frame_table[hand];

    }
};
class Clock:public Pager{
    Frame *select_victim_frame(){
        if (hand >= frame_size - 1){
            // cout << "this" << endl;
            hand = -1;
        }
        
        hand++;
        
        while(frame_table[hand].p->page_table[frame_table[hand].virtual_address].referenced){
            // cout << "hand: " << hand << "referenced: " << frame_table[hand].p->page_table[frame_table[hand].virtual_address].referenced << endl;
            
            frame_table[hand].p->page_table[frame_table[hand].virtual_address].referenced = 0;
            hand++;
            if (hand > frame_size - 1){
                hand = 0;
            }
        }
        frame_table[hand].p->unmaps++;
        PTE *pte = &frame_table[hand].p->page_table[frame_table[hand].virtual_address];
        unmap(pte, frame_table[hand].p, frame_table[hand].virtual_address);
        frame_table[hand].p = nullptr;
        frame_table[hand].virtual_address = 0;
        return &frame_table[hand];

    }
};

class NRU: public Pager{
    Frame *select_victim_frame(){
        bool class0 = false;
        bool class1 = false;
        bool class2 = false;
        bool class3 = false;
        Frame *frame_class0 = nullptr;
        Frame *frame_class1 = nullptr;
        Frame *frame_class2 = nullptr;
        Frame *frame_class3 = nullptr;
        Frame *victim;
        if (hand >= frame_size - 1){
            hand = -1;
        }
        hand++;
        bool reset = false;
        if (count_inst >= 50){
            count_inst = 0;
            reset = true;
        }
        // if (count_inst >= 50){
        //     count_inst = 0;
        //     daemon();
        // }
        // cout << "hand " << hand << endl;
        // cout << "count_inst " << count_inst << endl;
        int flag = hand;
        while(true){
            // if (reset){
            //     frame_table[hand].p->page_table[frame_table[hand].virtual_address].referenced = 0;
            // }
            int ref_bit = frame_table[hand].p->page_table[frame_table[hand].virtual_address].referenced;
            int mod_bit = frame_table[hand].p->page_table[frame_table[hand].virtual_address].modified;
            if (2 * ref_bit + mod_bit ==  0 && !reset){
                frame_class0 = &frame_table[hand];
                break;
            }
            if (2 * ref_bit + mod_bit ==  0 && reset){
                if (!class0){
                    frame_class0 = &frame_table[hand];
                    class0 = true;
                }
            }
            if (2 * ref_bit + mod_bit ==  1){
                if (!class1){
                    frame_class1 = &frame_table[hand];
                    class1 = true;
                }
                
            }
            if (2 * ref_bit + mod_bit ==  2){
                if (!class2){
                    frame_class2 = &frame_table[hand];
                    class2 = true;
                }
                
            }
            if (2 * ref_bit + mod_bit ==  3){
                if (!class3){
                    frame_class3 = &frame_table[hand];
                    class3 = true;
                }
                
            }
            if (reset){
                frame_table[hand].p->page_table[frame_table[hand].virtual_address].referenced = 0;
            }
            hand++;
            if (hand >= frame_size){
                hand = 0;
            }
            if (hand == flag){
                break;
            }
        }
        if (frame_class0 != nullptr){
            victim = frame_class0;
        }
        else if (frame_class1 != nullptr){
            victim = frame_class1;
        }
        else if (frame_class2 != nullptr){
            victim = frame_class2;
        }
        else if(frame_class3 != nullptr){
            victim = frame_class3;
        }
        else{
            cout << "I'm screwed" << endl;
        }
        // cout << "ref " << victim->p->page_table[victim->virtual_address].referenced << endl;
        // cout << "mod " << victim->p->page_table[victim->virtual_address].modified<< endl;
        victim->p->unmaps++;
        PTE *pte = &victim->p->page_table[victim->virtual_address];
        unmap(pte, victim->p, victim->virtual_address);
        victim->p = nullptr;
        victim->virtual_address = 0;
        hand = victim->frame_index;
        return victim;


    }
    void daemon(){
        for (int i = 0; i < frame_size; i++){
            frame_table[i].p->page_table[frame_table[i].virtual_address].referenced = 0;
        }
    }
};
class Aging: public Pager{
    public:
    Frame *select_victim_frame(){
       set_age();
       Frame *frame;
       unsigned int lowest_bit = numeric_limits<unsigned int>::max();
    //    cout << "lowest_bit " << lowest_bit << endl;
       if (hand >= frame_size - 1){
            hand = -1;
        }
       hand++;
       int flag =  hand;
       int index= 0;

       while(true){
            // cout << "hand " << hand << endl;
            // cout << "age " << frame_table[hand].age << endl;
            if (frame_table[hand].age < lowest_bit){
                // cout << "here " << endl;
               lowest_bit = frame_table[hand].age;
               frame = &frame_table[hand];
               index = hand;
            }
            hand++;
            if (hand > frame_size - 1){
                // cout << "here" << endl;
                hand = 0;
            }
            if ( hand == flag ){
                // cout << "this " << endl;
                break;
            }
           
        }
        hand = index;
        frame->p->unmaps++;
        PTE *pte = &frame->p->page_table[frame->virtual_address];
        unmap(pte, frame->p, frame->virtual_address);
        frame->p = nullptr;
        frame->virtual_address = 0;
        frame->age = 0;
        return frame;
        

    }
    void set_age(){
        for (int i = 0; i < frame_size; i++){
            frame_table[i].age = frame_table[i].age >> 1;
            if (frame_table[i].p->page_table[frame_table[i].virtual_address].referenced){
                
                frame_table[i].age = (frame_table[i].age | 0x80000000 );
                frame_table[i].p->page_table[frame_table[i].virtual_address].referenced = 0;
                // cout << "frame_table " << i << " " <<  "age " << frame_table[i].age << endl;
            }
        }
    }
    void reset_age(Frame *frame){
        frame->age = 0;
    }
};
class Working_set: public Pager{
    Frame *select_victim_frame(){
        if (hand >= frame_size - 1){
            hand = -1;
        }
       hand++;
       int flag =  hand;
       int index= 0;
       Frame *frame = &frame_table[hand];
       unsigned int oldest = numeric_limits<unsigned int>::max();
       for (int i = 0; i < frame_size; i++){
           if (frame_table[i].p->page_table[frame_table[i].virtual_address].referenced){
               frame_table[i].p->page_table[frame_table[i].virtual_address].referenced = 0;
               frame_table[i].current_time = count_inst;

           }
       }
    //    cout << "hand " << hand<< endl;
       while(true){
        //    cout << "hand " << hand<< endl;
        //    if (frame_table[hand].p->page_table[frame_table[hand].virtual_address].referenced){
        //        frame_table[hand].p->page_table[frame_table[hand].virtual_address].referenced = 0;
        //        frame_table[hand].current_time = count_inst;

        //    }
           if (count_inst - frame_table[hand].current_time > 50){
            //    cout << "instruction more than 50" << endl;
               frame = &frame_table[hand];
               break;
           }
           else {
               if (frame_table[hand].current_time < oldest){
                   frame = &frame_table[hand];
                   oldest = frame_table[hand].current_time;
                //    cout << "I'm the oldest" << endl;
               }
           }
           hand++;
            if (hand >= frame_size){
                hand = 0;
            }
            if (hand == flag){
                break;
            }
       }
       hand = frame->frame_index;
       frame->p->unmaps++;
       PTE *pte = &frame->p->page_table[frame->virtual_address];
       unmap(pte, frame->p, frame->virtual_address);
       frame->p = nullptr;
       frame->virtual_address = 0;
       frame->current_time = count_inst;
       return frame;

    }
};
int myrandom() {
    if (ofs >= stoi(randvals[0]) ){
        ofs = 0;
    }
    ofs++;
    return stoi(randvals[ofs]) % frame_size; 
    
}
class Random: public Pager{
    Frame *select_victim_frame(){
        int index = myrandom();
        Frame *frame = &frame_table[index];
        frame->p->unmaps++;
        PTE *pte = &frame->p->page_table[frame->virtual_address];
        unmap(pte, frame->p, frame->virtual_address);
        frame->p = nullptr;
        frame->virtual_address = 0;
        // frame->current_time = count_inst;
        return frame;
    }
};
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
    cout << "FT:" ;
    for (int i = 0; i < frame_size; i++){
        if (frame_table[i].p == nullptr){
            cout << " *";
        }
        else{
            printf(" %d:%d", frame_table[i].p->pid, frame_table[i].virtual_address);
        }
        
    }
    cout << endl;
}
void print_page_t(vector<Process*> process_ptr){
    for (int i = 0; i < process_ptr.size();  i++){
        printf("PT[%d]:", i);
        for (int j =  0; j < sizeof(process_ptr[i]->page_table)/sizeof(*process_ptr[i]->page_table); j++){
            char ref;
            char swap;
            char modify;
            if (! process_ptr[i] -> page_table[j].present){
                if (process_ptr[i] -> page_table[j].paged_out){
                    cout << " #";
                    continue;
                }
                else{
                    cout << " *";
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

            printf(" %d:%c%c%c", j, ref, modify, swap );
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
    Pager *the_Pager;
    hand = -1;
    count_inst = 0; 
    vector <char> instruction_char;
    vector <int> instruction_int;
    // ifstream file(argv[4]);
    string str;
    // vector < vector < int > > total_vma;
    vector <VMA*> total_vma;
    vector <int> vma;
    vector <Process*> process_ptr;
    int process_number= 0;
    int flag = 0;
    int num_p = -1;
    int num_vma = -1;
    ofs = 0;
    // getting option args:
    int c;
    string o;
    // int frame_size;
    bool print_PageTable = false;
    bool print_FrameTable = false;
    bool stats = false;
    ohno = false;
    char *get_pager = NULL;
    char* verbose = NULL;
    // string options = NULL;
    while ((c = getopt (argc, argv, "f:a:o:")) != -1)
    
    switch (c)
      {
      case 'f':
        frame_size = stoi(optarg);
        break;
      case 'a':
        get_pager = optarg;
        if (get_pager[0] == 'c'){
        the_Pager = new Clock;
        }
        else if (get_pager[0] == 'f'){
            the_Pager = new FIFO;
        }
        else if (get_pager[0] == 'a'){
            the_Pager = new Aging;
        }
        else if (get_pager[0] == 'w'){
            the_Pager = new Working_set;
        }
        else if (get_pager[0] == 'e'){
            the_Pager = new NRU;
        }
        else if (get_pager[0] == 'r'){
            the_Pager = new Random;
        }
        else{
            cout<< "the algorithm didn't get recognized" << endl;
        }
        
        break;
      case 'o':
        verbose = optarg;
        
        break;
      default:
        abort ();
    }
    string options = string(verbose);
    // cout << options << endl;
    if  (options.find('S') != string::npos){
        stats = true;
    }
    if (options.find('F') != string::npos){
        print_FrameTable = true;
    }
    if (options.find('P') != string::npos){
        print_PageTable = true;
    }
    if (options.find('O') != string::npos){
        ohno = true;
    }
    string filename_input;
    string filename_rand;
    // int index = 0;
    for (int i = optind; i < argc; i++){
        string arg(argv[i]);
        if (arg.find("rfile") != string::npos){
            filename_rand = arg;
            
            // ifstream file(argv[i]);
        }
        else {
            filename_input = arg;
            // index = i;
            // cout << "filename_input " << filename_input << endl;
        }
    }
    // string filename_rand = argv[5]; 
    
    ifstream r;
    r.open(filename_rand);
    copy(istream_iterator<string>(r),
    istream_iterator<string>(),
    back_inserter(randvals));
    r.close();
    
    ifstream f;
    // in.open(filename_input);
    f.open(filename_input);
    while (getline(f, str))
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
            process_number++;
            total_vma.clear();
            
        } 

    }
    f.close();
    for (int i = 0; i < frame_size; i++){
        frame_table[i].frame_index = i;
        free_list.push(&frame_table[i]);
    }
    
    // string frame_num(frame_size);
    // int frame_numbers = stoi(frame_num);
    // cout << frame_size << endl;
    // cout << "instruction_char_size " << instruction_char.size() << endl;

   
    
    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ simulation
    Process *current_process;
    // cost += instruction_char.size() - 1;
    for (int i = 0; i < instruction_char.size(); i++){
        count_inst++;
        if (ohno){
            printf("%d: ==> %c %d\n", i, instruction_char[i], instruction_int[i]);
        }
        
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
        else if (instruction_char[i] == 'e'){
            cost += 1250;
            // count_inst = 0;
            process_exits++;
            if (ohno){
                printf("EXIT current process %d\n", current_process->pid);
            }
            
            for (int j = 0; j < sizeof(current_process->page_table) / sizeof(*current_process->page_table); j++){
                if (current_process->page_table[j].mapped){
                    free_list.push(&frame_table[current_process->page_table[j].physical_frame]); 
                    frame_table[current_process->page_table[j].physical_frame].p = nullptr;
                    frame_table[current_process->page_table[j].physical_frame].virtual_address = 0;
                    // frame_table[current_process->page_table[j].physical_frame].current_time = count_inst;
                    unmap_exit(&current_process->page_table[j], current_process, j);
                    current_process->unmaps++;
                    
                    
                }
                else{

                    current_process->page_table[j].paged_out = 0;
                    current_process->page_table[j].modified = 0;
                    current_process->page_table[j].referenced = 0;
                }
            }
            continue;
        }
        PTE *pte = &current_process->page_table[instruction_int[i]];
        if (!pte->setBits_flag){
            set_bits(current_process, instruction_int[i]);
            pte->setBits_flag = 1;
        }
        if (!pte->accessed){
            cost += 340;
            if (ohno){
                cout<< " SEGV" <<endl;
            }
            
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
                if (ohno){
                    cout << " ZERO"<< endl;
                }
                
                cost += 140;
                current_process->zeros++;
                
            }
            else if (pte ->paged_out && !pte ->file_mapped){
                if (ohno){
                    cout << " IN" << endl;
                }
                
                cost += 3100;

                current_process->ins++;
            }
            else if (pte ->file_mapped){
                if (ohno){
                    cout << " FIN" << endl;
                }
                
                cost += 2800;
                current_process->fins++;
            }
            // page_fault(current_process, pte, the_Pager, instruction_int[i]);
            if (ohno){
                cout << " MAP " << frame->frame_index << endl;
            }
            
            cost += 300;
            current_process->maps++;
            pte ->mapped = 1;
            the_Pager->reset_age(frame);
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
            if (ohno){
                cout << " SEGPROT" << endl;
            }
            
            cost ++;
            cost += 420;
            
        }

        
    }
    if (print_PageTable){
        print_page_t(process_ptr);
    }
    if (print_FrameTable){
        print_frame_t();
    }
    if(stats){
         print_stats(process_ptr);
         printf("TOTALCOST %lu %lu %lu %llu %lu\n",
                instruction_char.size(), ctx_switches, process_exits, cost, sizeof(process_ptr[0]->page_table) /MAX_VPAGES);
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