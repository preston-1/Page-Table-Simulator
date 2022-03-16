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
        table_vec.push_back(table_row);
    }
    int** table = new int*[rows];
    for (int i = 0; i<rows; i++){
        table[i] = new int[4];
    }

    // populate the table structure from the given file
    for (int i = 0; i<rows; i++){
        for (int j = 0; j<4; j++) {
            pagetable >> table[i][j];
        }
    }

    // Take user input to use as virtual address
    std::string input;
    std::string address;
    std::string test;

    while (std::getline(std::cin, input)) { // take user input
        // Output the text from the file
        if (isDecimal(input)) { // checks if the user input is decimal
            address = dec2bin(std::stoi(input));
        }
        else if (isHex(input)){ // checks if the user input is hex
            address = hex2bin(input);
        }
        else { // user input is neither decimal nor hex
            // TODO: end if user enters EOF (control+D). continue otherwise?
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
        }
        else if(table_vec[table_index][0] == 0 && table_vec[table_index][1] == 1){ // invalid page but access permitted
            // TODO: if program is run with PART B functionality, need to update the page table using replacement algo
            std::cout << "DISK" << std::endl;
        }
        else if(table_vec[table_index][0] == 0 && table_vec[table_index][1] == 0){ // invalid page and no access
            std::cout << "SEGFAULT" << std::endl;
        }
    }

    return 0;
}