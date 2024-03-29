/*! @copyright (c) 2022-2023 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file include/nntile/tile/sum_fiber.hh
 * Sum over fibers into a slice of a Tile<T>
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @date 2023-07-22
 * */

#pragma once

#include <nntile/tile/tile.hh>

namespace nntile
{
namespace tile
{

// Tile-wise sum_fiber
template<typename T>
void sum_fiber_async(T alpha, const Tile<T> &src, T beta, const Tile<T> &dst,
        Index axis, Index batch_ndim);

// Tile-wise sum_fiber
template<typename T>
void sum_fiber(T alpha, const Tile<T> &src, T beta, const Tile<T> &dst,
        Index axis, Index batch_ndim);

} // namespace tile
} // namespace nntile

