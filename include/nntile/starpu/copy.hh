/*! @copyright (c) 2022-2022 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file include/nntile/starpu/copy.hh
 * Smart copy StarPU buffer
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @date 2022-08-31
 * */

#pragma once

#include <nntile/base_types.hh>
#include <nntile/starpu.hh>

namespace nntile
{
namespace starpu
{
namespace copy
{

//! Smart copying through StarPU buffers
template<typename T>
void cpu(void *buffers[], void *cl_args)
    noexcept;

extern StarpuCodelet codelet_fp32, codelet_fp64;

template<typename T>
constexpr StarpuCodelet *codelet()
{
    throw std::runtime_error("Non-supported type");
    return nullptr;
}

template<>
constexpr StarpuCodelet *codelet<fp32_t>()
{
    return &codelet_fp32;
}

template<>
constexpr StarpuCodelet *codelet<fp64_t>()
{
    return &codelet_fp64;
}

void init();

void restrict_where(uint32_t where);

void restore_where();

template<typename T>
void submit(Index ndim, const std::vector<Index> &src_start,
        const std::vector<Index> &src_stride,
        const std::vector<Index> &dst_start,
        const std::vector<Index> &dst_stride,
        const std::vector<Index> &copy_shape,
        starpu_data_handle_t src, starpu_data_handle_t dst,
        starpu_data_handle_t tmp_index, starpu_data_access_mode mode);

} // namespace copy
} // namespace starpu
} // namespace nntile
