/*! @copyright (c) 2022-2023 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file src/starpu/adamw_step.cc
 * AdamW operation of StarPU buffers
 *
 * @version 1.0.0
 * @author Aleksandr Katrutsa
 * @author Aleksandr Mikhalev
 * @date 2023-11-26
 * */

#include "nntile/starpu/adamw_step.hh"
#include "nntile/kernel/adamw_step.hh"
#include <cstdlib>

namespace nntile
{
namespace starpu
{
//! StarPU wrappers for one step of AdamW optimizer
namespace adamw_step
{

//! Apply AdamW step on StarPU buffers on CPU
template<typename T>
void cpu(void *buffers[], void *cl_args)
    noexcept
{
    // Get arguments
    auto args = reinterpret_cast<args_t<T> *>(cl_args);
    // Get interfaces
    auto interfaces = reinterpret_cast<VariableInterface **>(buffers);
    T *grad = interfaces[0]->get_ptr<T>();
    T *first_moments = interfaces[1]->get_ptr<T>();
    T *second_moments = interfaces[2]->get_ptr<T>();
    T* p = interfaces[3]->get_ptr<T>();
    // Launch kernel
    kernel::adamw_step::cpu<T>(args->num_iter, args->num_elems, args->beta_1,
            args->beta_2, args->eps, args->lr, args->weight_decay, grad,
            first_moments, second_moments, p);
}

#ifdef NNTILE_USE_CUDA
//! Apply AdamW step operation on StarPU buffer on CUDA
template<typename T>
void cuda(void *buffers[], void *cl_args)
    noexcept
{
    // Get arguments
    auto args = reinterpret_cast<args_t<T> *>(cl_args);
    // Get interfaces
    auto interfaces = reinterpret_cast<VariableInterface **>(buffers);
    T *grad = interfaces[0]->get_ptr<T>();
    T *first_moments = interfaces[1]->get_ptr<T>();
    T *second_moments = interfaces[2]->get_ptr<T>();
    T* p = interfaces[3]->get_ptr<T>();
    // Get CUDA stream
    cudaStream_t stream = starpu_cuda_get_local_stream();
    // Launch kernel
    kernel::adamw_step::cuda<T>(stream, args->num_iter, args->num_elems,
            args->beta_1, args->beta_2, args->eps, args->lr,
            args->weight_decay, grad, first_moments, second_moments, p);
}
#endif // NNTILE_USE_CUDA

Codelet codelet_fp32, codelet_fp64;

void init()
{
    codelet_fp32.init("nntile_adamw_step_fp32",
            nullptr,
            {cpu<fp32_t>},
#ifdef NNTILE_USE_CUDA
            {cuda<fp32_t>}
#else // NNTILE_USE_CUDA
            {}
#endif // NNTILE_USE_CUDA
            );
    codelet_fp64.init("nntile_adamw_step_fp64",
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
void submit(Index num_iter, Index num_elems, T beta_1, T beta_2, T eps, T lr, T weight_decay,
            Handle grad, Handle first_moment, Handle second_moment, Handle p)
{
    // Codelet arguments
    args_t<T>* args = (args_t<T>*)std::malloc(sizeof(*args));
    args->num_iter = num_iter;
    args->num_elems = num_elems;
    args->beta_1 = beta_1;
    args->beta_2 = beta_2;
    args->eps = eps;
    args->lr = lr;
    args->weight_decay = weight_decay;
    //fp64_t nflops = 5 * nelems;
    // Submit task
    enum starpu_data_access_mode moments_mode;
    if (num_iter == 1)
    {
        moments_mode = STARPU_W;
    }
    else
    {
        moments_mode = STARPU_RW;
    }
    int ret = starpu_task_insert(codelet<T>(),
            STARPU_R, static_cast<starpu_data_handle_t>(grad),
            moments_mode, static_cast<starpu_data_handle_t>(first_moment),
            moments_mode, static_cast<starpu_data_handle_t>(second_moment),
            STARPU_RW, static_cast<starpu_data_handle_t>(p),
            STARPU_CL_ARGS, args, sizeof(*args),
            0);
    // Check submission
    if(ret != 0)
    {
        throw std::runtime_error("Error in adamw_step task submission");
    }
}

// Explicit instantiaion
template
void submit<fp32_t>(Index num_iter, Index num_elems, fp32_t beta_1, fp32_t beta_2,
            fp32_t eps, fp32_t lr, fp32_t weight_decay,
            Handle grad, Handle first_moment, Handle second_moment, Handle p);

template
void submit<fp64_t>(Index num_iter, Index num_elems, fp64_t beta_1, fp64_t beta_2,
            fp64_t eps, fp64_t lr, fp64_t weight_decay,
            Handle grad, Handle first_moment, Handle second_moment, Handle p);

} // namespace adamw_step
} // namespace starpu
} // namespace nntile

