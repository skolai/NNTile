/*! @copyright (c) 2022-2023 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file include/nntile/tensor/logsumexp.hh
 * Log sum exp of Tensor<T> along axis
 *
 * @version 1.0.0
 * @author Aleksandr Katrutsa
 * @date 2023-05-05
 * */

#pragma once

#include <nntile/tensor/tensor.hh>

namespace nntile
{
namespace tensor
{

template<typename T>
void logsumexp_async(const Tensor<T> &src, const Tensor<T> &dst);

template<typename T>
void logsumexp(const Tensor<T> &src, const Tensor<T> &dst);

} // namespace tensor
} // namespace nntile

