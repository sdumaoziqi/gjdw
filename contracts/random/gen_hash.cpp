#include <iostream>
#include <string>

std::string HASH_STRING_PIECE(std::string string_piece) {
    int64_t result = 0;
    for (auto it = string_piece.cbegin(); it != string_piece.cend(); ++it) {  
        result = (result * 131) + *it;
    }                                                                         
    return std::to_string(result);
}


int main(int argc, char *argv[])
{
	std::string  n = HASH_STRING_PIECE(argv[1]);
	std::cout << n << std::endl;

	return 0;
}
