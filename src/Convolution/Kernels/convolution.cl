__kernel void convolve(
    const __global uint* const input,
    __constant uint* const mask,
    __global uint* const output,
    const int input_width,
    const int mask_width)
{
    // Initialise variables
    const int x = get_global_id(0);
    const int y = get_global_id(1);

    uint sum = 0;
    for(int r = 0; r < mask_width; r++){
        // TBA
    }
}