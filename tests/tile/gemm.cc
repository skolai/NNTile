#include "nntile/tile/gemm.hh"
#include "nntile/tile/randn.hh"
#include "nntile/tile/copy.hh"
#include "../testing.hh"
#include <limits>
#include <cmath>
#include <iostream>

using namespace nntile;

template<typename T>
void gemm_naive(TransOp transA, TransOp transB, int M, int N, int K,
        T alpha, const T *A, int ldA, const T *B, int ldB, T beta,
        T *C, int ldC)
{
    for(int m = 0; m < M; ++m)
    {
        for(int n = 0; n < N; ++n)
        {
            T res = 0;
            for(int k = 0; k < K; ++k)
            {
                size_t A_offset, B_offset;
                switch(transA.value)
                {
                    case TransOp::NoTrans:
                        A_offset = k*ldA + m;
                        break;
                    default:
                        A_offset = m*ldA + k;
                }
                switch(transB.value)
                {
                    case TransOp::NoTrans:
                        B_offset = n*ldB + k;
                        break;
                    default:
                        B_offset = k*ldB + n;
                }
                res += A[A_offset] * B[B_offset];
            }
            size_t C_offset = n*ldC + m;
            if(beta == 0)
            {
                C[C_offset] = alpha * res;
            }
            else
            {
                C[C_offset] = beta*C[C_offset] + alpha*res;
            }
        }
    }
}

template<typename T>
T norm(int M, int N, const T *C, int ldC)
{
    T scale = 1, ssq = 0;
    for(int m = 0; m < M; ++m)
    {
        for(int n = 0; n < N; ++n)
        {
            size_t C_offset = n*ldC + m;
            T val = C[C_offset];
            if(val > scale)
            {
                T tmp = scale / val;
                scale = val;
                ssq *= tmp * tmp;
                ssq += 1;
            }
            else
            {
                T tmp = val / scale;
                ssq += tmp * tmp;
            }
        }
    }
    return std::sqrt(ssq) * scale;
}

template<typename T>
void test_gemm(TransOp transA, TransOp transB, int M, int N, int K,
        T alpha, const T *A, int ldA, const T *B, int ldB, T beta,
        const T *C, int ldC, T *D, int ldD)
{
    // D = alpha*op(A)*op(B) + beta*C
    T full = norm(M, N, D, ldD);
    T one = 1;
    gemm_naive(transA, transB, M, N, K, -alpha, A, ldA, B, ldB, one, D, ldD);
    // D = beta*C
    if(beta != 0)
    {
        for(int m = 0; m < M; ++m)
        {
            for(int n = 0; n < N; ++n)
            {
                size_t C_offset = n*ldC + m;
                size_t D_offset = n*ldD + m;
                D[D_offset] -= beta * C[C_offset];
            }
        }
    }
    // D = 0
    T diff = norm(M, N, D, ldD);
    // 3 is a magic constant to supress growing rounding errors
    T threshold = 3 * full * std::numeric_limits<T>::epsilon();
    if(diff > threshold)
    {
        std::cout << "diff/threshold=" << diff/threshold << "\n";
        throw std::runtime_error("GEMM is incorrect");
    }
}

template<typename T>
void validate_gemm()
{
    // Traits for different tiles to check operations
    TileTraits A1_traits({3, 2, 2, 10}),
               A1T_traits({10, 3, 2, 2}),
               B1_traits({10, 5, 6}),
               B1T_traits({5, 6, 10}),
               C1_traits({3, 2, 2, 5, 6}),
               C1T_traits({5, 6, 3, 2, 2}),
               A2_traits({3, 4, 5}),
               A2T_traits({4, 5, 3}),
               B2_traits({4, 5, 5, 6}),
               B2T_traits({5, 6, 4, 5}),
               C2_traits({3, 5, 6}),
               C2T_traits({5, 6, 3});
    // Sizes of corresponding matrices
    int C1M = 12, C1N = 30, C1K = 10, C2M = 3, C2N = 30, C2K = 20;
    // Allocate memory for tiles
    auto *A1_ptr = new T[A1_traits.nelems];
    auto *B1_ptr = new T[B1_traits.nelems];
    auto *C1_ptr = new T[C1_traits.nelems];
    auto *C1_copy_ptr = new T[C1_traits.nelems];
    auto *A2_ptr = new T[A2_traits.nelems];
    auto *B2_ptr = new T[B2_traits.nelems];
    auto *C2_ptr = new T[C2_traits.nelems];
    auto *C2_copy_ptr = new T[C2_traits.nelems];
    // Construct tiles
    Tile<T> A1(A1_traits, A1_ptr, A1_traits.nelems),
        A1T(A1T_traits, A1_ptr, A1_traits.nelems),
        B1(B1_traits, B1_ptr, B1_traits.nelems),
        B1T(B1T_traits, B1_ptr, B1_traits.nelems),
        C1(C1_traits, C1_ptr, C1_traits.nelems),
        C1T(C1T_traits, C1_ptr, C1_traits.nelems),
        C1_copy(C1_traits, C1_copy_ptr, C1_traits.nelems),
        C1T_copy(C1T_traits, C1_copy_ptr, C1_traits.nelems),
        A2(A2_traits, A2_ptr, A2_traits.nelems),
        A2T(A2T_traits, A2_ptr, A2_traits.nelems),
        B2(B2_traits, B2_ptr, B2_traits.nelems),
        B2T(B2T_traits, B2_ptr, B2_traits.nelems),
        C2(C2_traits, C2_ptr, C2_traits.nelems),
        C2T(C2T_traits, C2_ptr, C2_traits.nelems),
        C2_copy(C2_traits, C2_copy_ptr, C2_traits.nelems),
        C2T_copy(C2T_traits, C2_copy_ptr, C2_traits.nelems);
    // Randomly init
    unsigned long long A1_seed = 100, B1_seed = 101, C1_seed=102,
                  A2_seed = 103, B2_seed = 104, C2_seed = 105;
    randn(A1, A1_seed);
    randn(B1, B1_seed);
    randn(C1, C1_seed);
    randn(A2, A2_seed);
    randn(B2, B2_seed);
    randn(C2, C2_seed);
    // Scalar values
    T one = 1.0, zero = 0.0;
    // Check gemm with alpha=one and beta=zero
    for(size_t i = 0; i < C1.nelems; ++i)
    {
        C1_copy_ptr[i] = C1_ptr[i];
    }
    gemm(one, TransOp::NoTrans, A1, TransOp::NoTrans, B1, zero, C1, 1);
    test_gemm(TransOp::NoTrans, TransOp::NoTrans, C1M, C1N, C1K, one,
            A1_ptr, C1M, B1_ptr, C1K, zero, C1_copy_ptr, C1M, C1_ptr, C1M);
    for(size_t i = 0; i < C1.nelems; ++i)
    {
        C1_copy_ptr[i] = C1_ptr[i];
    }
    gemm(one, TransOp::NoTrans, A1, TransOp::Trans, B1T, zero, C1, 1);
    test_gemm(TransOp::NoTrans, TransOp::Trans, C1M, C1N, C1K, one,
            A1_ptr, C1M, B1_ptr, C1N, zero, C1_copy_ptr, C1M, C1_ptr, C1M);
    for(size_t i = 0; i < C1.nelems; ++i)
    {
        C1_copy_ptr[i] = C1_ptr[i];
    }
    gemm(one, TransOp::Trans, A1T, TransOp::NoTrans, B1, zero, C1, 1);
    test_gemm(TransOp::Trans, TransOp::NoTrans, C1M, C1N, C1K, one,
            A1_ptr, C1K, B1_ptr, C1K, zero, C1_copy_ptr, C1M, C1_ptr, C1M);
    for(size_t i = 0; i < C1.nelems; ++i)
    {
        C1_copy_ptr[i] = C1_ptr[i];
    }
    gemm(one, TransOp::Trans, A1T, TransOp::Trans, B1T, zero, C1, 1);
    test_gemm(TransOp::Trans, TransOp::Trans, C1M, C1N, C1K, one,
            A1_ptr, C1K, B1_ptr, C1N, zero, C1_copy_ptr, C1M, C1_ptr, C1M);
    for(size_t i = 0; i < C1.nelems; ++i)
    {
        C1_copy_ptr[i] = C1_ptr[i];
    }
    gemm(one, TransOp::NoTrans, B1T, TransOp::NoTrans, A1T, zero, C1T, 1);
    test_gemm(TransOp::NoTrans, TransOp::NoTrans, C1N, C1M, C1K, one,
            B1_ptr, C1N, A1_ptr, C1K, zero, C1_copy_ptr, C1N, C1_ptr, C1N);
    for(size_t i = 0; i < C1.nelems; ++i)
    {
        C1_copy_ptr[i] = C1_ptr[i];
    }
    gemm(one, TransOp::NoTrans, B1T, TransOp::Trans, A1, zero, C1T, 1);
    test_gemm(TransOp::NoTrans, TransOp::Trans, C1N, C1M, C1K, one,
            B1_ptr, C1N, A1_ptr, C1M, zero, C1_copy_ptr, C1N, C1_ptr, C1N);
    for(size_t i = 0; i < C1.nelems; ++i)
    {
        C1_copy_ptr[i] = C1_ptr[i];
    }
    gemm(one, TransOp::Trans, B1, TransOp::NoTrans, A1T, zero, C1T, 1);
    test_gemm(TransOp::Trans, TransOp::NoTrans, C1N, C1M, C1K, one,
            B1_ptr, C1K, A1_ptr, C1K, zero, C1_copy_ptr, C1N, C1_ptr, C1N);
    for(size_t i = 0; i < C1.nelems; ++i)
    {
        C1_copy_ptr[i] = C1_ptr[i];
    }
    gemm(one, TransOp::Trans, B1, TransOp::Trans, A1, zero, C1T, 1);
    test_gemm(TransOp::Trans, TransOp::Trans, C1N, C1M, C1K, one,
            B1_ptr, C1K, A1_ptr, C1M, zero, C1_copy_ptr, C1N, C1_ptr, C1N);
    for(size_t i = 0; i < C2.nelems; ++i)
    {
        C2_copy_ptr[i] = C2_ptr[i];
    }
    gemm(one, TransOp::NoTrans, A2, TransOp::NoTrans, B2, zero, C2, 2);
    test_gemm(TransOp::NoTrans, TransOp::NoTrans, C2M, C2N, C2K, one,
            A2_ptr, C2M, B2_ptr, C2K, zero, C2_copy_ptr, C2M, C2_ptr, C2M);
    for(size_t i = 0; i < C2.nelems; ++i)
    {
        C2_copy_ptr[i] = C2_ptr[i];
    }
    gemm(one, TransOp::NoTrans, A2, TransOp::Trans, B2T, zero, C2, 2);
    test_gemm(TransOp::NoTrans, TransOp::Trans, C2M, C2N, C2K, one,
            A2_ptr, C2M, B2_ptr, C2N, zero, C2_copy_ptr, C2M, C2_ptr, C2M);
    for(size_t i = 0; i < C2.nelems; ++i)
    {
        C2_copy_ptr[i] = C2_ptr[i];
    }
    gemm(one, TransOp::Trans, A2T, TransOp::NoTrans, B2, zero, C2, 2);
    test_gemm(TransOp::Trans, TransOp::NoTrans, C2M, C2N, C2K, one,
            A2_ptr, C2K, B2_ptr, C2K, zero, C2_copy_ptr, C2M, C2_ptr, C2M);
    for(size_t i = 0; i < C2.nelems; ++i)
    {
        C2_copy_ptr[i] = C2_ptr[i];
    }
    gemm(one, TransOp::Trans, A2T, TransOp::Trans, B2T, zero, C2, 2);
    test_gemm(TransOp::Trans, TransOp::Trans, C2M, C2N, C2K, one,
            A2_ptr, C2K, B2_ptr, C2N, zero, C2_copy_ptr, C2M, C2_ptr, C2M);
    for(size_t i = 0; i < C2.nelems; ++i)
    {
        C2_copy_ptr[i] = C2_ptr[i];
    }
    gemm(one, TransOp::NoTrans, B2T, TransOp::NoTrans, A2T, zero, C2T, 2);
    test_gemm(TransOp::NoTrans, TransOp::NoTrans, C2N, C2M, C2K, one,
            B2_ptr, C2N, A2_ptr, C2K, zero, C2_copy_ptr, C2N, C2_ptr, C2N);
    for(size_t i = 0; i < C2.nelems; ++i)
    {
        C2_copy_ptr[i] = C2_ptr[i];
    }
    gemm(one, TransOp::NoTrans, B2T, TransOp::Trans, A2, zero, C2T, 2);
    test_gemm(TransOp::NoTrans, TransOp::Trans, C2N, C2M, C2K, one,
            B2_ptr, C2N, A2_ptr, C2M, zero, C2_copy_ptr, C2N, C2_ptr, C2N);
    for(size_t i = 0; i < C2.nelems; ++i)
    {
        C2_copy_ptr[i] = C2_ptr[i];
    }
    gemm(one, TransOp::Trans, B2, TransOp::NoTrans, A2T, zero, C2T, 2);
    test_gemm(TransOp::Trans, TransOp::NoTrans, C2N, C2M, C2K, one,
            B2_ptr, C2K, A2_ptr, C2K, zero, C2_copy_ptr, C2N, C2_ptr, C2N);
    for(size_t i = 0; i < C2.nelems; ++i)
    {
        C2_copy_ptr[i] = C2_ptr[i];
    }
    gemm(one, TransOp::Trans, B2, TransOp::Trans, A2, zero, C2T, 2);
    test_gemm(TransOp::Trans, TransOp::Trans, C2N, C2M, C2K, one,
            B2_ptr, C2K, A2_ptr, C2M, zero, C2_copy_ptr, C2N, C2_ptr, C2N);
    // Check gemm with alpha=one and beta=one
    copy(C1, C1_copy);
    gemm(one, TransOp::NoTrans, A1, TransOp::NoTrans, B1, one, C1, 1);
    test_gemm(TransOp::NoTrans, TransOp::NoTrans, C1M, C1N, C1K, one,
            A1_ptr, C1M, B1_ptr, C1K, one, C1_copy_ptr, C1M, C1_ptr, C1M);
    copy(C1, C1_copy);
    gemm(one, TransOp::NoTrans, A1, TransOp::Trans, B1T, one, C1, 1);
    test_gemm(TransOp::NoTrans, TransOp::Trans, C1M, C1N, C1K, one,
            A1_ptr, C1M, B1_ptr, C1N, one, C1_copy_ptr, C1M, C1_ptr, C1M);
    copy(C1, C1_copy);
    gemm(one, TransOp::Trans, A1T, TransOp::NoTrans, B1, one, C1, 1);
    test_gemm(TransOp::Trans, TransOp::NoTrans, C1M, C1N, C1K, one,
            A1_ptr, C1K, B1_ptr, C1K, one, C1_copy_ptr, C1M, C1_ptr, C1M);
    copy(C1, C1_copy);
    gemm(one, TransOp::Trans, A1T, TransOp::Trans, B1T, one, C1, 1);
    test_gemm(TransOp::Trans, TransOp::Trans, C1M, C1N, C1K, one,
            A1_ptr, C1K, B1_ptr, C1N, one, C1_copy_ptr, C1M, C1_ptr, C1M);
    copy(C1, C1_copy);
    gemm(one, TransOp::NoTrans, B1T, TransOp::NoTrans, A1T, one, C1T, 1);
    test_gemm(TransOp::NoTrans, TransOp::NoTrans, C1N, C1M, C1K, one,
            B1_ptr, C1N, A1_ptr, C1K, one, C1_copy_ptr, C1N, C1_ptr, C1N);
    copy(C1, C1_copy);
    gemm(one, TransOp::NoTrans, B1T, TransOp::Trans, A1, one, C1T, 1);
    test_gemm(TransOp::NoTrans, TransOp::Trans, C1N, C1M, C1K, one,
            B1_ptr, C1N, A1_ptr, C1M, one, C1_copy_ptr, C1N, C1_ptr, C1N);
    copy(C1, C1_copy);
    gemm(one, TransOp::Trans, B1, TransOp::NoTrans, A1T, one, C1T, 1);
    test_gemm(TransOp::Trans, TransOp::NoTrans, C1N, C1M, C1K, one,
            B1_ptr, C1K, A1_ptr, C1K, one, C1_copy_ptr, C1N, C1_ptr, C1N);
    copy(C1, C1_copy);
    gemm(one, TransOp::Trans, B1, TransOp::Trans, A1, one, C1T, 1);
    test_gemm(TransOp::Trans, TransOp::Trans, C1N, C1M, C1K, one,
            B1_ptr, C1K, A1_ptr, C1M, one, C1_copy_ptr, C1N, C1_ptr, C1N);
    copy(C2, C2_copy);
    gemm(one, TransOp::NoTrans, A2, TransOp::NoTrans, B2, one, C2, 2);
    test_gemm(TransOp::NoTrans, TransOp::NoTrans, C2M, C2N, C2K, one,
            A2_ptr, C2M, B2_ptr, C2K, one, C2_copy_ptr, C2M, C2_ptr, C2M);
    copy(C2, C2_copy);
    gemm(one, TransOp::NoTrans, A2, TransOp::Trans, B2T, one, C2, 2);
    test_gemm(TransOp::NoTrans, TransOp::Trans, C2M, C2N, C2K, one,
            A2_ptr, C2M, B2_ptr, C2N, one, C2_copy_ptr, C2M, C2_ptr, C2M);
    copy(C2, C2_copy);
    gemm(one, TransOp::Trans, A2T, TransOp::NoTrans, B2, one, C2, 2);
    test_gemm(TransOp::Trans, TransOp::NoTrans, C2M, C2N, C2K, one,
            A2_ptr, C2K, B2_ptr, C2K, one, C2_copy_ptr, C2M, C2_ptr, C2M);
    copy(C2, C2_copy);
    gemm(one, TransOp::Trans, A2T, TransOp::Trans, B2T, one, C2, 2);
    test_gemm(TransOp::Trans, TransOp::Trans, C2M, C2N, C2K, one,
            A2_ptr, C2K, B2_ptr, C2N, one, C2_copy_ptr, C2M, C2_ptr, C2M);
    copy(C2, C2_copy);
    gemm(one, TransOp::NoTrans, B2T, TransOp::NoTrans, A2T, one, C2T, 2);
    test_gemm(TransOp::NoTrans, TransOp::NoTrans, C2N, C2M, C2K, one,
            B2_ptr, C2N, A2_ptr, C2K, one, C2_copy_ptr, C2N, C2_ptr, C2N);
    copy(C2, C2_copy);
    gemm(one, TransOp::NoTrans, B2T, TransOp::Trans, A2, one, C2T, 2);
    test_gemm(TransOp::NoTrans, TransOp::Trans, C2N, C2M, C2K, one,
            B2_ptr, C2N, A2_ptr, C2M, one, C2_copy_ptr, C2N, C2_ptr, C2N);
    copy(C2, C2_copy);
    gemm(one, TransOp::Trans, B2, TransOp::NoTrans, A2T, one, C2T, 2);
    test_gemm(TransOp::Trans, TransOp::NoTrans, C2N, C2M, C2K, one,
            B2_ptr, C2K, A2_ptr, C2K, one, C2_copy_ptr, C2N, C2_ptr, C2N);
    copy(C2, C2_copy);
    gemm(one, TransOp::Trans, B2, TransOp::Trans, A2, one, C2T, 2);
    test_gemm(TransOp::Trans, TransOp::Trans, C2N, C2M, C2K, one,
            B2_ptr, C2K, A2_ptr, C2M, one, C2_copy_ptr, C2N, C2_ptr, C2N);
    copy(C2, C2_copy);
    gemm(T{2}, TransOp::Trans, B2, TransOp::Trans, A2, T{2}, C2T, 2);
    test_gemm(TransOp::Trans, TransOp::Trans, C2N, C2M, C2K, T{2},
            B2_ptr, C2K, A2_ptr, C2M, T{2}, C2_copy_ptr, C2N, C2_ptr, C2N);
    // Negative tests
    auto fail_trans_val = static_cast<TransOp::Value>(-1);
    auto fail_trans = *reinterpret_cast<TransOp *>(&fail_trans_val);
    TESTN(gemm(one, fail_trans, A1, TransOp::NoTrans, B1, one, C1, 1));
    TESTN(gemm(one, fail_trans, A1, TransOp::Trans, B1T, one, C1, 1));
    TESTN(gemm(one, TransOp::NoTrans, A1, fail_trans, B1, one, C1, 1));
    TESTN(gemm(one, TransOp::Trans, A1T, fail_trans, B1, one, C1, 1));
    TESTN(gemm(one, TransOp::Trans, B2, TransOp::Trans, A2, one, C2T, -1));
    TESTN(gemm(one, TransOp::Trans, B1, TransOp::Trans, A2, one, C2T, 2));
    TESTN(gemm(one, TransOp::Trans, B2, TransOp::Trans, A2, one, C2T, 4));
    TESTN(gemm(one, TransOp::Trans, B1, TransOp::Trans, A1, one, C1T, 4));
    TESTN(gemm(one, TransOp::NoTrans, A1T, TransOp::NoTrans, B1, one, C1, 1));
    TESTN(gemm(one, TransOp::Trans, A1, TransOp::NoTrans, B1, one, C1, 1));
    TESTN(gemm(one, TransOp::NoTrans, A1T, TransOp::Trans, B1T, one, C1, 1));
    TESTN(gemm(one, TransOp::Trans, A1, TransOp::Trans, B1T, one, C1, 1));
    TESTN(gemm(one, TransOp::NoTrans, A1, TransOp::NoTrans, B1, one, C1T, 1));
    TESTN(gemm(one, TransOp::Trans, A1T, TransOp::NoTrans, B1, one, C1T, 1));
    TESTN(gemm(one, TransOp::NoTrans, A1, TransOp::Trans, B1T, one, C1T, 1));
    TESTN(gemm(one, TransOp::Trans, A1T, TransOp::Trans, B1T, one, C1T, 1));
    Tile<T> C3({3, 2, 2, 5, 5});
    TESTN(gemm(one, TransOp::NoTrans, A1, TransOp::NoTrans, B1, one, C3, 1));
    TESTN(gemm(one, TransOp::Trans, A1T, TransOp::NoTrans, B1, one, C3, 1));
    TESTN(gemm(one, TransOp::NoTrans, A1, TransOp::Trans, B1T, one, C3, 1));
    TESTN(gemm(one, TransOp::Trans, A1T, TransOp::Trans, B1T, one, C3, 1));
}

int main(int argc, char **argv)
{
    Starpu starpu;
    validate_gemm<float>();
    validate_gemm<double>();
    return 0;
}

