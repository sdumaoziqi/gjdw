#include <iostream>
#include <string>

uint64_t HASH_STRING_PIECE(std::string string_piece) {                 
    uint64_t result = 0;                                                        
    for (auto it = string_piece.cbegin(); it != string_piece.cend(); ++it) {  
        result = (result * 131) + *it;                                       
    }                                                                         
    return result;
}

int main(int argc, char *argv[])
{
	uint64_t  n = HASH_STRING_PIECE(argv[1]);
	std::cout << n << std::endl;

	return 0;
}
