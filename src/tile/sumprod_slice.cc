/*! @copyright (c) 2022-2023 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file src/tile/sumprod_slice.cc
 * Sums over fibers into a slice of a product of two Tile<T>
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @date 2023-04-26
 * */

#include "nntile/tile/sumprod_slice.hh"
#include "nntile/starpu/sumprod_slice.hh"

namespace nntile
{
namespace tile
{

template<typename T>
void sumprod_slice_async(T alpha, const Tile<T> &src1, const Tile<T> &src2,
        T beta, const Tile<T> &dst, Index axis)
{
    // Check shapes of src1 and src2
    if(src1.shape != src2.shape)
    {
        throw std::runtime_error("src1.shape != src2.shape");
    }
    // Check dimensions
    if(src1.ndim != dst.ndim+1)
    {
        throw std::runtime_error("src1.ndim != dst.ndim+1");
    }
    Index ndim = src1.ndim;
    // Treat special case of ndim=0
    if(ndim == 0)
    {
        throw std::runtime_error("Scalar input makes no sense");
    }
    // Check axis
    if(axis < 0)
    {
        throw std::runtime_error("axis < 0");
    }
    if(axis >= ndim)
    {
        throw std::runtime_error("axis >= ndim");
    }
    // Check shapes of src1 and dst
    for(Index i = 0; i < axis; ++i)
    {
        if(src1.shape[i] != dst.shape[i])
        {
            throw std::runtime_error("src1.shape[i] != dst.shape[i]");
        }
    }
    for(Index i = axis+1; i < src1.ndim; ++i)
    {
        if(src1.shape[i] != dst.shape[i-1])
        {
            throw std::runtime_error("src1.shape[i] != dst.shape[i-1]");
        }
    }
    // Get sizes
    Index m, n, k;
    m = src1.stride[axis];
    n = src1.matrix_shape[axis+1][1];
    k = src1.shape[axis];
    // Insert task
    starpu::sumprod_slice::submit<T>(m, n, k, alpha, src1, src2, beta, dst);
}

template<typename T>
void sumprod_slice(T alpha, const Tile<T> &src1, const Tile<T> &src2, T beta,
        const Tile<T> &dst, Index axis)
{
    sumprod_slice_async<T>(alpha, src1, src2, beta, dst, axis);
    starpu_task_wait_for_all();
}

// Explicit instantiation
template
void sumprod_slice_async<fp32_t>(fp32_t alpha, const Tile<fp32_t> &src1,
        const Tile<fp32_t> &src2, fp32_t beta, const Tile<fp32_t> &dst,
        Index axis);

template
void sumprod_slice_async<fp64_t>(fp64_t alpha, const Tile<fp64_t> &src1,
        const Tile<fp64_t> &src2, fp64_t beta, const Tile<fp64_t> &dst,
        Index axis);

// Explicit instantiation
template
void sumprod_slice<fp32_t>(fp32_t alpha, const Tile<fp32_t> &src1,
        const Tile<fp32_t> &src2, fp32_t beta, const Tile<fp32_t> &dst,
        Index axis);

template
void sumprod_slice<fp64_t>(fp64_t alpha, const Tile<fp64_t> &src1,
        const Tile<fp64_t> &src2, fp64_t beta, const Tile<fp64_t> &dst,
        Index axis);

} // namespace tile
} // namespace nntile

