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

#include <matrix.h>
#include <math.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include "omp.h"

float buildPhantom3D_core(float *A, int ModelSelected, int N, char *ModelParametersFilename);
float buildPhantom3D_core_single(float *A, int N,  int Object, float C0, float x0, float y0, float z0, float a, float b, float c, float phi_rot);
float parameters_check3D(float C0, float x0, float y0, float z0, float a, float b, float c, float phi_rot);
