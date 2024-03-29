/*! @randnright (c) 2022-2022 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file include/nntile/starpu/randn.hh
 * Randn operation on StarPU buffer
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @date 2022-09-27
 * */

#pragma once

#include <nntile/base_types.hh>
#include <nntile/starpu/config.hh>

namespace nntile
{
namespace starpu
{
namespace randn
{

// Randn operation on StarPU buffers
template<typename T>
void cpu(void *buffers[], void *cl_args)
    noexcept;

// Randn operation on StarPU buffers
template<typename T>
void cpu_ndim0(void *buffers[], void *cl_args)
    noexcept;

extern Codelet codelet_fp32, codelet_fp64,
       codelet_fp32_ndim0, codelet_fp64_ndim0;

template<typename T>
constexpr Codelet *codelet()
{
    throw std::runtime_error("Non-supported type");
    return nullptr;
}

template<>
constexpr Codelet *codelet<fp32_t>()
{
    return &codelet_fp32;
}

template<>
constexpr Codelet *codelet<fp64_t>()
{
    return &codelet_fp64;
}

template<typename T>
constexpr Codelet *codelet_ndim0()
{
    throw std::runtime_error("Non-supported type");
    return nullptr;
}

template<>
constexpr Codelet *codelet_ndim0<fp32_t>()
{
    return &codelet_fp32_ndim0;
}

template<>
constexpr Codelet *codelet_ndim0<fp64_t>()
{
    return &codelet_fp64_ndim0;
}

void init();

void restrict_where(uint32_t where);

void restore_where();

template<typename T>
void submit(Index ndim, Index nelems, unsigned long long seed,
        T mean, T stddev, const std::vector<Index> &start,
        const std::vector<Index> &shape, const std::vector<Index> &stride,
        const std::vector<Index> &underlying_shape, Handle data,
        Handle tmp_index);

} // namespace randn
} // namespace starpu
} // namespace nntile

