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

class FrameTable{

};
vector <FrameTable*> frame_table_array;

class PTE{
    unsigned int present:1;
    unsigned int referenced:1;
    unsigned int modified:1;
    unsigned int write_protect:1;
    unsigned int paged_out:1;
    unsigned int physical_frame:7;
};
class Process{
    public:
    int pid;
    vector <vector <int> > vma;
    vector <PTE*> page_table;
    Process(int);
};
Process::Process(int id){
    pid = id;

}

int main(int argc, char** argv){
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
        int a, b, c, d;
        
        // if (success == 4){
        success = sscanf(stoc, "%d %d %d %d", &a, &b, &c, &d);
        // if (num_vma != 0){
        if (success == 4){

            vma.push_back(a);
            vma.push_back(b);
            vma.push_back(c);
            vma.push_back(d);
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
            process_number++;
            total_vma.clear();
            
        } 

    }
    // cout << num_p << endl;
    // for (int i = 0; i < processes.size(); i++){
    //     for (int j = 0; j < processes[i].size(); j++){
    //         cout<< processes[i][j] << endl;
    //     }
    // }
    // cout << process_ptr.size() << endl;
    for (int i = 0; i < process_ptr.size(); i++){
        // cout << "thiiiiiiiiiis " << endl;
        cout<< "pid: " <<process_ptr[i] -> pid << endl;
        for (int j = 0; j < process_ptr[i] -> vma.size(); j++){
            for (int k = 0; k < process_ptr[i] -> vma[j].size(); k++){
                cout << process_ptr[i] -> vma[j][k] << " ";
            }
            cout<< endl;
        }
    }
    cout << endl;
    // for (int i = 0; i < instruction_char.size(); i++){
    //     cout << instruction_char[i] << " " << instruction_int[i] << endl;
    // }

    // }
    return 0;
}