/*! @copyright (c) 2022-2022 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file src/starpu/normalize.cc
 * Normalize operation for StarPU buffer
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @date 2022-08-31
 * */

#include "nntile/starpu/normalize.hh"
#include "nntile/kernel/normalize/cpu.hh"
#ifdef NNTILE_USE_CUDA
#   include "nntile/kernel/normalize/cuda.hh"
#endif // NNTILE_USE_CUDA

namespace nntile
{
namespace starpu
{
namespace normalize
{

//! Renormalize buffer along middle axis of StarPU buffer
template<typename T>
void cpu(void *buffers[], void *cl_args)
    noexcept
{
    // Get arguments
    auto args = reinterpret_cast<args_t<T> *>(cl_args);
    // Get interfaces
    auto interfaces = reinterpret_cast<StarpuVariableInterface **>(buffers);
    const T *gamma_beta = interfaces[0]->get_ptr<T>();
    const T *gamma = &gamma_beta[0], *beta = &gamma_beta[1];
    const T *sumnorm = interfaces[1]->get_ptr<T>();
    T *dst = interfaces[2]->get_ptr<T>();
    // Launch kernel
    kernel::normalize::cpu<T>(args->m, args->n, args->k, args->l, args->eps,
            gamma, beta, sumnorm, dst);
}

#ifdef NNTILE_USE_CUDA
//! Renormalize buffer along middle axis of StarPU buffer
template<typename T>
void cuda(void *buffers[], void *cl_args)
    noexcept
{
    // Get arguments
    auto args = reinterpret_cast<args_t<T> *>(cl_args);
    // Get interfaces
    auto interfaces = reinterpret_cast<StarpuVariableInterface **>(buffers);
    const T *gamma_beta = interfaces[0]->get_ptr<T>();
    const T *gamma = &gamma_beta[0], *beta = &gamma_beta[1];
    const T *sumnorm = interfaces[1]->get_ptr<T>();
    T *dst = interfaces[2]->get_ptr<T>();
    // Get CUDA stream
    cudaStream_t stream = starpu_cuda_get_local_stream();
    // Launch kernel
    kernel::normalize::cuda<T>(stream, args->m, args->n, args->k, args->l,
            args->eps, gamma, beta, sumnorm, dst);
}
#endif // NNTILE_USE_CUDA

//! Footprint for normalize tasks that depends only on m, n and k
template<typename T>
static
uint32_t footprint(struct starpu_task *task)
{
    // Get arguments
    auto args = reinterpret_cast<args_t<T> *>(task->cl_arg);
    // Apply hash over parameters m, n and k. This way if we swap values of m,
    // n and k, then the total size of buffers will remain the same, but the
    // footprint will be different
    uint32_t hash = 0;
    hash = starpu_hash_crc32c_be_n(&args->m, sizeof(args->m), hash);
    hash = starpu_hash_crc32c_be_n(&args->n, sizeof(args->n), hash);
    hash = starpu_hash_crc32c_be_n(&args->k, sizeof(args->k), hash);
    return hash;
}

StarpuCodelet codelet_fp32, codelet_fp64;

void init()
{
    codelet_fp32.init("nntile_normalize_fp32",
            footprint<fp32_t>,
            {cpu<fp32_t>},
#ifdef NNTILE_USE_CUDA
            {cuda<fp32_t>}
#else // NNTILE_USE_CUDA
            {}
#endif // NNTILE_USE_CUDA
            );
    codelet_fp64.init("nntile_normalize_fp64",
            footprint<fp64_t>,
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
void submit(Index m, Index n, Index k, Index l, T eps,
        starpu_data_handle_t gamma_beta, starpu_data_handle_t sumnorm,
        starpu_data_handle_t dst)
//! Insert normalize task into StarPU pool of tasks
/*! No argument checking is performed. All the inputs are packed and passed to
 * starpu_task_insert() function. If task submission fails, this routines
 * throws an std::runtime_error() exception.
 * */
{
    // Codelet arguments
    auto args = new args_t<T>
    {
        .m = m,
        .n = n,
        .k = k,
        .l = l,
        .eps = eps
    };
    fp64_t nflops = 14 * m * n * k;
    // Submit task
    int ret = starpu_task_insert(codelet<T>(),
            STARPU_R, gamma_beta,
            STARPU_R, sumnorm,
            STARPU_RW, dst,
            STARPU_CL_ARGS, args, sizeof(*args),
            STARPU_FLOPS, nflops,
            0);
    // Check submission
    if(ret != 0)
    {
        throw std::runtime_error("Error in normalize task submission");
    }
}

// Explicit instantiation
template
void submit<fp32_t>(Index m, Index n, Index k, Index l, fp32_t eps,
        starpu_data_handle_t gamma_beta, starpu_data_handle_t sumnorm,
        starpu_data_handle_t dst);

template
void submit<fp64_t>(Index m, Index n, Index k, Index l, fp64_t eps,
        starpu_data_handle_t gamma_beta, starpu_data_handle_t sumnorm,
        starpu_data_handle_t dst);

} // namespace normalize
} // namespace starpu
} // namespace nntile
