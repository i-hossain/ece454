/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include <time.h>
#include <math.h>
#define MULX(a,b) (a * b)

/* 
 * ECE454 Students: 
 * Please fill in the following team struct 
 */
team_t team = {
    "Akatsuki: The Brotherhood",              /* Team name */

    "Rushab Ramesh Kumar",     /* First member full name */
    "rushab.kumar@mail.utoronto.ca",  /* First member email address */

    "Ismail Hossain",                   /* Second member full name (leave blank if none) */
    "ridoy.hosssain@mail.utoronto.ca"                    /* Second member email addr (leave blank if none) */
};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/

/* 
 * naive_rotate - The naive baseline version of rotate 
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) 
{
    //clock_t begin;

    /* Mark beginning time */
    //begin = clock();

    //printf("dIM %d \n", dim);
    int i, j;

    for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	    dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];

    //printf ("time spent: %lf \n", (double)(clock() - begin) * 1000 / CLOCKS_PER_SEC);
}

/*
 * ECE 454 Students: Write your rotate functions here:
 */ 

/* 
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */

 /* 
 * total unroll tiling attempt (3 loops)
 * 
 *      "ONE CANNOT KNOW TRUE PEACE, UNLESS THEY KNOW TRUE PAIN. THERE IS NO PEACE WITHOUT PAIN."
 *      ~Nagato
 */
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst) 
{
    int M = 16; // for destination
    int N = 32;

    int i, j, bj;
    int destMultInit, srcMult, srcMultInit;

    int multiply[] = {
        0, 0,
        MULX(2,dim),
        MULX(3,dim),
        MULX(4,dim),
        MULX(5,dim),
        MULX(6,dim),
        MULX(7,dim),
        MULX(8,dim),
        MULX(9,dim),
        MULX(10,dim),
        MULX(11,dim),
        MULX(12,dim),
        MULX(13,dim),
        MULX(14,dim),
        MULX(15,dim)
    };

    for (i = 0; i < dim; i += M)
    {
        srcMultInit = i * dim;

        for (j = 0; j < dim; j += N)
        {
            for (bj = j; bj < j + N - 7; bj += 8)
            {
                destMultInit = (dim-1-bj) * dim + i;
                srcMult = srcMultInit + bj;

                dst[destMultInit] = src[srcMult];
                dst[destMultInit + 1] = src[srcMult + dim];
                dst[destMultInit + 2] = src[srcMult + multiply[2]];
                dst[destMultInit + 3] = src[srcMult + multiply[3]];
                dst[destMultInit + 4] = src[srcMult + multiply[4]];
                dst[destMultInit + 5] = src[srcMult + multiply[5]];
                dst[destMultInit + 6] = src[srcMult + multiply[6]];
                dst[destMultInit + 7] = src[srcMult + multiply[7]];
                dst[destMultInit + 8] = src[srcMult + multiply[8]];
                dst[destMultInit + 9] = src[srcMult + multiply[9]];
                dst[destMultInit + 10] = src[srcMult + multiply[10]];
                dst[destMultInit + 11] = src[srcMult + multiply[11]];
                dst[destMultInit + 12] = src[srcMult + multiply[12]];
                dst[destMultInit + 13] = src[srcMult + multiply[13]];
                dst[destMultInit + 14] = src[srcMult + multiply[14]];
                dst[destMultInit + 15] = src[srcMult + multiply[15]];

                destMultInit -= dim;
                srcMult++;

                dst[destMultInit] = src[srcMult];
                dst[destMultInit + 1] = src[srcMult + dim];
                dst[destMultInit + 2] = src[srcMult + multiply[2]];
                dst[destMultInit + 3] = src[srcMult + multiply[3]];
                dst[destMultInit + 4] = src[srcMult + multiply[4]];
                dst[destMultInit + 5] = src[srcMult + multiply[5]];
                dst[destMultInit + 6] = src[srcMult + multiply[6]];
                dst[destMultInit + 7] = src[srcMult + multiply[7]];
                dst[destMultInit + 8] = src[srcMult + multiply[8]];
                dst[destMultInit + 9] = src[srcMult + multiply[9]];
                dst[destMultInit + 10] = src[srcMult + multiply[10]];
                dst[destMultInit + 11] = src[srcMult + multiply[11]];
                dst[destMultInit + 12] = src[srcMult + multiply[12]];
                dst[destMultInit + 13] = src[srcMult + multiply[13]];
                dst[destMultInit + 14] = src[srcMult + multiply[14]];
                dst[destMultInit + 15] = src[srcMult + multiply[15]];

                destMultInit -= dim;
                srcMult++;

                dst[destMultInit] = src[srcMult];
                dst[destMultInit + 1] = src[srcMult + dim];
                dst[destMultInit + 2] = src[srcMult + multiply[2]];
                dst[destMultInit + 3] = src[srcMult + multiply[3]];
                dst[destMultInit + 4] = src[srcMult + multiply[4]];
                dst[destMultInit + 5] = src[srcMult + multiply[5]];
                dst[destMultInit + 6] = src[srcMult + multiply[6]];
                dst[destMultInit + 7] = src[srcMult + multiply[7]];
                dst[destMultInit + 8] = src[srcMult + multiply[8]];
                dst[destMultInit + 9] = src[srcMult + multiply[9]];
                dst[destMultInit + 10] = src[srcMult + multiply[10]];
                dst[destMultInit + 11] = src[srcMult + multiply[11]];
                dst[destMultInit + 12] = src[srcMult + multiply[12]];
                dst[destMultInit + 13] = src[srcMult + multiply[13]];
                dst[destMultInit + 14] = src[srcMult + multiply[14]];
                dst[destMultInit + 15] = src[srcMult + multiply[15]];

                destMultInit -= dim;
                srcMult++;

                dst[destMultInit] = src[srcMult];
                dst[destMultInit + 1] = src[srcMult + dim];
                dst[destMultInit + 2] = src[srcMult + multiply[2]];
                dst[destMultInit + 3] = src[srcMult + multiply[3]];
                dst[destMultInit + 4] = src[srcMult + multiply[4]];
                dst[destMultInit + 5] = src[srcMult + multiply[5]];
                dst[destMultInit + 6] = src[srcMult + multiply[6]];
                dst[destMultInit + 7] = src[srcMult + multiply[7]];
                dst[destMultInit + 8] = src[srcMult + multiply[8]];
                dst[destMultInit + 9] = src[srcMult + multiply[9]];
                dst[destMultInit + 10] = src[srcMult + multiply[10]];
                dst[destMultInit + 11] = src[srcMult + multiply[11]];
                dst[destMultInit + 12] = src[srcMult + multiply[12]];
                dst[destMultInit + 13] = src[srcMult + multiply[13]];
                dst[destMultInit + 14] = src[srcMult + multiply[14]];
                dst[destMultInit + 15] = src[srcMult + multiply[15]];

                destMultInit -= dim;
                srcMult++;

                dst[destMultInit] = src[srcMult];
                dst[destMultInit + 1] = src[srcMult + dim];
                dst[destMultInit + 2] = src[srcMult + multiply[2]];
                dst[destMultInit + 3] = src[srcMult + multiply[3]];
                dst[destMultInit + 4] = src[srcMult + multiply[4]];
                dst[destMultInit + 5] = src[srcMult + multiply[5]];
                dst[destMultInit + 6] = src[srcMult + multiply[6]];
                dst[destMultInit + 7] = src[srcMult + multiply[7]];
                dst[destMultInit + 8] = src[srcMult + multiply[8]];
                dst[destMultInit + 9] = src[srcMult + multiply[9]];
                dst[destMultInit + 10] = src[srcMult + multiply[10]];
                dst[destMultInit + 11] = src[srcMult + multiply[11]];
                dst[destMultInit + 12] = src[srcMult + multiply[12]];
                dst[destMultInit + 13] = src[srcMult + multiply[13]];
                dst[destMultInit + 14] = src[srcMult + multiply[14]];
                dst[destMultInit + 15] = src[srcMult + multiply[15]];

                destMultInit -= dim;
                srcMult++;

                dst[destMultInit] = src[srcMult];
                dst[destMultInit + 1] = src[srcMult + dim];
                dst[destMultInit + 2] = src[srcMult + multiply[2]];
                dst[destMultInit + 3] = src[srcMult + multiply[3]];
                dst[destMultInit + 4] = src[srcMult + multiply[4]];
                dst[destMultInit + 5] = src[srcMult + multiply[5]];
                dst[destMultInit + 6] = src[srcMult + multiply[6]];
                dst[destMultInit + 7] = src[srcMult + multiply[7]];
                dst[destMultInit + 8] = src[srcMult + multiply[8]];
                dst[destMultInit + 9] = src[srcMult + multiply[9]];
                dst[destMultInit + 10] = src[srcMult + multiply[10]];
                dst[destMultInit + 11] = src[srcMult + multiply[11]];
                dst[destMultInit + 12] = src[srcMult + multiply[12]];
                dst[destMultInit + 13] = src[srcMult + multiply[13]];
                dst[destMultInit + 14] = src[srcMult + multiply[14]];
                dst[destMultInit + 15] = src[srcMult + multiply[15]];

                destMultInit -= dim;
                srcMult++;

                dst[destMultInit] = src[srcMult];
                dst[destMultInit + 1] = src[srcMult + dim];
                dst[destMultInit + 2] = src[srcMult + multiply[2]];
                dst[destMultInit + 3] = src[srcMult + multiply[3]];
                dst[destMultInit + 4] = src[srcMult + multiply[4]];
                dst[destMultInit + 5] = src[srcMult + multiply[5]];
                dst[destMultInit + 6] = src[srcMult + multiply[6]];
                dst[destMultInit + 7] = src[srcMult + multiply[7]];
                dst[destMultInit + 8] = src[srcMult + multiply[8]];
                dst[destMultInit + 9] = src[srcMult + multiply[9]];
                dst[destMultInit + 10] = src[srcMult + multiply[10]];
                dst[destMultInit + 11] = src[srcMult + multiply[11]];
                dst[destMultInit + 12] = src[srcMult + multiply[12]];
                dst[destMultInit + 13] = src[srcMult + multiply[13]];
                dst[destMultInit + 14] = src[srcMult + multiply[14]];
                dst[destMultInit + 15] = src[srcMult + multiply[15]];

                destMultInit -= dim;
                srcMult++;

                dst[destMultInit] = src[srcMult];
                dst[destMultInit + 1] = src[srcMult + dim];
                dst[destMultInit + 2] = src[srcMult + multiply[2]];
                dst[destMultInit + 3] = src[srcMult + multiply[3]];
                dst[destMultInit + 4] = src[srcMult + multiply[4]];
                dst[destMultInit + 5] = src[srcMult + multiply[5]];
                dst[destMultInit + 6] = src[srcMult + multiply[6]];
                dst[destMultInit + 7] = src[srcMult + multiply[7]];
                dst[destMultInit + 8] = src[srcMult + multiply[8]];
                dst[destMultInit + 9] = src[srcMult + multiply[9]];
                dst[destMultInit + 10] = src[srcMult + multiply[10]];
                dst[destMultInit + 11] = src[srcMult + multiply[11]];
                dst[destMultInit + 12] = src[srcMult + multiply[12]];
                dst[destMultInit + 13] = src[srcMult + multiply[13]];
                dst[destMultInit + 14] = src[srcMult + multiply[14]];
                dst[destMultInit + 15] = src[srcMult + multiply[15]];
            }
        }
    }
}


/* 
 * tiling attempt
 */
char rotate_two_descr[] = "tiling attempt";
void attempt_two(int dim, pixel *src, pixel *dst) 
{
    //clock_t begin;

    /* Mark beginning time */
    //begin = clock();

    //printf("dIM %d \n", dim);

    int N = 32;

    //RIDX (a1, a2) (a1*N + a2)

    int i, j, bi, bj;
    int jN = N, iN, destMult, srcMult, srcMultInit;
    int dimminus = (dim-1) * dim;

    for (j = 0; j < dim; j += N, jN += N)
    {
        iN = N;
        for (i = 0; i < dim; i += N, iN += N)
        {
            //printf("In block %d,%d \n", i, j);
            destMult = dimminus - (j*dim);
            srcMultInit = i * dim;

            for (bj = j; bj < jN - 1; bj += 2, destMult -= dim + dim)
            {
                srcMult = srcMultInit;
                for (bi = i; bi < iN - 1; bi += 2, srcMult += dim + dim)
                {
                    dst[destMult + bi] = src[srcMult + bj];
                    dst[destMult + bi + 1] = src[srcMult + dim + bj];
                }

                srcMult = srcMultInit;
                for (bi = i; bi < iN - 1; bi += 2, srcMult += dim + dim)
                {
                    dst[destMult - dim + bi] = src[srcMult + bj + 1];
                    dst[destMult - dim + bi + 1] = src[srcMult + dim + bj + 1];
                }
            }
        }
    }

    //printf ("time spent: %lf \n", (double)(clock() - begin) * 1000 / CLOCKS_PER_SEC);
}

/* 
 * tiling attempt with 3 loops
 */
char rotate_three_descr[] = "less loop tiling attempt";
void attempt_three(int dim, pixel *src, pixel *dst) 
{
    //clock_t begin;

    /* Mark beginning time */
    //begin = clock();

    //printf("dIM %d \n", dim);

    int M = 32;
    int N = (dim > 1024)?16:32;
    int k, bi, bj, i = 0, j = 0, destMult, srcMult, srcMultInit;
    int jN = M, iN = N, dimminus = (dim-1) * dim;

    int num_blocks = (dim * dim) / (M * N);

    for (k = 0; k < num_blocks; k++)
    {
        //printf("doing block %d of %d: i = %d , j = %d \n", k, num_blocks, i, j);
        destMult = dimminus - (j*dim);
        srcMultInit = i * dim;

        for (bj = j; bj < jN - 1; bj += 2, destMult -= dim + dim)
        {
            srcMult = srcMultInit;
            for (bi = i; bi < iN - 1; bi += 2, srcMult += dim + dim)
            {
                dst[destMult + bi] = src[srcMult + bj];
                dst[destMult + bi + 1] = src[srcMult + dim + bj];
            }

            srcMult = srcMultInit;
            for (bi = i; bi < iN - 1; bi += 2, srcMult += dim + dim)
            {
                dst[destMult - dim + bi] = src[srcMult + bj + 1];
                dst[destMult - dim + bi + 1] = src[srcMult + dim + bj + 1];
            }
        }

        i += N;
        iN += N;

        if ( i >= dim ) {
            i = 0;
            iN = N;

            j += M;
            jN += M;
        }
    }

    //printf ("time spent: %lf \n", (double)(clock() - begin) * 1000 / CLOCKS_PER_SEC);
}

/* 
 * transpose and swap attempt
 */
char rotate_four_descr[] = "transpose and swap attempt";
void attempt_four(int dim, pixel *src, pixel *dst) 
{
    clock_t begin;

    /* Mark beginning time */
    begin = clock();

    printf("dIM %d \n", dim);

    /*
    dim = 4;

    pixel *srcTest = malloc(sizeof(pixel)*16);

    pixel *dstTest = malloc(sizeof(pixel)*16);

    int kk;
    for(kk= 0; kk<16;kk++)
    {
        srcTest[kk].red = kk;
        srcTest[kk].green = kk;
        srcTest[kk].blue = kk;

        //printf (" src[%d]: {%d,%d,%d}\n", kk, srcTest[kk].red, srcTest[kk].green, srcTest[kk].blue);

        dstTest[kk].red = 0;
        dstTest[kk].green = 0;
        dstTest[kk].blue = 0;
    }
    */

    int N = 32;
    int k, bi, bj, i = 0, j = 0;

    int num_blocks = pow(dim / N, 2);

    pixel temp;
    pixel *swapA;
    pixel *swapB;

    // Transpose in block
    for (k = 0; k < num_blocks; k++)
    {
        //printf("doing block %d of %d: i = %d , j = %d \n", k, num_blocks, i, j);
        
        for (bj = 0; bj < N; bj++)
        {
            for (bi = 0; bi < N; bi++)
            {
                swapA = &src[RIDX(i + bi, j + bj, dim)];
                swapB = &src[RIDX(j + bj, i + bi, dim)];

                temp.red = swapA->red;
                temp.green = swapA->green;
                temp.blue = swapA->blue;

                swapA->red = swapB->red;
                swapA->green = swapB->green;
                swapA->blue = swapB->blue;
                
                swapB->red = temp.red;
                swapB->green = temp.green;
                swapB->blue = temp.blue;
                //dst[RIDX(j + bj, i + bi, dim)] = src[RIDX(i + bi, j + bj, dim)];
            }
        }

        i += N;
        if ( i >= dim ) {
            i = 0;
            j += N;
        }
    }



    /*for(kk= 0; kk<16;kk++)
    {
        printf (" dst[%d]: {%d}\n", kk, dstTest[kk].red);
    }*/

    // Swap rows
    /*
    for (i = 0; i < dim / 2; i++)
    {
        for (j = 0; j < dim; j++)
        {
            //printf("doing swap i = %d , j = %d , dim-1-i = %d \n", i, j, dim-1-i);

            swapA = &dst[RIDX(i, j, dim)];
            swapB = &dst[RIDX(dim-1-i, j, dim)];

            //printf (" A: {%d}, B: {%d}\n", swapA->red, swapB->red);
            
            temp.red = swapA->red;
            temp.green = swapA->green;
            temp.blue = swapA->blue;

            swapA->red = swapB->red;
            swapA->green = swapB->green;
            swapA->blue = swapB->blue;
            
            swapB->red = temp.red;
            swapB->green = temp.green;
            swapB->blue = temp.blue;

            //printf (" Swap A: {%d}, B: {%d}\n", swapA->red, swapB->red);
        }
    }
    */

    printf ("time spent: %lf \n", (double)(clock() - begin) * 1000 / CLOCKS_PER_SEC);

    /*for(kk= 0; kk<16;kk++)
    {
        printf (" dst[%d]: {%d}\n", kk, dstTest[kk].red);
    }*/

    //exit(0);
}

/* 
 * transpose and swap complete matrix attempt
 */
char rotate_five_descr[] = "transpose and swap complete matrix attempt";
void attempt_five(int dim, pixel *src, pixel *dst) 
{
    clock_t begin;

    /* Mark beginning time */
    begin = clock();

    printf("dIM %d \n", dim);
    int i, j;

    pixel temp;
    pixel *swapA;
    pixel *swapB;

    // Transpose full
    for (j = 0; j < dim; j++)
    {
        for (i = 0; i < dim; i++)
        {
            dst[RIDX(j, i, dim)] = src[RIDX(i, j, dim)];
        }
    }

    // Swap rows 
    for (i = 0; i < dim / 2; i++)
    {
        for (j = 0; j < dim; j++)
        {
            swapA = &dst[RIDX(i, j, dim)];
            swapB = &dst[RIDX(dim-1-i, j, dim)];

            temp.red = swapA->red;
            temp.green = swapA->green;
            temp.blue = swapA->blue;

            swapA->red = swapB->red;
            swapA->green = swapB->green;
            swapA->blue = swapB->blue;
            
            swapB->red = temp.red;
            swapB->green = temp.green;
            swapB->blue = temp.blue;
        }
    }

    printf ("time spent: %lf \n", (double)(clock() - begin) * 1000 / CLOCKS_PER_SEC);
}

/* 
 * total unroll tiling attempt (3 loops)
 * 
 *      "ONE CANNOT KNOW TRUE PEACE, UNLESS THEY KNOW TRUE PAIN. THERE IS NO PEACE WITHOUT PAIN."
 *      ~Nagato
 */
char rotate_six_descr[] = "total unroll";
void attempt_six(int dim, pixel *src, pixel *dst) 
{
    //copied to rotate
}


/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_rotate_functions() 
{
    add_rotate_function(&naive_rotate, naive_rotate_descr);   
    add_rotate_function(&rotate, rotate_descr);   

    //add_rotate_function(&attempt_two, rotate_two_descr);   
    //add_rotate_function(&attempt_three, rotate_three_descr);   
    //add_rotate_function(&attempt_four, rotate_four_descr);   
    //add_rotate_function(&attempt_five, rotate_five_descr);   
    //add_rotate_function(&attempt_six, rotate_six_descr);   
    //add_rotate_function(&attempt_seven, rotate_seven_descr);   
    //add_rotate_function(&attempt_eight, rotate_eight_descr);   
    //add_rotate_function(&attempt_nine, rotate_nine_descr);   
    //add_rotate_function(&attempt_ten, rotate_ten_descr);   
    //add_rotate_function(&attempt_eleven, rotate_eleven_descr);   

    /* ... Register additional rotate functions here */
}

