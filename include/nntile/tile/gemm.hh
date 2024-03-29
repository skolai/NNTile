/*! @copyright (c) 2022-2023 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file include/nntile/tile/gemm.hh
 * GEMM operation for Tile<T>
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @date 2023-05-04
 * */

#pragma once

#include <nntile/tile/tile.hh>
#include <nntile/constants.hh>

namespace nntile
{
namespace tile
{

// Check if tensors match gemm
void gemm_check(const TransOp &transA, const TileTraits &A,
        const TransOp &transB, const TileTraits &B, const TileTraits &C,
        Index ndim, Index batch_ndim);

// Asynchronous tile-wise gemm operation
template<typename T, typename T_scal>
void gemm_async(T_scal alpha, const TransOp &transA, const Tile<T> &A,
        const TransOp &transB, const Tile<T> &B, T_scal beta, const Tile<T> &C,
        Index ndim, Index batch_ndim);

// Blocking version of tile-wise gemm operation
template<typename T, typename T_scal>
void gemm(T_scal alpha, const TransOp &transA, const Tile<T> &A,
        const TransOp &transB, const Tile<T> &B, T_scal beta, const Tile<T> &C,
        Index ndim, Index batch_ndim);

} // namespace tile
} // namespace nntile

