__kernel void square(__global int* buffer){
    const size_t id = get_global_id(0);

    buffer[id] = buffer[id] *buffer[id];
}