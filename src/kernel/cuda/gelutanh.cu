/*! @copyright (c) 2022-2022 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file src/kernel/cuda/gelutanh.cu
 * Approximate GeLU operation on CUDA based on tanh function
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @date 2022-08-15
 * */

#include "nntile/kernel/cuda/gelutanh.hh"

namespace nntile
{
namespace kernel
{
namespace cuda
{

template<typename T>
static __global__
void gelutanh_kernel(Index nelems, T *data)
{
    int start = threadIdx.x + blockIdx.x*blockDim.x,
        step = blockDim.x * gridDim.x;
    // Constants
    constexpr T pi = 3.141592653589793238462643383279502884L,
        one = 1, pt5 = 0.5, f1 = T{0.044715};
    // Square root is not constexpr by standard, proceed with a static const
    const T sqrt_pi = sqrt(pi), sqrt_2 = sqrt(T{2}),
        f2 = sqrt_2/sqrt_pi, f3 = -T{2}*f2, f4 = f3*f1;
    for(Index i = start; i < nelems; i += step)
    {
        T z = data[i];
        T y = z * (f3 + f4*z*z);
        data[i] = z / (one+exp(y));
    }
}

template<typename T>
void gelutanh(cudaStream_t stream, Index nelems, T *data)
    noexcept
//! Approximate GeLU operation
/*! Applies the following approximation of the GeLU function:
 * GeLU(z) \approx 0.5z(1+tanh(sqrt(2/pi)(z+0.044715z^3))),
 * which is actually implemented as
 * GeLU(z) \approx z/(1+exp(-2sqrt(2/pi)z(1+0.044715z^2)))
 *
 * @params[in] nelems: Number of elements in a buffer
 * @params[inout] data: Buffer to apply GeLU
 * */
{
    // Source is an m-by-n matrix and destination is an m-by-k-by-n tensor
    // Both source and destination are Fortran-contiguous
    dim3 blocks(256), threads(32);
    (gelutanh_kernel<T>)<<<blocks, threads, 0, stream>>>(nelems, data);
}

// Explicit instantiation
template
void gelutanh<fp32_t>(cudaStream_t stream, Index nelems, fp32_t *data)
    noexcept;

template
void gelutanh<fp64_t>(cudaStream_t stream, Index nelems, fp64_t *data)
    noexcept;

} // namespace cuda
} // namespace kernel
} // namespace nntile

