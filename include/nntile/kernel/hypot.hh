/*! @copyright (c) 2022-2023 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file include/nntile/kernel/hypot.hh
 * hypot low-level kernel
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @date 2023-07-03
 * */

#pragma once

#include <nntile/kernel/hypot/cpu.hh>
#include <nntile/defs.h>
#ifdef NNTILE_USE_CUDA
#include <nntile/kernel/hypot/cuda.hh>
#endif // NNTILE_USE_CUDA

namespace nntile
{
namespace kernel
{
//! @namespace nntile::kernel::hypot
/*! Low-level implementations of hypot operation
 * */
namespace hypot
{

} // namespace hypot
} // namespace kernel
} // namespace nntile

