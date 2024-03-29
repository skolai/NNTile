/*! @copyright (c) 2022-2023 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file include/nntile/kernel/transpose/cpu.hh
 * Transpose operation on buffers on CPU
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @date 2023-07-20
 * */

#pragma once

#include <nntile/base_types.hh>

namespace nntile
{
namespace kernel
{
namespace transpose
{

// Apply transpose for buffers on CPU
template<typename T>
void cpu(Index m, Index n, T alpha, const T* src, T* dst)
    noexcept;

} // namespace transpose
} // namespace kernel
} // namespace nntile

