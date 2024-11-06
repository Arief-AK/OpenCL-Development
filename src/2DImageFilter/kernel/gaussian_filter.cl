__kernel void gaussian_filter(__read_only image2d_t src_image,
                              __write_only image2d_t dst_image,
                              sampler_t sampler,
                              int width, int height)
{
    /* Gaussian Kernel
        1 2 1
        2 4 2
        1 2 1
    */
    float kernel_weights[9] = {1.0f, 2.0f, 1.0f,
                               2.0f, 4.0f, 2.0f,
                               1.0f, 2.0f, 1.0f};

    // Set work-items
    int2 start_image_coord = (int2)(get_global_id(0) - 1, get_global_id(1) - 1);
    int2 end_image_coord = (int2)(get_global_id(0) + 1, get_global_id(1) + 1);
    int2 out_image_coord = (int2)(get_global_id(0), get_global_id(1));

    if(out_image_coord.x < width && out_image_coord.y < height){
        // Initialise variables
        int weight = 0;
        float4 out_colour = (float4)(0.0f, 0.0f, 0.0f, 0.0f);

        // Go through the coordinates
        for(int y = start_image_coord.y; y <= end_image_coord.y; y++){
            for(int x = start_image_coord.x; x <= end_image_coord.x; x++){
                out_colour += (read_imagef(src_image, sampler, (int2)(x,y)) * (kernel_weights[weight] / 16.0f));
                weight += 1;
            }
        }

        // Write output value to the image
        write_imagef(dst_image, out_image_coord, out_colour);
    }
}