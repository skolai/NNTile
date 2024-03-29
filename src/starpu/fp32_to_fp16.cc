/*! @copyright (c) 2022-2023 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file src/starpu/fp32_to_fp16.cc
 * Convert fp32_t array into fp16_t array
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @date 2023-05-09
 * */

#include "nntile/starpu/fp32_to_fp16.hh"
#include "nntile/kernel/fp32_to_fp16.hh"

namespace nntile
{
namespace starpu
{
namespace fp32_to_fp16
{

#ifdef NNTILE_USE_CUDA
//! StarPU wrapper for kernel::fp32_to_fp16::cpu<T>
void cpu(void *buffers[], void *cl_args)
    noexcept
{
    // Get arguments
    Index nelems = reinterpret_cast<Index *>(cl_args)[0];
    // Get interfaces
    auto interfaces = reinterpret_cast<VariableInterface **>(buffers);
    const fp32_t *src = interfaces[0]->get_ptr<fp32_t>();
    fp16_t *dst = interfaces[1]->get_ptr<fp16_t>();
    // Launch kernel
    kernel::fp32_to_fp16::cpu(nelems, src, dst);
}

//! StarPU wrapper for kernel::fp32_to_fp16::cuda<T>
void cuda(void *buffers[], void *cl_args)
    noexcept
{
    // Get arguments
    Index nelems = reinterpret_cast<Index *>(cl_args)[0];
    // Get interfaces
    auto interfaces = reinterpret_cast<VariableInterface **>(buffers);
    const fp32_t *src = interfaces[0]->get_ptr<fp32_t>();
    fp16_t *dst = interfaces[1]->get_ptr<fp16_t>();
    // Get CUDA stream
    cudaStream_t stream = starpu_cuda_get_local_stream();
    // Launch kernel
    kernel::fp32_to_fp16::cuda(stream, nelems, src, dst);
}
#endif // NNTILE_USE_CUDA

Codelet codelet;

void init()
{
    codelet.init("nntile_fp32_to_fp16",
            nullptr,
#ifdef NNTILE_USE_CUDA
            {cpu},
            {cuda}
#else // NNTILE_USE_CUDA
            {},
            {}
#endif // NNTILE_USE_CUDA
            );
}

void restrict_where(uint32_t where)
{
    codelet.restrict_where(where);
}

void restore_where()
{
    codelet.restore_where();
}

void submit(Index nelems, Handle src, Handle dst)
{
    Index *nelems_ = new Index{nelems};
    //fp64_t nflops = 5 * nelems;
    int ret = starpu_task_insert(&codelet,
            STARPU_R, static_cast<starpu_data_handle_t>(src),
            STARPU_W, static_cast<starpu_data_handle_t>(dst),
            STARPU_CL_ARGS, nelems_, sizeof(*nelems_),
            //STARPU_FLOPS, nflops,
            0);
    // Check submission
    if(ret != 0)
    {
        throw std::runtime_error("Error in fp32_to_fp16 task submission");
    }
}

} // namespace fp32_to_fp16
} // namespace starpu
} // namespace nntile

