#pragma once

#include <nntile/tile/tile.hh>

namespace nntile
{

template<typename T>
void randn_async(const TileTraits &src, const Tile<T> &dst,
        const std::vector<size_t> &dst_coord, unsigned long long seed,
        T mean=0, T stddev=1);

extern template
void randn_async(const TileTraits &src, const Tile<float> &dst,
        const std::vector<size_t> &dst_coord, unsigned long long seed,
        float mean=0, float stddev=1);

extern template
void randn_async(const TileTraits &src, const Tile<double> &dst,
        const std::vector<size_t> &dst_coord, unsigned long long seed,
        double mean=0, double stddev=1);

template<typename T>
void randn_async(const Tile<T> &A, unsigned long long seed, T mean=0,
        T stddev=1)
{
    randn_async<T>(A, A, std::vector<size_t>(A.ndim, 0), seed, mean, stddev);
}

template<typename T>
void randn(const TileTraits &src, const Tile<T> &dst,
        const std::vector<size_t> &dst_coord, unsigned long long seed,
        T mean=0, T stddev=1)
{
    randn_async<T>(src, dst, dst_coord, seed, mean, stddev);
    starpu_task_wait_for_all();
}

template<typename T>
void randn(const Tile<T> &A, unsigned long long seed, T mean=0, T stddev=1)
{
    randn_async<T>(A, A, std::vector<size_t>(A.ndim, 0), seed, mean, stddev);
    starpu_task_wait_for_all();
}

} // namespace nntile

