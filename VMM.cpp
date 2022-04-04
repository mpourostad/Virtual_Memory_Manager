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
    vector < vector < int > > processes;
    vector <int> process;
    int process_number= 0;
    int flag = 0;
    int num_p = -1;
    int num_vma = 0;
    while (getline(file, str))
    {
        
        int n = str.length();
        char stoc [n + 1];
        strcpy(stoc, str.c_str());
        
        int success;
        int a, b, c, d;
        
        // if (success == 4){
        success = sscanf(stoc, "%d %d %d %d", &a, &b, &c, &d);
        if (num_vma != 0){
            if (success == 4){
                process.push_back(a);
                process.push_back(b);
                process.push_back(c);
                process.push_back(d);
                processes.push_back(process);
                process.clear();
                num_vma --;
            }           
           
        }        
        else if (success == 1){
            if (flag == 0){
                sscanf(stoc, "%d", &num_p);
                flag++;
            }
            else{
                sscanf(stoc, "%d", &num_vma);
                // cout << "vma: " << num_vma << endl; 
            }
        }
        if (num_vma == 0 &&  flag > 0){
            // to do: build a new Process class
            Process *pcb;
            pcb =  new Process(process_number);
        } 

    }
    cout << num_p << endl;
    // for (int i = 0; i < processes.size(); i++){
    //     for (int j = 0; j < processes[i].size(); j++){
    //         cout<< processes[i][j] << endl;
    //     }
    // }
    // for (int i = 0; i < processes.size(); i++){
    //     Process p = new Process();
    // }

    // }
    return 0;
}