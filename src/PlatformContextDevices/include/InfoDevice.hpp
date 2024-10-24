#include <CL/cl.h>
#include <iostream>

template <typename T>
class InfoDevice
{
public:

    // Function to display device information
    template <typename U>
    static void display(cl_device_id id, cl_device_info name, std::string str);

    static void print();

private:

    // Helper function for display function
    template <typename K>
    void appendBitField(K info, K value, std::string name, std::string& str);
};
