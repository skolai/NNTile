/*! @copyright (c) 2022-2023 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file include/nntile/kernel/softmax/cpu.hh
 * Softmax operation on CPU
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @date 2023-11-20
 * */

#pragma once

#include <nntile/base_types.hh>

namespace nntile
{
namespace kernel
{
namespace softmax
{

template<typename T>
void cpu(Index m, Index n, Index k, const T *maxsumexp, const T *src, T alpha,
        T *dst)
    noexcept;

} // namespace softmax
} // namespace kernel
} // namespace nntile

