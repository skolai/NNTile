/*! @copyright (c) 2022-2023 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file src/starpu/logsumexp.cc
 * Log of sum of exponents for StarPU buffer
 *
 * @version 1.0.0
 * @author Aleksandr Katrutsa
 * @author Aleksandr Mikhalev
 * @date 2023-06-30
 * */

#include "nntile/starpu/logsumexp.hh"
#include "nntile/kernel/logsumexp.hh"
#include <cstdlib>

namespace nntile
{
namespace starpu
{
namespace logsumexp
{

template<typename T>
void cpu(void *buffers[], void *cl_args)
    noexcept
{
    // Get arguments
    Index nelems = reinterpret_cast<Index *>(cl_args)[0];
    // Get interfaces
    auto interfaces = reinterpret_cast<VariableInterface **>(buffers);
    const T *maxsumexp = interfaces[0]->get_ptr<T>();
    T *logsumexp = interfaces[1]->get_ptr<T>();
    // Launch kernel
    kernel::logsumexp::cpu<T>(nelems, maxsumexp, logsumexp);
}

#ifdef NNTILE_USE_CUDA
template<typename T>
void cuda(void *buffers[], void *cl_args)
    noexcept
{
    // Get arguments
    Index nelems = reinterpret_cast<Index *>(cl_args)[0];
    // Get interfaces
    auto interfaces = reinterpret_cast<VariableInterface **>(buffers);
    const T *maxsumexp = interfaces[0]->get_ptr<T>();
    T *logsumexp = interfaces[1]->get_ptr<T>();
    // Get CUDA stream
    cudaStream_t stream = starpu_cuda_get_local_stream();
    // Launch kernel
    kernel::logsumexp::cuda<T>(stream, nelems, maxsumexp, logsumexp);
}
#endif // NNTILE_USE_CUDA

Codelet codelet_fp32, codelet_fp64;

void init()
{
    codelet_fp32.init("nntile_logsumexp_fp32",
            nullptr,
            {cpu<fp32_t>},
#ifdef NNTILE_USE_CUDA
            {cuda<fp32_t>}
#else // NNTILE_USE_CUDA
            {}
#endif // NNTILE_USE_CUDA
            );
    codelet_fp64.init("nntile_logsumexp_fp64",
            nullptr,
            {cpu<fp64_t>},
#ifdef NNTILE_USE_CUDA
            {cuda<fp64_t>}
#else // NNTILE_USE_CUDA
            {}
#endif // NNTILE_USE_CUDA
            );
}

void restrict_where(uint32_t where)
{
    codelet_fp32.restrict_where(where);
    codelet_fp64.restrict_where(where);
}

void restore_where()
{
    codelet_fp32.restore_where();
    codelet_fp64.restore_where();
}

template<typename T>
void submit(Index nelems, Handle maxsumexp, Handle logsumexp)
//! Insert logsumexp task into StarPU pool of tasks
/*! No argument checking is performed. All the inputs are packed and passed to
 * starpu_task_insert() function. If task submission fails, this routines
 * throws an std::runtime_error() exception.
 * */
{
    // Codelet arguments
    Index *nelems_ = (Index *)std::malloc(sizeof(*nelems_));
    *nelems_ = nelems;
    // Submit task
    int ret = starpu_task_insert(codelet<T>(),
            STARPU_R, static_cast<starpu_data_handle_t>(maxsumexp),
            STARPU_CL_ARGS, nelems_, sizeof(*nelems_),
            STARPU_W, static_cast<starpu_data_handle_t>(logsumexp),
            //STARPU_FLOPS, nflops,
            0);
    // Check submission
    if(ret != 0)
    {
        throw std::runtime_error("Error in logsumexp task submission");
    }
}

// Explicit instantiation
template
void submit<fp32_t>(Index nelems, Handle maxsumexp, Handle logsumexp);

template
void submit<fp64_t>(Index nelems, Handle maxsumexp, Handle logsumexp);

} // namespace logsumexp
} // namespace starpu
} // namespace nntile

