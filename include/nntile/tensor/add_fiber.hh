/*! @copyright (c) 2022-2023 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file include/nntile/tensor/add_fiber.hh
 * Tensor wrappers for addition of a tensor and a broadcasted fiber
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @date 2023-07-21
 * */

#pragma once

#include <nntile/tensor/tensor.hh>

namespace nntile
{
namespace tensor
{

// Tensor<T> addition of a tensor and a broadcasted fiber
template<typename T>
void add_fiber_async(T alpha, const Tensor<T> &src, T beta,
        const Tensor<T> &dst, Index axis, Index batch_ndim);

// Tensor<T> addition of a tensor and a broadcasted fiber
template<typename T>
void add_fiber(T alpha, const Tensor<T> &src, T beta, const Tensor<T> &dst,
        Index axis, Index batch_ndim);

} // namespace tensor
} // namespace nntile

