#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include"seamcarving.h"


#define MIN(x, y) (((x) < (y)) ? (x) : (y)) 

#define MIN3(x,y,z) (MIN(MIN(x, y), z))

void get_rgbxy(struct rgb_img *im, int *r_xy, int *g_xy, int *b_xy, int i1, int j1, int i2, int j2){
    *r_xy = (get_pixel(im, i1, j1, 0) - get_pixel(im, i2, j2, 0));
    *g_xy = (get_pixel(im, i1, j1, 1) - get_pixel(im, i2, j2, 1));
    *b_xy = (get_pixel(im, i1, j1, 2) - get_pixel(im, i2, j2, 2));
}

void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    int height = im->height; 
    int width = im->width;
    int d_x, d_y; 
    int r_xy, g_xy, b_xy; 

    *grad = (struct rgb_img *)malloc(sizeof(struct rgb_img));
    (*grad)->height = height; 
    (*grad)->width = width; 
    (*grad)->raster = (uint8_t *)malloc(3 * height * width);

    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            if (i == 0){
                get_rgbxy(im, &r_xy, &g_xy, &b_xy, height-1, j, i+1, j);
            }else if (i == height - 1){
                get_rgbxy(im, &r_xy, &g_xy, &b_xy, i-1, j, 0, j);
            }else {
                get_rgbxy(im, &r_xy, &g_xy, &b_xy, i-1, j, i+1, j);
            }

            d_y = pow(r_xy, 2) + pow(g_xy, 2) + pow(b_xy, 2);

            if (j == 0){
                get_rgbxy(im, &r_xy, &g_xy, &b_xy, i, j+1, i, width-1);
            }else if (j == width - 1){
                get_rgbxy(im, &r_xy, &g_xy, &b_xy, i, 0, i, j-1);
            }else{
                get_rgbxy(im, &r_xy, &g_xy, &b_xy, i, j+1, i, j-1);
            }

            d_x = pow(r_xy, 2) + pow(g_xy, 2) + pow(b_xy, 2);
            
            int energy = (sqrt(d_x + d_y));
            uint8_t color = energy/10;
            
            set_pixel(*grad, i, j, color, color, color);
        }
    }
}

void print_arr(double *best_arr, int height, int width){
    for (int i = 0; i < height; i ++){
        for (int j = 0; j < width; j ++ ){
            printf ("%f\t", best_arr[i*width + j]);
        }
        printf ("\n");
    }
}

void dynamic_seam(struct rgb_img *grad, double **best_arr){

    int height = grad->height;
    int width = grad->width;
    double a, b, c;
    *best_arr = (double *)malloc(height*width *sizeof(double));

    for (int i = 0; i < height; i ++){
        for (int j = 0; j < width; j ++){
            (*best_arr)[i*width + j] = (double)(get_pixel(grad, i, j, 0));
        }
    }
    // for (int i = 0; i < height; i ++){
    //     for (int j = 0; j < width; j ++ ){
    //         printf ("%f\t", (*best_arr)[i*width + j]);
    //     }
    //     printf ("\n");
    // }

    for (int i = 1; i < height; i++){
        a = (*best_arr)[(i-1)*width];
        b = (*best_arr)[(i-1)*width + 1];

        // printf ("First column: %d\n", i);
        // printf ("%f %f \n", a, b);
        // printf ("Current: %f\n", (double)(get_pixel(grad, i, 0, 0)));
        // printf ("Entry: %f\n", (double)(get_pixel(grad, i, 0, 0)) + MIN(a, b));
        (*best_arr)[i*width] = (double)(get_pixel(grad, i, 0, 0)) + MIN(a, b);

        for (int j = 1; j < width - 1; j ++){
            a = (*best_arr)[(i-1)*width + j - 1];
            b = (*best_arr)[(i-1)*width + j];
            c = (*best_arr)[(i-1)*width + j + 1];
            // printf ("Middle column: %d\n", i);
            // printf ("%f %f %f\n", a, b, c);
            // printf ("Current: %f\n", (double)(get_pixel(grad, i, j, 0)));
            // printf ("Entry: %f\n", (double)(get_pixel(grad, i, j, 0)) + MIN3(a, b, c));

            (*best_arr)[i*width + j] = (double)(get_pixel(grad, i, j, 0)) + MIN3(a, b, c);
        }

        a = (*best_arr)[(i-1)*width + width - 1];
        b = (*best_arr)[(i-1)*width + width - 2];
        // printf ("Last column: %d\n", i);
        // printf ("%f %f \n", a, b);
        // printf ("Current: %f\n", (double)(get_pixel(grad, i, width-1, 0)));
        // printf ("Entry: %f\n\n", (double)(get_pixel(grad, i, width-1, 0)) + MIN(a, b));

        (*best_arr)[i*width + width-1] = (double)(get_pixel(grad, i, width-1, 0)) + MIN(a, b);
        // print_arr(*best_arr, height, width);
        // printf("\n");
    }
    // printf("\n");
    // print_arr(*best_arr, height, width);
}

void recover_path(double *best, int height, int width, int **path){
    
    int col = 0;
    double cur_min = 100000;
    *path = (int *)malloc(height * sizeof(int));
    
    for (int i = 0; i < width; i ++){
        // printf("%f %f \n", cur_min, best[(height-1)*width + i]);
        if (cur_min > best[(height-1)*width + i]){
            cur_min = best[(height-1)*width + i];
            col = i;
        }
    }
    (*path)[height-1] = col; 
    // printf ("%f %d\n", cur_min, (*path)[height-1]);

    for (int i = height-2; i > -1; i--){
        cur_min = 1e9;
        col = (*path)[i+1];
        if (col == 0){
            for (int j = col; j < col + 2; j ++){
                if (best[i*width + j] < cur_min){
                    cur_min = best[i*width + j];
                    (*path)[i] = j;
                }
            }
        }else if (col == width-1){
            for (int j = col-1; j < col+1; j++){
                if (best[i*width + j] < cur_min){
                    cur_min = best[i*width + j];
                    (*path)[i] = j;
                }
            }
        }else{
            for (int j = col-1; j < col+2; j++){
                if (best[i*width + j] < cur_min){
                    cur_min = best[i*width + j];
                    (*path)[i] = j;
                }
            }
        }
        // (*path)[i] = col; 
    }

    // for (int i = 0; i < height; i++){
    //     printf("%d ", (*path)[i]);
    // }
    // printf("\n");
}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path){
    int height = src->height; 
    int width = src->width;
    int col;

    *dest = (struct rgb_img*)malloc(sizeof(struct rgb_img));
    (*dest)->height = height;
    (*dest)->width = width - 1; 
    (*dest)->raster = (uint8_t *)malloc(3 * height * (width-1));

    for(int i = 0; i < height; i++){
        col = 0;

        // if (0 != path[i]){
        //     set_pixel(*dest, i, col, get_pixel(src, i, 0, 0), get_pixel(src, i, 0, 1), get_pixel(src, i, 0, 2));
        //     col ++;
        // }

        for(int j = 0; j < width; j++){
            if (j != path[i]){
                set_pixel(*dest, i, col, get_pixel(src, i, j, 0), get_pixel(src, i, j, 1), get_pixel(src, i, j, 2));
                col ++;
            }
        }

        // if(width - 1 != path[i]){
        //     set_pixel(*dest, i, col, get_pixel(src, i, width-1, 0), get_pixel(src, i, width-1, 1), get_pixel(src, i, width-1, 2));
        // }
    }    
}