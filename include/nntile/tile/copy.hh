/*! @copyright (c) 2022-2022 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file include/nntile/tile/copy.hh
 * Copy one tile into another
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @date 2022-09-12
 * */

#pragma once

#include <nntile/tile/tile.hh>

namespace nntile
{
namespace tile
{

template<typename T>
void copy_async(const Tile<T> &src, const Tile<T> &dst);

template<typename T>
void copy(const Tile<T> &src, const Tile<T> &dst);

} // namespace tile
} // namespace nntile

