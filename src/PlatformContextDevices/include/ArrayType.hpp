#include <iostream>

template <typename T>
class ArrayType
{
public:
    static bool isChar() {return false;}
};

template<>
class ArrayType<char>
{
public:
    static bool isChar() {return true;}
};