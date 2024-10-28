#ifndef INFODEVICE_H
#define INFODEVICE_H

#include <CL/cl.h>
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

// Helper function for display function
template <typename T>
void appendBitField(T info, T value, std::string name, std::string& str)
{
    // Check if exists
    if(info & value){
        if(str.length() > 0){
            str.append(" | ");
        }
        str.append(name);
    }
}

template <typename T>
class InfoDevice
{
public:
    static void display(cl_device_id id, cl_device_info name, std::string str, std::string& received_device_type)
    {
        cl_int err_num = {};
        size_t param_value_size = {};

        // Determine the number of devices
        err_num = clGetDeviceInfo(id, name, 0, NULL, &param_value_size);
        if(err_num != CL_SUCCESS){
            std::cerr << "Failed to determine number of OpenCL devices" << std::endl;
            return;
        }

        // Allocate memory to store the device information
        T* info = (T*)alloca(sizeof(T) * param_value_size);
        err_num = clGetDeviceInfo(id, name, param_value_size, info, NULL);
        if(err_num != CL_SUCCESS){
            std::cerr << "Failed to retrieve OpenCL device information" << std::endl;
            return;
        }

        switch (name)
        {
        case CL_DEVICE_TYPE:
            {
                std::string device_type = {};
                appendBitField<cl_device_type>(*(reinterpret_cast<cl_device_type*>(info)), CL_DEVICE_TYPE_CPU, "CL_DEVICE_TYPE_CPU", device_type);                  // CPU type
                appendBitField<cl_device_type>(*(reinterpret_cast<cl_device_type*>(info)), CL_DEVICE_TYPE_GPU, "CL_DEVICE_TYPE_GPU", device_type);                  // GPU type
                appendBitField<cl_device_type>(*(reinterpret_cast<cl_device_type*>(info)), CL_DEVICE_TYPE_ACCELERATOR, "CL_DEVICE_TYPE_ACCELERATOR", device_type);  // Accelerator type
                appendBitField<cl_device_type>(*(reinterpret_cast<cl_device_type*>(info)), CL_DEVICE_TYPE_DEFAULT, "CL_DEVICE_TYPE_DEFAULT", device_type);          // Default type
                
                // Display the type
                std::cout << "\t\t" << str << ":\t" << device_type << std::endl;
                received_device_type = device_type;
            }
            break;

        case CL_DEVICE_SINGLE_FP_CONFIG:
            {
                std::string floating_point_type = {};
                appendBitField<cl_device_fp_config>(*(reinterpret_cast<cl_device_fp_config*>(info)), CL_FP_DENORM, "CL_FP_DENORM", floating_point_type);                        // Floating-point denormalised support
                appendBitField<cl_device_fp_config>(*(reinterpret_cast<cl_device_fp_config*>(info)), CL_FP_INF_NAN, "CL_FP_INF_NAN", floating_point_type);                      // Floating-point infinite and not a number supported
                appendBitField<cl_device_fp_config>(*(reinterpret_cast<cl_device_fp_config*>(info)), CL_FP_ROUND_TO_NEAREST, "CL_FP_ROUND_TO_NEAREST", floating_point_type);    // Floating-point round-to-nearest mode is supported    (MANDATORY)
                appendBitField<cl_device_fp_config>(*(reinterpret_cast<cl_device_fp_config*>(info)), CL_FP_ROUND_TO_ZERO, "CL_FP_ROUND_TO_ZERO", floating_point_type);          // Floating-point round-to-zero mode is supported
                appendBitField<cl_device_fp_config>(*(reinterpret_cast<cl_device_fp_config*>(info)), CL_FP_ROUND_TO_INF, "CL_FP_ROUND_TO_INF", floating_point_type);            // Floating-point round-to-inifinity mode is supported  (MANDATORY)
                appendBitField<cl_device_fp_config>(*(reinterpret_cast<cl_device_fp_config*>(info)), CL_FP_FMA, "CL_FP_FMA", floating_point_type);                              // Floating-point IEEE 754-2008 fused multiply-add is supported
                appendBitField<cl_device_fp_config>(*(reinterpret_cast<cl_device_fp_config*>(info)), CL_FP_SOFT_FLOAT, "CL_FP_SOFT_FLOAT", floating_point_type);                // Basic floating-point operations are implemented in software
            
                // Display the floating point type
                std::cout << "\t\t" << str << ":\t" << floating_point_type << std::endl;
            }
            break;

        case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:
            {
                std::string memory_type = {};
                appendBitField<cl_device_mem_cache_type>(*(reinterpret_cast<cl_device_mem_cache_type*>(info)), CL_NONE, "CL_NONE", memory_type);
                appendBitField<cl_device_mem_cache_type>(*(reinterpret_cast<cl_device_mem_cache_type*>(info)), CL_READ_ONLY_CACHE, "CL_READ_ONLY_CACHE", memory_type);
                appendBitField<cl_device_mem_cache_type>(*(reinterpret_cast<cl_device_mem_cache_type*>(info)), CL_READ_WRITE_CACHE, "CL_READ_WRITE_CACHE", memory_type);

                // Display the global memory type
                std::cout << "\t\t" << str << ":\t" << memory_type << std::endl;
            }
            break;

        case CL_DEVICE_LOCAL_MEM_TYPE:
            {
                std::string memory_type = {};
                appendBitField<cl_device_local_mem_type>(*(reinterpret_cast<cl_device_local_mem_type*>(info)), CL_GLOBAL, "CL_LOCAL", memory_type);
                appendBitField<cl_device_local_mem_type>(*(reinterpret_cast<cl_device_local_mem_type*>(info)), CL_GLOBAL, "CL_GLOBAL", memory_type);

                // Display the local memory type
                std::cout << "\t\t" << str << ":\t" << memory_type << std::endl;
            }
            break;

        case CL_DEVICE_EXECUTION_CAPABILITIES:
            {
                std::string device_execution = {};
                appendBitField<cl_device_exec_capabilities>(*(reinterpret_cast<cl_device_exec_capabilities*>(info)), CL_EXEC_KERNEL, "CL_EXEC_KERNEL", device_execution);                // OpenCL device can execute OpenCL kernels (MANDATORY)
                appendBitField<cl_device_exec_capabilities>(*(reinterpret_cast<cl_device_exec_capabilities*>(info)), CL_EXEC_NATIVE_KERNEL, "CL_EXEC_NATIVE_KERNEL", device_execution);  // OpenCL device can execute native kernels

                // Display the device execution capabilities
                std::cout << "\t\t" << str << ":\t" << device_execution << std::endl;
            }
            break;

        case CL_DEVICE_QUEUE_PROPERTIES:
            {
                std::string device_queue = {};
                appendBitField<cl_device_exec_capabilities>(*(reinterpret_cast<cl_device_exec_capabilities*>(info)), CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, "CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE", device_queue);    // OpenCL device is enabled with out-of-order command queue execution
                appendBitField<cl_device_exec_capabilities>(*(reinterpret_cast<cl_device_exec_capabilities*>(info)), CL_QUEUE_PROFILING_ENABLE, "CL_QUEUE_PROFILING_ENABLE", device_queue);                              // OpenCL device can be profiled (MANDATORY)

                // Display the device command queue capabilities
                std::cout << "\t\t" << str << ":\t" << device_queue << std::endl;
            }
            break;

        default:
            // Display the contents that is received from clGetDeviceInfo
            std::cout << "\t\t" << str << ":\t" << *info << std::endl;
            break;
        }
    }
};

template <typename T>
class InfoDevice<ArrayType<T>>
{
public:
    static void display(cl_device_id id, cl_device_info name, std::string str)
    {
        cl_int err_num = {};
        size_t param_value_size = {};

        // Determine the number of devices
        err_num = clGetDeviceInfo(id, name, 0, NULL, &param_value_size);
        if(err_num != CL_SUCCESS){
            std::cerr << "Failed to determine number of OpenCL devices" << std::endl;
            return;
        }

        // Allocate memory to store the device information
        T* info = (T*)alloca(sizeof(T) * param_value_size);
        err_num = clGetDeviceInfo(id, name, param_value_size, info, NULL);
        if(err_num != CL_SUCCESS){
            std::cerr << "Failed to retrieve OpenCL device information" << std::endl;
            return;
        }

        if(ArrayType<T>::isChar()){
            std::cout << "\t" << str << ":\t" << info << std::endl;
        }
        else if(name == CL_DEVICE_MAX_WORK_ITEM_SIZES){
            cl_uint max_work_item_dimensions = {};

            err_num = clGetDeviceInfo(id, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(cl_uint), &max_work_item_dimensions, NULL);
            if(err_num != CL_SUCCESS){
                std::cerr << "Failed to find OpenCL device information" << std::endl;
                return;
            }

            // Display the device information
            std::cout << "\t" << str << ":\t";
            for (cl_uint i = 0; i < max_work_item_dimensions; i++){
                std::cout << info[i] << " ";
            }
            std::cout << std::endl;
        }
    }
};

#endif // INFODEVICE_H