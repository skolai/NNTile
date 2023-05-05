/*! @copyright (c) 2022-2023 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file src/kernel/dgelutanh/cpu.cc
 * Derivative of approximate GeLU operation on CPU based on tanh function
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @date 2023-05-05
 * */

#include "nntile/kernel/dgelutanh/cpu.hh"
#include <cmath>

namespace nntile
{
namespace kernel
{
namespace dgelutanh
{

template<typename T>
void cpu(Index nelems, T *data)
    noexcept
//! Derivative of approximate GeLU operation on CPU
/*! Applies the following derivative of approximation of the GeLU function:
 * GeLU(z) \approx AGeLU(z)
 * f(z) = -2 sqrt(2/pi) z (1+0.044715z^2)
 * AGeLU(z) = z / (1+exp(f(z))
 * AGeLU'(z) = 1/(1+exp(f(z)) - (zf'(z)exp(f(z)))/(1+exp(f(z)))^2
 * AGeLU'(z) = (1-(zf'(z)-1)exp(f(z))) / (1+exp(f(z)))^2
 * zf'(z) = -2 sqrt(2/pi) z (1+3*0.044715z^2)
 *
 * @params[in] nelems: Number of elements in a buffer
 * @params[inout] data: Buffer to apply derivative of approximate GeLU
 * */
{
    // Constants
    constexpr T pi = 3.141592653589793238462643383279502884L,
        zero = 0, one = 1, pt5 = 0.5, f1 = T{0.044715};
    // Square root is not constexpr by standard, proceed with a static const
    static const T sqrt_pi = std::sqrt(pi), sqrt_2 = std::sqrt(T{2}),
        f2 = sqrt_2/sqrt_pi, f3 = -T{2}*f2, f4 = f3*f1, f5 = T{3}*f4;
    for(Index i = 0; i < nelems; ++i)
    {
        T z = data[i];
        T z2 = z * z;
        T y1 = z * (f3 + f4*z2);
        T y2 = z * (f3 + f5*z2);
        T expy1 = std::exp(y1);
        if(std::isinf(expy1))
        {
            data[i] = zero;
        }
        else
        {
            T inv_expy1p1 = one / (expy1 + one);
            data[i] = (one-y2*(one-inv_expy1p1)) * inv_expy1p1;
        }
    }
}

// Explicit instantiation
template
void cpu<fp32_t>(Index nelems, fp32_t *data)
    noexcept;

template
void cpu<fp64_t>(Index nelems, fp64_t *data)
    noexcept;

} // namespace dgelutanh
} // namespace kernel
} // namespace nntile

