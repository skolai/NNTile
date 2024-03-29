/*! @copyright (c) 2022-2022 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file include/nntile/tile/randn.hh
 * Randn operation for Tile<T>
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @date 2022-08-31
 * */

#pragma once

#include <nntile/tile/tile.hh>

namespace nntile
{
namespace tile
{

template<typename T>
void randn_async(const Tile<T> &dst, const std::vector<Index> &start,
        const std::vector<Index> &underlying_shape, unsigned long long seed,
        T mean, T stddev);

template<typename T>
void randn(const Tile<T> &dst, const std::vector<Index> &start,
        const std::vector<Index> &underlying_shape, unsigned long long seed,
        T mean, T stddev);

} // namespace tile
} // namespace nntile

