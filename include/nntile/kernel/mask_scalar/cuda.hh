/*! @copyright (c) 2022-2023 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file include/nntile/kernel/mask_scalar/cuda.hh
 * Mask operation with scalar on CUDA
 *
 * @version 1.0.0
 * @author Aleksandr Katrutsa
 * @author Aleksandr Mikhalev
 * @date 2023-06-29
 * */

#pragma once

#include <nntile/base_types.hh>
#include <cuda_runtime.h>

namespace nntile
{
namespace kernel
{
namespace mask_scalar
{

template<typename T>
void cuda(cudaStream_t stream, Index nrows, Index ncols, const bool_t *mask,
        T val, T *data)
    noexcept;

} // namespace mask_scalar
} // namespace kernel
} // namespace nntile
