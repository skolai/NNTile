/*! @copyright (c) 2022-2023 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file include/nntile/kernel/sumnorm/cpu.hh
 * Sum and Euclidean norm of a buffer on CPU
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @date 2023-05-05
 * */

#pragma once

#include <nntile/base_types.hh>

namespace nntile
{
namespace kernel
{
namespace sumnorm
{

// Compute sum and Euclidean norm along middle axis
template<typename T>
void cpu(Index m, Index n, Index k, const T *src, T *sumnorm)
    noexcept;

} // namespace sumnorm
} // namespace kernel
} // namespace nntile

