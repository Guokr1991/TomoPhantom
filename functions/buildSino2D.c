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
#include <math.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include "omp.h"

#include "buildSino2D_core.h"

#define M_PI 3.14159265358979323846

/* Function to create 2D analytical sinograms (parallel beam geometry) to 2D phantoms using Phantom2DLibrary.dat
 * (MATLAB wrapper)
 *
 * Input Parameters:
 * 1. Model number (see Phantom2DLibrary.dat) [required]
 * 2. ImageSize in pixels (N x N) [required]
 * 3. Detector array size P (in pixels) [required]
 * 4. Projection angles Th (in degrees) [required] 
 * 5. An absolute path to the file Phantom2DLibrary.dat (see OS-specific syntax-differences) [required]
 * 6. ImageCentring, choose 'radon' or 'astra' (default) [optional]
 *
 * Output:
 * 1. 2D sinogram size of [P, length(Th)]
 */

void mexFunction(
        int nlhs, mxArray *plhs[],
        int nrhs, const mxArray *prhs[])
        
{
    int ModelSelected, N, CenTypeIn, P;
    float *A, *Th;
    mwSize  NStructElems;
    char *ModelParameters_PATH;
    
    /*Handling Matlab input data*/
    if ((nrhs < 5) && (nrhs > 6)) mexErrMsgTxt("Input of 5 or 6 parameters is required: Model, ImageSize, Detector Size, Projection angles, PATH, Centering");
    if (mxGetClassID(prhs[3]) != mxSINGLE_CLASS) {mexErrMsgTxt("The vector of angles must be in a single precision"); }
    
    ModelSelected  = (int) mxGetScalar(prhs[0]); /* selected model */
    N  = (int) mxGetScalar(prhs[1]); /* choosen dimension (N x N) */
    P  = (int) mxGetScalar(prhs[2]); /* detector size */
    Th  = (float*) mxGetData(prhs[3]); /* angles */
    ModelParameters_PATH = mxArrayToString(prhs[4]); /* provide an absolute path to the file */      
    CenTypeIn = 1; /* astra-type centering is the default one */
    
   if (nrhs == 6)  {
        char *CenType;
        CenType = mxArrayToString(prhs[5]); /* 'radon' or 'astra' (default) */
        if ((strcmp(CenType, "radon") != 0) && (strcmp(CenType, "astra") != 0)) mexErrMsgTxt("Choose 'radon' or 'astra''");
        if (strcmp(CenType, "radon") == 0)  CenTypeIn = 0;  /* enable 'radon'-type centaering */
        mxFree(CenType);
    }
    NStructElems = mxGetNumberOfElements(prhs[3]);
    
    /*Handling Matlab output data*/
    int N_dims[] = {P, NStructElems}; /*format: detectors, angles dim*/
    A = (float*)mxGetPr(plhs[0] = mxCreateNumericArray(2, N_dims, mxSINGLE_CLASS, mxREAL));    
        
    buildSino2D_core(A, ModelSelected, N, P, Th, (int)NStructElems, CenTypeIn, ModelParameters_PATH);
    
    mxFree(ModelParameters_PATH);
}

