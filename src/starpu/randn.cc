/*! @copyright (c) 2022-2022 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file src/starpu/randn.cc
 * Randn operation on a StarPU buffer
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @date 2022-09-27
 * */

#include "nntile/starpu/randn.hh"
#include "nntile/kernel/randn.hh"

namespace nntile
{
namespace starpu
{
namespace randn
{

//! Randn operation on StarPU buffers
template<typename T>
void cpu(void *buffers[], void *cl_args)
    noexcept
{
    // Get arguments
    const Index *ndim_ptr, *nelems_ptr, *start, *shape, *stride,
          *underlying_shape;
    const unsigned long long *seed_ptr;
    const T *mean_ptr, *stddev_ptr;
    Config::unpack_args_ptr(cl_args, ndim_ptr, nelems_ptr, seed_ptr, mean_ptr,
            stddev_ptr, start, shape, stride, underlying_shape);
    // Get interfaces
    Index ndim = *ndim_ptr;
    auto interfaces = reinterpret_cast<VariableInterface **>(buffers);
    T *data = interfaces[0]->get_ptr<T>();
    Index *tmp_index = interfaces[1]->get_ptr<Index>();
    // Launch kernel
    kernel::randn::cpu<T>(ndim, *nelems_ptr, *seed_ptr, *mean_ptr,
            *stddev_ptr, start, shape, underlying_shape, data, stride,
            tmp_index);
}

//! Randn operation on StarPU buffers for ndim=0
template<typename T>
void cpu_ndim0(void *buffers[], void *cl_args)
    noexcept
{
    // Get arguments
    const unsigned long long *seed_ptr;
    const T *mean_ptr, *stddev_ptr;
    Config::unpack_args_ptr(cl_args, seed_ptr, mean_ptr, stddev_ptr);
    // Get interfaces
    auto interfaces = reinterpret_cast<VariableInterface **>(buffers);
    T *data = interfaces[0]->get_ptr<T>();
    // Launch kernel
    kernel::randn::cpu_ndim0<T>(*seed_ptr, *mean_ptr, *stddev_ptr, data);
}

//! Footprint for randn tasks that depend on shape
template<typename T>
static
uint32_t footprint(struct starpu_task *task)
{
    // Get arguments
    const Index *ndim_ptr, *nelems_ptr, *start, *shape, *stride,
          *underlying_shape;
    const unsigned long long *seed_ptr;
    const T *mean_ptr, *stddev_ptr;
    Config::unpack_args_ptr(task->cl_arg, ndim_ptr, nelems_ptr, seed_ptr,
            mean_ptr, stddev_ptr, start, shape, stride, underlying_shape);
    std::size_t shape_size = *ndim_ptr * sizeof(*shape);
    // Apply hash over parameter copy_shape
    return starpu_hash_crc32c_be_n(shape, shape_size, 0);
}

Codelet codelet_fp32, codelet_fp64, codelet_fp32_ndim0, codelet_fp64_ndim0;

void init()
{
    codelet_fp32.init("nntile_randn_fp32",
            footprint<fp32_t>,
            {cpu<fp32_t>},
            {});
    codelet_fp64.init("nntile_randn_fp64",
            footprint<fp64_t>,
            {cpu<fp64_t>},
            {});
    codelet_fp32_ndim0.init("nntile_randn_fp32",
            nullptr,
            {cpu_ndim0<fp32_t>},
            {});
    codelet_fp64_ndim0.init("nntile_randn_fp64",
            nullptr,
            {cpu_ndim0<fp64_t>},
            {});
}

void restrict_where(uint32_t where)
{
    codelet_fp32.restrict_where(where);
    codelet_fp64.restrict_where(where);
    codelet_fp32_ndim0.restrict_where(where);
    codelet_fp64_ndim0.restrict_where(where);
}

void restore_where()
{
    codelet_fp32.restore_where();
    codelet_fp64.restore_where();
    codelet_fp32_ndim0.restore_where();
    codelet_fp64_ndim0.restore_where();
}

template<typename T>
void submit(Index ndim, Index nelems, unsigned long long seed,
        T mean, T stddev, const std::vector<Index> &start,
        const std::vector<Index> &shape, const std::vector<Index> &stride,
        const std::vector<Index> &underlying_shape, Handle data,
        Handle tmp_index)
{
    fp64_t nflops = 2 * nelems;
    // Submit task
    int ret;
    if(ndim > 0)
    {
        ret = starpu_task_insert(codelet<T>(),
                STARPU_VALUE, &ndim, sizeof(ndim),
                STARPU_VALUE, &nelems, sizeof(nelems),
                STARPU_VALUE, &seed, sizeof(seed),
                STARPU_VALUE, &mean, sizeof(mean),
                STARPU_VALUE, &stddev, sizeof(stddev),
                STARPU_VALUE, &start[0], ndim*sizeof(start[0]),
                STARPU_VALUE, &shape[0], ndim*sizeof(shape[0]),
                STARPU_VALUE, &stride[0], ndim*sizeof(stride[0]),
                STARPU_VALUE, &underlying_shape[0],
                ndim*sizeof(underlying_shape[0]),
                STARPU_W, static_cast<starpu_data_handle_t>(data),
                STARPU_SCRATCH, static_cast<starpu_data_handle_t>(tmp_index),
                STARPU_FLOPS, nflops,
                0);
    }
    else
    {
        ret = starpu_task_insert(codelet_ndim0<T>(),
                STARPU_VALUE, &seed, sizeof(seed),
                STARPU_VALUE, &mean, sizeof(mean),
                STARPU_VALUE, &stddev, sizeof(stddev),
                STARPU_W, static_cast<starpu_data_handle_t>(data),
                STARPU_FLOPS, nflops,
                0);
    }
    // Check submission
    if(ret != 0)
    {
        throw std::runtime_error("Error in randn task submission");
    }
}

// Explicit instantiation
template
void submit<fp32_t>(Index ndim, Index nelems, unsigned long long seed,
        fp32_t mean, fp32_t stddev, const std::vector<Index> &start,
        const std::vector<Index> &shape, const std::vector<Index> &stride,
        const std::vector<Index> &underlying_shape, Handle data,
        Handle tmp_index);

template
void submit<fp64_t>(Index ndim, Index nelems, unsigned long long seed,
        fp64_t mean, fp64_t stddev, const std::vector<Index> &start,
        const std::vector<Index> &shape, const std::vector<Index> &stride,
        const std::vector<Index> &underlying_shape, Handle data,
        Handle tmp_index);

} // namespace randn
} // namespace starpu
} // namespace nntile

