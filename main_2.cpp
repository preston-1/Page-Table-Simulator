#include "main.hpp"
#include "vector"
#include "math.h"
#include "iostream"

using namespace std;
int virtual_address, physical_address, size, offset;

int translate_v2p(std::string bin_address){
    std::string address_bits = bin_address.substr(0, offset);
    return bin2dec(address_bits);
}

bool isDecimal(std::string input)
{
    for (char const &c : input) {
        if (std::isdigit(c) == 0) return false;
    }
    return true;
}

bool isHex(std::string input)
{
    if (input.compare(0,2,"0x")) return false;
    for (char const &c : input.substr(2)) {
        if (std::isxdigit(c) == 0 ) return false;
    }
    return true;
}

std::string dec2bin(int decimal){
    std::string binary;
    while(decimal != 0) {
        binary= (decimal % 2 == 0 ? "0" : "1") + binary;
        decimal/=2;
    }
    return binary;
}

int bin2dec(std::string binary){
    return std::stoi(binary, 0, 2);
}

std::string hex2bin(std::string hex){
    hex = hex.substr(2);
    std::string binary;
    for(unsigned i = 0; i != hex.length(); ++i)
        binary += hex2bin_helper(hex[i]);

    return binary;
}

const char* hex2bin_helper(char c){
    switch(toupper(c))
    {
        case '0': return "0000";
        case '1': return "0001";
        case '2': return "0010";
        case '3': return "0011";
        case '4': return "0100";
        case '5': return "0101";
        case '6': return "0110";
        case '7': return "0111";
        case '8': return "1000";
        case '9': return "1001";
        case 'A': return "1010";
        case 'B': return "1011";
        case 'C': return "1100";
        case 'D': return "1101";
        case 'E': return "1110";
        default: return "1111";
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) { // if there aren't arguments passed
        std::cout << "Please include a filename as an argument." << std::endl;
        return 1;
    }
    // Handle Page Table File
    std::ifstream pagetable;
    pagetable.open(argv[1]); // argv[1] should be the filename
    if (!pagetable.is_open()) { // if the file doesn't open, print error message and close program
        std::cout << "Failed to open file \"" << argv[1] << "\"" << std::endl;
        return 1;
    }
    pagetable >> virtual_address; // read the virtual address in from the file (bits)
    pagetable >> physical_address; // read the physical address in from the file (bits)
    pagetable >> size; // read the size in from the file (bytes)
    offset = virtual_address - log2(size); // calculate how many bits are used for offset

    // create the page table
    const int rows = pow(2, offset);
    std::vector<std::vector<int>> table_vec;
    for(int i= 0; i < rows; i++){
        std::vector<int> table_row;
        for(int j = 0; j < 4; j++){
            int val;
            pagetable >> val;
            table_row.push_back(val);
        }
        table_row.push_back(i); //add the index to the row to be used later on in eviction
        table_vec.push_back(table_row);
    }
    // Take user input to use as virtual address
    //now we have our frames
    int pointer = 0; //pointer starts at 0 for our frames
    std::string input;
    std::string address;

    vector<int> table_frames_index;
    for(int i = 0; i < table_vec.size() ; i++)
    {
        if(table_vec[i][0] == 1){//is a valid page
            //insert its index into your frames
            table_frames_index.push_back(table_vec[i][4]);
        }
    }
    while (std::getline(std::cin, input)) { // take user input
        // Output the text from the file
        if (isDecimal(input)) { // checks if the user input is decimal
            address = dec2bin(std::stoi(input));
        }
        else if (isHex(input)){ // checks if the user input is hex
            address = hex2bin(input);
        }
        else { // user input is neither decimal nor hex
            break;
        }
        if(address.length() < virtual_address){ //prepend with 0's
            std::string zeros = std::string(virtual_address - address.length(),'0');
            address = zeros + address; //updated binary to be the lenght of the Virtual Adress
        }
        else if(address.length() > virtual_address){ //truncate binary
            address = address.substr(address.size() - virtual_address);
        }
        std::string offset_string = address.substr(offset);
        std::string lookup = address.substr(0, offset);
        int table_index = bin2dec(lookup); //convert binary string to int and subtract one for 0th index in table
        if(table_index == -1){
            table_index = 0;
        }

        if (table_vec[table_index][0] == 1) { // valid page, print address
            std::cout << table_vec[table_index][2] * size + bin2dec(offset_string) << std::endl;
            table_vec[table_index][3]=1;
        }
        else if(table_vec[table_index][0] == 0 && table_vec[table_index][1] == 0){ // invalid page and no access
            std::cout << "SEGFAULT" << std::endl;
            table_vec[table_index][3]=1;
        }
        else{
                
            bool complete = false;
            while(!complete){
                if(table_frames_index.size() == 0){
                    //edge case: no valid pages so no eviction can occur
                    std::cout << "SEGFAULT" << std::endl;
                    break;
                }
                else if(table_vec[table_frames_index[pointer]][3] == 1) {//check the isUsed bit
                    table_vec[table_frames_index[pointer]][3] = 0;
                    if(pointer == table_frames_index.size()-1){ // if pointer is at the end set it to start
                        pointer = 0;
                    }
                    else{
                        pointer++;
                    }
                }
                else{ //isUsed is 0 so we will do a eviction
                    //1. get index of one kicking out
                    //2. update Page table and make evicition invalid and make promoted page valid, update frame, set bit
                    table_vec[table_frames_index[pointer]][0] = 0;

                    table_vec[table_index][0] = 1;

                    table_vec[table_index][2] = table_vec[table_frames_index[pointer]][2];

                    table_vec[table_index][3] = 1;

                    //update queue
                    table_frames_index[pointer]  = table_vec[table_index][4];

                    if(pointer == table_frames_index.size()-1){ // if pointer is at the end set it to start
                        pointer = 0;
                    }
                    else{
                        pointer++;
                    }
                    std::cout << "PAGEFAULT " << table_vec[table_index][2] * size + bin2dec(offset_string) << std::endl;
                    complete = true;
                }
                
            }            
        }
    }

    return 0;
}