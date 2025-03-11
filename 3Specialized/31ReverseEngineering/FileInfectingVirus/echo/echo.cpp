#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << 0;
    }
    else
    {
        std::cout << argv[1];
    }
    return 0;
}