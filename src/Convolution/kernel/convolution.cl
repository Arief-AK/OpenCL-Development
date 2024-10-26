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
    
    // Iterate through the input signal and mask matrices
    for(int r = 0; r < mask_width; r++){
        const int index = (y + r) * input_width + x;

        for(int c = 0; c < mask_width; c++){
            sum += mask[(r*mask_width) + c] * input[index + c];
        }
    }

    // Set to the output array
    output[y*get_global_size(0) + x] = sum;
}