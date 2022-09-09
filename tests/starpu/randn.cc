/*! @randnright (c) 2022-2022 Skolkovo Institute of Science and Technology
 *                           (Skoltech). All rights reserved.
 *
 * NNTile is software framework for fast training of big neural networks on
 * distributed-memory heterogeneous systems based on StarPU runtime system.
 *
 * @file tests/starpu/randn.cc
 * Smart randn StarPU buffer
 *
 * @version 1.0.0
 * @author Aleksandr Mikhalev
 * @date 2022-09-06
 * */

#include "nntile/starpu/randn.hh"
#include "nntile/kernel/randn/cpu.hh"
#include "common.hh"
#include <array>
#include <vector>
#include <stdexcept>
#include <iostream>

using namespace nntile;

template<typename T, std::size_t NDIM>
void validate_cpu(std::array<Index, NDIM> start, std::array<Index, NDIM> shape,
        std::array<Index, NDIM> underlying_shape)
{
    // Randn related constants
    T mean = 2, stddev = 4;
    unsigned long long seed = -1;
    // Strides and number of elements
    Index nelems = shape[0];
    std::vector<Index> stride(NDIM);
    stride[0] = 2; // Custom stride
    Index size = stride[0]*(shape[0]-1) + 1;
    for(Index i = 1; i < NDIM; ++i)
    {
        stride[i] = stride[i-1]*shape[i-1] + 1; // Custom stride
        size += stride[i] * (shape[i]-1);
        nelems *= shape[i];
    }
    // Init all the data
    std::vector<T> data(size);
    for(Index i = 0; i < size; ++i)
    {
        data[i] = T(i+1);
    }
    // Create copies of data
    std::vector<T> data2(data);
    // Launch low-level kernel
    std::vector<Index> tmp_index(NDIM);
    std::cout << "Run kernel::randn::cpu<T>\n";
    kernel::randn::cpu<T>(NDIM, nelems, seed, mean, stddev, &start[0],
            &shape[0], &underlying_shape[0], &data[0], &stride[0],
            &tmp_index[0]);
    // Check by actually submitting a task
    StarpuVariableHandle data2_handle(&data2[0], sizeof(T)*size),
        tmp_handle(&tmp_index[0], sizeof(Index)*NDIM);
    std::vector<Index> start_(start.cbegin(), start.cend()),
        shape_(shape.cbegin(), shape.cend()),
        underlying_shape_(underlying_shape.cbegin(), underlying_shape.cend());
    starpu::randn::restrict_where(STARPU_CPU);
    std::cout << "Run starpu::randn::submit<T> restricted to CPU\n";
    starpu::randn::submit<T>(NDIM, nelems, seed, mean, stddev, start_, shape_,
            stride, underlying_shape_, data2_handle, tmp_handle);
    starpu_task_wait_for_all();
    data2_handle.unregister();
    // Check result
    for(Index i = 0; i < size; ++i)
    {
        if(data[i] != data2[i])
        {
            throw std::runtime_error("StarPU submission wrong result");
        }
    }
    std::cout << "OK: starpu::randn::submit<T> restricted to CPU\n";
}

// Run multiple tests for a given precision
template<typename T>
void validate_many()
{
    validate_cpu<T, 1>({0}, {1}, {2});
    validate_cpu<T, 1>({2}, {1}, {4});
    validate_cpu<T, 1>({0}, {2}, {2});
    validate_cpu<T, 3>({0, 0, 0}, {1, 2, 4}, {2, 3, 4});
    validate_cpu<T, 3>({1, 0, 0}, {1, 3, 4}, {2, 3, 4});
    validate_cpu<T, 3>({1, 0, 0}, {1, 2, 2}, {2, 3, 4});
    validate_cpu<T, 3>({0, 1, 2}, {2, 2, 2}, {2, 3, 4});
}

int main(int argc, char **argv)
{
    // Init StarPU for testing
    StarpuTest starpu;
    // Init codelet
    starpu::randn::init();
    // Launch all tests
    validate_many<fp32_t>();
    validate_many<fp64_t>();
    return 0;
}
