#include<stdlib.h>
#include<stdio.h>
#include"seamcarving.h"

int main (void){

    // struct rgb_img *img;
    // struct rgb_img *grad;
    // double *best_arr;
    // struct rgb_img *cur_im;
    // int *path;
    // read_in_img(&img, "6x5.bin");
    // calc_energy(img, &grad);
    // print_grad(grad);
    // dynamic_seam(grad, &best_arr);
    // recover_path(best_arr, grad->height, grad->width, &path);
    // remove_seam(img, &cur_im, path);



    struct rgb_img *im;
    struct rgb_img *cur_im;
    struct rgb_img *grad;
    double *best;
    int *path;

    read_in_img(&im, "HJoceanSmall.bin");
    
    for(int i = 0; i < 150; i++){
        printf("i = %d\n", i);
        calc_energy(im,  &grad);
        dynamic_seam(grad, &best);
        recover_path(best, grad->height, grad->width, &path);
        remove_seam(im, &cur_im, path);

        // char filename[200];
        // sprintf(filename, "img%d.bin", i);
        // write_img(cur_im, filename);

        destroy_image(im);
        destroy_image(grad);
        free(best);
        free(path);
        im = cur_im;
    }
    write_img (im, "img1.bin");
    destroy_image(im);
    return 0;
}