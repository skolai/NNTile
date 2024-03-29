/*! @copyright (c) 2022-2023 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file include/nntile/tile/pow.hh
 * Power operation for Tile<T>
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @date 2023-04-18
 * */

#pragma once

#include <nntile/tile/tile.hh>

namespace nntile
{
namespace tile
{

// Asynchronous tile-wise power operation
template<typename T>
void pow_async(T alpha, T exp, const Tile<T> &A);

// Blocking version of tile-wise power operation
template<typename T>
void pow(T alpha, T exp, const Tile<T> &A);

} // namespace tile
} // namespace nntile

