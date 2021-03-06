/*
Copyright 2017 Daniil Kazantsev

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "mex.h"
#include <matrix.h>
#include <math.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include "omp.h"

#define M_PI 3.14159265358979323846

/* C-OMP Mex-function to perform forward/inverse deformation according to [1]
 *
 * Input Parameters:
 * 1. A - image to deform (N x N )
 * 2. RFP - propotional to the focal point distance
 * 3. AngleTransform - deformation angle in degrees
 * 4. DeformType - deformation type, 0 - forward, 1 - inverse
 *
 * Output:
 * 1. Deformed image
 *
 * to compile with OMP support: mex DeformObject_C.c CFLAGS="\$CFLAGS -fopenmp -Wall -std=c99" LDFLAGS="\$LDFLAGS -fopenmp"
 * References:
 * [1] D. Kazantsev & V. Pickalov, "New iterative reconstruction methods for fan-beam tomography" IPSE, 2017
 */

double Deform_func(double *A, double *B, double *Tomorange_X_Ar, double H_x, double RFP, double angleRad, int DeformType, int dimX, int dimY);

void mexFunction(
        int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[])
        
{
    int number_of_dims, dimX, dimY, DeformType, i;
    const mwSize  *dim_array;
    double *A, *B, *Tomorange_X_Ar=NULL, RFP, angle, angleRad, Tomorange_Xmin, Tomorange_Xmax, H_x;;
    
    number_of_dims = mxGetNumberOfDimensions(prhs[0]);
    dim_array = mxGetDimensions(prhs[0]);
    
    /*Handling Matlab input data*/
    if (nrhs != 4) mexErrMsgTxt("Input of 4 parameters is required");
    
    A  = (double *) mxGetData(prhs[0]); /* image to deform */
    RFP =  (double) mxGetScalar(prhs[1]); /*  propotional to focal point distance*/
    angle =  (double) mxGetScalar(prhs[2]); /*  deformation angle in degrees */
    DeformType =  (int) mxGetScalar(prhs[3]); /* deformation type, 0 - forward, 1 - inverse  */
    
    /*Handling Matlab output data*/
    dimX = dim_array[0]; dimY = dim_array[1];
    
    if (number_of_dims == 2) {
        
        B = (double*)mxGetPr(plhs[0] = mxCreateNumericArray(2, dim_array, mxDOUBLE_CLASS, mxREAL));
        
        Tomorange_X_Ar = malloc(dimX*sizeof(double));
        
        angleRad = angle*(M_PI/180.0f);
        Tomorange_Xmin = -1.0f;
        Tomorange_Xmax = 1.0f;
        H_x = (Tomorange_Xmax - Tomorange_Xmin)/(dimX);
        for(i=0; i<dimX; i++)  {Tomorange_X_Ar[i] = Tomorange_Xmin + (double)i*H_x;}
                
        /* perform deformation */
        Deform_func(A, B, Tomorange_X_Ar, H_x, RFP, angleRad, DeformType, dimX, dimY); 
    
        free(Tomorange_X_Ar);
    }
}

/* Related Functions */
/*****************************************************************/
double Deform_func(double *A, double *B, double *Tomorange_X_Ar, double H_x, double RFP, double angleRad, int DeformType, int dimX, int dimY)
{
    double i0,j0,xx, yy, xPersp, yPersp, xPersp1, yPersp1, u, v, ll, mm, Ar1step_inv, a, b, c, d;
    int i,j,i1,j1,i2,j2;
    Ar1step_inv = 1.0f/H_x;
#pragma omp parallel for shared(A,B,Tomorange_X_Ar,Ar1step_inv) private(i,j,i1,j1,i2,j2,i0,j0,xx, yy, xPersp, yPersp, xPersp1, yPersp1, u, v, ll, mm, a, b, c, d )
    for(i=0; i<dimX; i++) {
        for(j=0; j<dimY; j++) {
            xx = Tomorange_X_Ar[i]*cos(angleRad) + Tomorange_X_Ar[j]*sin(angleRad);
            yy = -Tomorange_X_Ar[i]*sin(angleRad) + Tomorange_X_Ar[j]*cos(angleRad);
            
            if (DeformType == 0) {
                /* do forward transform*/
                xPersp1 = xx*(1.0f - yy*RFP);
                yPersp1 = yy;
                xPersp = xPersp1*(1.0f - yPersp1*RFP)*cos(angleRad) - yPersp1*sin(angleRad);
                yPersp = xPersp1*(1.0f - yPersp1*RFP)*sin(angleRad) + yPersp1*cos(angleRad);
            }
            else {
                /* do inverse transform */
                xPersp1 = xx/(1.0f - yy*RFP);
                yPersp1 = yy;
                xPersp = xPersp1/(1.0f - yPersp1*RFP)*cos(angleRad) - yPersp1*sin(angleRad);
                yPersp = xPersp1/(1.0f - yPersp1*RFP)*sin(angleRad) + yPersp1*cos(angleRad);
            }
//             printf("%f %f \n", xPersp, yPersp);
            /*Bilinear 2D Interpolation */
            ll = (xPersp - (-1.0f))*Ar1step_inv;
            mm = (yPersp - (-1.0f))*Ar1step_inv;
            
            i0 = (double)floor((double)ll);
            j0 = (double)floor((double)mm);
            u = ll - i0;
            v = mm - j0;
            
            i2 = (int)i0;
            j2 = (int)j0;
            
            i1 = i2+1;
            j1 = j2+1;
            
            a = 0.0f; b = 0.0f; c = 0.0f; d = 0.0f;
            
            if ((i2 >= 0) && (i2 < dimX) && (j2 >= 0) && (j2 < dimX))   a = A[(i2)*dimY + (j2)];
            if ((i2 >= 0) && (i2 < dimX-1) && (j2 >= 0) && (j2 < dimX)) b = A[(i1)*dimY + (j2)];
            if ((i2 >= 0) && (i2 < dimX) && (j2 >= 0) && (j2 < dimX-1)) c = A[(i2)*dimY + (j1)];
            if ((i2 >= 0) && (i2 < dimX-1) && (j2 >= 0) && (j2 < dimX-1))  d = A[(i1)*dimY + (j1)];
            
            B[(i)*dimY + (j)] = (1.0f - u)*(1.0f - v)*a + u*(1.0f - v)*b + (1.0f - u)*v*c+ u*v*d;
            
        }}
    return *B;
}
