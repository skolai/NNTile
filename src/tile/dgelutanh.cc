/*! @copyright (c) 2022-2022 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file src/tile/dgelutanh.cc
 * Derivative of approximate GeLU operation for Tile<T>
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @date 2022-10-26
 * */

#include "nntile/tile/dgelutanh.hh"
#include "nntile/starpu/dgelutanh.hh"

namespace nntile
{
namespace tile
{

//! Blocking version of tile-wise derivative of approximate GeLU operation
/*! @param[inout] A: Tile for the element-wise derivative of GeLU operation
 * */
template<typename T>
void dgelutanh_async(const Tile<T> &A)
{
    // Submit task without any arguments checked
    starpu::dgelutanh::submit<T>(A.nelems, A);
}

//! Blocking version of tile-wise derivative of approximate GeLU operation
/*! @param[inout] A: Tile for the element-wise derivative of GeLU operation
 * */
template<typename T>
void dgelutanh(const Tile<T> &A)
{
    dgelutanh_async<T>(A);
    starpu_task_wait_for_all();
}

// Explicit instantiation
template
void dgelutanh<fp32_t>(const Tile<fp32_t> &A);

template
void dgelutanh<fp64_t>(const Tile<fp64_t> &A);

} // namespace tile
} // namespace nntile

