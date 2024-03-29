/*! @copyright (c) 2022-2023 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file include/nntile/starpu/subcopy.hh
 * Copy subarray based on contiguous indices
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @author Aleksandr Katrutsa
 * @date 2023-09-01
 * */

#pragma once

#include <nntile/base_types.hh>
#include <nntile/starpu/config.hh>

namespace nntile
{
namespace starpu
{
namespace subcopy
{

// Complex copying through StarPU buffers is available only on CPU
template<typename T>
void cpu(void *buffers[], void *cl_args)
    noexcept;

extern Codelet codelet_fp16, codelet_fp32, codelet_fp64, codelet_int64,
       codelet_bool;

template<typename T>
constexpr Codelet *codelet()
{
    throw std::runtime_error("Non-supported type");
    return nullptr;
}

template<>
constexpr Codelet *codelet<fp16_t>()
{
    return &codelet_fp16;
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

template<>
constexpr Codelet *codelet<Index>()
{
    return &codelet_int64;
}

template<>
constexpr Codelet *codelet<bool_t>()
{
    return &codelet_bool;
}

void init();

void restrict_where(uint32_t where);

void restore_where();

template<typename T>
void submit(Index ndim, const std::vector<Index> &src_start,
        const std::vector<Index> &src_stride,
        const std::vector<Index> &dst_start,
        const std::vector<Index> &dst_stride,
        const std::vector<Index> &copy_shape, Handle src, Handle dst,
        Handle tmp_index, starpu_data_access_mode mode);

} // namespace subcopy
} // namespace starpu
} // namespace nntile

