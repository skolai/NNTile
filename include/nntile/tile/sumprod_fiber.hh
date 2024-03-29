/*! @copyright (c) 2022-2023 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file include/nntile/tile/sumprod_fiber.hh
 * Sums over fibers into a slice of a product of two Tile<T>
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @date 2023-05-02
 * */

#pragma once

#include <nntile/tile/tile.hh>

namespace nntile
{
namespace tile
{

template<typename T>
void sumprod_fiber_async(T alpha, const Tile<T> &src1, const Tile<T> &src2,
        T beta, const Tile<T> &dst, Index axis);

template<typename T>
void sumprod_fiber(T alpha, const Tile<T> &src1, const Tile<T> &src2, T beta,
        const Tile<T> &dst, Index axis);

} // namespace tile
} // namespace nntile

