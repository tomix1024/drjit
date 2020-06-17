/*
    enoki/array_avx512.h -- Packed SIMD array (AVX512 specialization)

    Enoki is a C++ template library that enables transparent vectorization
    of numerical kernels using SIMD instruction sets available on current
    processor architectures.

    Copyright (c) 2020 Wenzel Jakob <wenzel.jakob@epfl.ch>

    All rights reserved. Use of this source code is governed by a BSD-style
    license that can be found in the LICENSE file.
*/

#pragma once

NAMESPACE_BEGIN(enoki)
ENOKI_PACKET_DECLARE(64)

/// Partial overload of StaticArrayImpl using AVX512 intrinsics (single precision)
template <bool IsMask_, typename Derived_> struct alignas(64)
    StaticArrayImpl<float, 16, IsMask_, Derived_>
  : StaticArrayBase<float, 16, IsMask_, Derived_> {
    ENOKI_PACKET_TYPE(float, 16, __m512)

    // -----------------------------------------------------------------------
    //! @{ \name Value constructors
    // -----------------------------------------------------------------------

    ENOKI_INLINE StaticArrayImpl(Value value) : m(_mm512_set1_ps(value)) { }

    ENOKI_INLINE StaticArrayImpl(Value f0,  Value f1,  Value f2,  Value f3,
                                 Value f4,  Value f5,  Value f6,  Value f7,
                                 Value f8,  Value f9,  Value f10, Value f11,
                                 Value f12, Value f13, Value f14, Value f15)
        : m(_mm512_setr_ps(f0, f1, f2, f3, f4, f5, f6, f7, f8,
                           f9, f10, f11, f12, f13, f14, f15)) { }

    //! @}
    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------
    //! @{ \name Type converting constructors
    // -----------------------------------------------------------------------

    // ENOKI_CONVERT(half)
    //     : m(_mm512_cvtph_ps(_mm256_loadu_si256((const __m256i *) a.derived().data()))) { }

    ENOKI_CONVERT(float) : m(a.derived().m) { }

    ENOKI_CONVERT(int32_t) : m(_mm512_cvtepi32_ps(a.derived().m)) { }

    ENOKI_CONVERT(uint32_t) : m(_mm512_cvtepu32_ps(a.derived().m)) { }

    ENOKI_CONVERT(double)
        : m(detail::concat(_mm512_cvtpd_ps(low(a).m),
                           _mm512_cvtpd_ps(high(a).m))) { }

    ENOKI_CONVERT(int64_t)
        : m(detail::concat(_mm512_cvtepi64_ps(low(a).m),
                           _mm512_cvtepi64_ps(high(a).m))) { }

    ENOKI_CONVERT(uint64_t)
        : m(detail::concat(_mm512_cvtepu64_ps(low(a).m),
                           _mm512_cvtepu64_ps(high(a).m))) { }

    //! @}
    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------
    //! @{ \name Reinterpreting constructors, mask converters
    // -----------------------------------------------------------------------

    ENOKI_REINTERPRET(float) : m(a.derived().m) { }

    ENOKI_REINTERPRET(int32_t) : m(_mm512_castsi512_ps(a.derived().m)) { }
    ENOKI_REINTERPRET(uint32_t) : m(_mm512_castsi512_ps(a.derived().m)) { }

    //! @}
    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------
    //! @{ \name Converting from/to half size vectors
    // -----------------------------------------------------------------------

    StaticArrayImpl(const Array1 &a1, const Array2 &a2)
        : m(detail::concat(a1.m, a2.m)) { }

    ENOKI_INLINE Array1 low_()  const { return _mm512_castps512_ps256(m); }
    ENOKI_INLINE Array2 high_() const {
        return _mm512_extractf32x8_ps(m, 1);
    }

    //! @}
    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------
    //! @{ \name Vertical operations
    // -----------------------------------------------------------------------

    ENOKI_INLINE Derived add_(Ref a) const { return _mm512_add_ps(m, a.m); }
    ENOKI_INLINE Derived sub_(Ref a) const { return _mm512_sub_ps(m, a.m); }
    ENOKI_INLINE Derived mul_(Ref a) const { return _mm512_mul_ps(m, a.m); }
    ENOKI_INLINE Derived div_(Ref a) const { return _mm512_div_ps(m, a.m); }

    ENOKI_INLINE Derived neg_() const { return _mm512_sub_ps(_mm512_setzero_ps(), m); }

    template <typename T> ENOKI_INLINE Derived or_(const T &a) const {
        if constexpr (is_mask_v<T>)
            return _mm512_mask_mov_ps(m, a.k, _mm512_set1_ps(memcpy_cast<Value>(int32_t(-1))));
        else
            return _mm512_or_ps(m, a.m);
    }

    template <typename T> ENOKI_INLINE Derived and_(const T &a) const {
        if constexpr (is_mask_v<T>)
            return _mm512_maskz_mov_ps(a.k, m);
        else
            return _mm512_and_ps(m, a.m);
    }

    template <typename T> ENOKI_INLINE Derived andnot_(const T &a) const {
        if constexpr (is_mask_v<T>)
            return _mm512_mask_mov_ps(m, a.k, _mm512_setzero_ps());
        else
            return _mm512_andnot_ps(a.m, m);
    }

    template <typename T> ENOKI_INLINE Derived xor_(const T &a) const {
        if constexpr (is_mask_v<T>) {
            const __m512 c = _mm512_set1_ps(memcpy_cast<Value>(int32_t(-1)));
            return _mm512_mask_xor_ps(m, a.k, m, c);
        } else {
            return _mm512_xor_ps(m, a.m);
        }
    }

    ENOKI_INLINE auto lt_ (Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_ps_mask(m, a.m, _CMP_LT_OQ));  }
    ENOKI_INLINE auto gt_ (Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_ps_mask(m, a.m, _CMP_GT_OQ));  }
    ENOKI_INLINE auto le_ (Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_ps_mask(m, a.m, _CMP_LE_OQ));  }
    ENOKI_INLINE auto ge_ (Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_ps_mask(m, a.m, _CMP_GE_OQ));  }
    ENOKI_INLINE auto eq_ (Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_ps_mask(m, a.m, _CMP_EQ_OQ));  }
    ENOKI_INLINE auto neq_(Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_ps_mask(m, a.m, _CMP_NEQ_UQ)); }

    ENOKI_INLINE Derived abs_() const { return andnot_(Derived(_mm512_set1_ps(-0.f))); }

    ENOKI_INLINE Derived min_(Ref b) const { return _mm512_min_ps(b.m, m); }
    ENOKI_INLINE Derived max_(Ref b) const { return _mm512_max_ps(b.m, m); }
    ENOKI_INLINE Derived sqrt_()     const { return _mm512_sqrt_ps(m); }

    ENOKI_INLINE Derived ceil_()     const { return _mm512_ceil_ps(m);     }
    ENOKI_INLINE Derived floor_()    const { return _mm512_floor_ps(m);    }
    ENOKI_INLINE Derived trunc_() const {
        return _mm512_roundscale_ps(m, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
    }
    ENOKI_INLINE Derived round_() const {
        return _mm512_roundscale_ps(m, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
    }

    #define ENOKI_ROUND_OP(name, flag)                                        \
        template <typename T> ENOKI_INLINE auto name() const {                \
            if constexpr (sizeof(scalar_t<T>) == 4) {                         \
                if constexpr (std::is_signed_v<scalar_t<T>>)                  \
                    return T(_mm512_cvt_roundps_epi32(m, flag));              \
                else                                                          \
                    return T(_mm512_cvt_roundps_epu32(m, flag));              \
            } else {                                                          \
                using A = typename T::Array1;                                 \
                if constexpr (std::is_signed_v<scalar_t<T>>)                  \
                    return T(                                                 \
                        A(_mm512_cvt_roundps_epi64(low(derived()).m, flag)),  \
                        A(_mm512_cvt_roundps_epi64(high(derived()).m, flag)));\
                else                                                          \
                    return T(                                                 \
                        A(_mm512_cvt_roundps_epu64(low(derived()).m, flag)),  \
                        A(_mm512_cvt_roundps_epu64(high(derived()).m, flag)));\
            }                                                                 \
        }

    ENOKI_ROUND_OP(ceil2int_,  _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC)
    ENOKI_ROUND_OP(floor2int_, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC)
    ENOKI_ROUND_OP(trunc2int_, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC)
    ENOKI_ROUND_OP(round2int,  _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)

    #undef ENOKI_ROUND_OP

    ENOKI_INLINE Derived fmadd_   (Ref b, Ref c) const { return _mm512_fmadd_ps   (m, b.m, c.m); }
    ENOKI_INLINE Derived fmsub_   (Ref b, Ref c) const { return _mm512_fmsub_ps   (m, b.m, c.m); }
    ENOKI_INLINE Derived fnmadd_  (Ref b, Ref c) const { return _mm512_fnmadd_ps  (m, b.m, c.m); }
    ENOKI_INLINE Derived fnmsub_  (Ref b, Ref c) const { return _mm512_fnmsub_ps  (m, b.m, c.m); }

    template <typename Mask>
    static ENOKI_INLINE Derived select_(const Mask &m, Ref t, Ref f) {
        return _mm512_mask_blend_ps(m.k, f.m, t.m);
    }

    template <size_t I0,  size_t I1,  size_t I2,  size_t I3,  size_t I4,
              size_t I5,  size_t I6,  size_t I7,  size_t I8,  size_t I9,
              size_t I10, size_t I11, size_t I12, size_t I13, size_t I14,
              size_t I15>
    ENOKI_INLINE Derived shuffle_() const {
        const __m512i idx =
            _mm512_setr_epi32(I0, I1, I2, I3, I4, I5, I6, I7, I8,
                              I9, I10, I11, I12, I13, I14, I15);
        return _mm512_permutexvar_ps(idx, m);
    }

    template <typename Index> ENOKI_INLINE Derived shuffle_(const Index &index) const {
        return _mm512_permutexvar_ps(index.m, m);
    }

    ENOKI_INLINE Derived rcp_() const {
        /* Use best reciprocal approximation available on the current
           hardware and refine */
        __m512 r = _mm512_rcp14_ps(m); /* rel error < 2^-14 */

        /* Refine using one Newton-Raphson iteration */
        __m512 t0 = _mm512_add_ps(r, r),
               t1 = _mm512_mul_ps(r, m);

        r = _mm512_fnmadd_ps(t1, r, t0);

        return _mm512_fixupimm_ps(r, m,
            _mm512_set1_epi32(0x0087A622), 0);
    }

    ENOKI_INLINE Derived rsqrt_() const {
        __m512 r = _mm512_rsqrt14_ps(m); /* rel error < 2^-14 */

        /* Refine using one Newton-Raphson iteration */
        const __m512 c0 = _mm512_set1_ps(0.5f),
                     c1 = _mm512_set1_ps(3.0f);

        __m512 t0 = _mm512_mul_ps(r, c0),
               t1 = _mm512_mul_ps(r, m);

        r = _mm512_mul_ps(_mm512_fnmadd_ps(t1, r, c1), t0);

        return _mm512_fixupimm_ps(r, m,
            _mm512_set1_epi32(0x0383A622), 0);
    }

    ENOKI_INLINE Derived ldexp_(Ref arg) const { return _mm512_scalef_ps(m, arg.m); }

    ENOKI_INLINE std::pair<Derived, Derived> frexp_() const {
        return std::make_pair<Derived, Derived>(
            _mm512_getmant_ps(m, _MM_MANT_NORM_p5_1, _MM_MANT_SIGN_src),
            _mm512_getexp_ps(m));
    }

    // -----------------------------------------------------------------------
    //! @{ \name Horizontal operations
    // -----------------------------------------------------------------------

    ENOKI_INLINE Value hsum_()  const { return hsum(low_() + high_()); }
    ENOKI_INLINE Value hprod_() const { return hprod(low_() * high_()); }
    ENOKI_INLINE Value hmin_()  const { return hmin(min(low_(), high_())); }
    ENOKI_INLINE Value hmax_()  const { return hmax(max(low_(), high_())); }

    //! @}
    // -----------------------------------------------------------------------

    ENOKI_INLINE void store_(void *ptr) const {
        _mm512_store_ps((Value *) ENOKI_ASSUME_ALIGNED(ptr, 64), m);
    }

    ENOKI_INLINE void store_unaligned_(void *ptr) const {
        _mm512_storeu_ps((Value *) ptr, m);
    }

    static ENOKI_INLINE Derived load_(const void *ptr, size_t) {
        return _mm512_load_ps((const Value *) ENOKI_ASSUME_ALIGNED(ptr, 64));
    }

    static ENOKI_INLINE Derived load_unaligned_(const void *ptr, size_t) {
        return _mm512_loadu_ps((const Value *) ptr);
    }

    static ENOKI_INLINE Derived empty_(size_t) { return _mm512_undefined_ps(); }
    static ENOKI_INLINE Derived zero_(size_t) { return _mm512_setzero_ps(); }

    template <bool, typename Index, typename Mask>
    static ENOKI_INLINE Derived gather_(const void *ptr, const Index &index, const Mask &mask) {
        if constexpr (sizeof(scalar_t<Index>) == 4) {
            return _mm512_mask_i32gather_ps(_mm512_setzero_ps(), mask.k, index.m, (const float *) ptr, 4);
        } else {
            return detail::concat(
                _mm512_mask_i64gather_ps(_mm256_setzero_ps(),  low(mask).k,  low(index).m, (const float *) ptr, 4),
                _mm512_mask_i64gather_ps(_mm256_setzero_ps(), high(mask).k, high(index).m, (const float *) ptr, 4));
        }
    }

    template <bool, typename Index, typename Mask>
    ENOKI_INLINE void scatter_(void *ptr, const Index &index, const Mask &mask) const {
        if constexpr (sizeof(scalar_t<Index>) == 4) {
            _mm512_mask_i32scatter_ps(ptr, mask.k, index.m, m, 4);
        } else {
            _mm512_mask_i64scatter_ps(ptr, low(mask).k,   low(index).m,  low(derived()).m,  4);
            _mm512_mask_i64scatter_ps(ptr, high(mask).k, high(index).m, high(derived()).m, 4);
        }
    }

    //! @}
    // -----------------------------------------------------------------------
} ENOKI_MAY_ALIAS;

/// Partial overload of StaticArrayImpl using AVX512 intrinsics (double precision)
template <bool IsMask_, typename Derived_> struct alignas(64)
    StaticArrayImpl<double, 8, IsMask_, Derived_>
  : StaticArrayBase<double, 8, IsMask_, Derived_> {
    ENOKI_PACKET_TYPE(double, 8, __m512d)

    // -----------------------------------------------------------------------
    //! @{ \name Value constructors
    // -----------------------------------------------------------------------

    ENOKI_INLINE StaticArrayImpl(const Value &value) : m(_mm512_set1_pd(value)) { }
    ENOKI_INLINE StaticArrayImpl(Value f0, Value f1, Value f2, Value f3,
                                 Value f4, Value f5, Value f6, Value f7)
        : m(_mm512_setr_pd(f0, f1, f2, f3, f4, f5, f6, f7)) { }


    // -----------------------------------------------------------------------
    //! @{ \name Type converting constructors
    // -----------------------------------------------------------------------

    // ENOKI_CONVERT(half)
    //     : m(_mm512_cvtps_pd(
    //           _mm256_cvtph_ps(_mm_loadu_si128((const __m128i *) a.derived().data())))) { }

    ENOKI_CONVERT(float) : m(_mm512_cvtps_pd(a.derived().m)) { }

    ENOKI_CONVERT(double) : m(a.derived().m) { }

    ENOKI_CONVERT(int32_t) : m(_mm512_cvtepi32_pd(a.derived().m)) { }

    ENOKI_CONVERT(uint32_t) : m(_mm512_cvtepu32_pd(a.derived().m)) { }

    ENOKI_CONVERT(int64_t)
        : m(_mm512_cvtepi64_pd(a.derived().m)) { }

    ENOKI_CONVERT(uint64_t)
        : m(_mm512_cvtepu64_pd(a.derived().m)) { }

    //! @}
    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------
    //! @{ \name Reinterpreting constructors, mask converters
    // -----------------------------------------------------------------------

    ENOKI_REINTERPRET(double) : m(a.derived().m) { }

    ENOKI_REINTERPRET(int64_t) : m(_mm512_castsi512_pd(a.derived().m)) { }
    ENOKI_REINTERPRET(uint64_t) : m(_mm512_castsi512_pd(a.derived().m)) { }

    //! @}
    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------
    //! @{ \name Converting from/to half size vectors
    // -----------------------------------------------------------------------

    StaticArrayImpl(const Array1 &a1, const Array2 &a2)
        : m(detail::concat(a1.m, a2.m)) { }

    ENOKI_INLINE Array1 low_()  const { return _mm512_castpd512_pd256(m); }
    ENOKI_INLINE Array2 high_() const { return _mm512_extractf64x4_pd(m, 1); }

    //! @}
    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------
    //! @{ \name Vertical operations
    // -----------------------------------------------------------------------

    ENOKI_INLINE Derived add_(Ref a) const { return _mm512_add_pd(m, a.m); }
    ENOKI_INLINE Derived sub_(Ref a) const { return _mm512_sub_pd(m, a.m); }
    ENOKI_INLINE Derived mul_(Ref a) const { return _mm512_mul_pd(m, a.m); }
    ENOKI_INLINE Derived div_(Ref a) const { return _mm512_div_pd(m, a.m); }

    ENOKI_INLINE Derived neg_() const { return _mm512_sub_pd(_mm512_setzero_pd(), m); }

    template <typename T> ENOKI_INLINE Derived or_(const T &a) const {
        if constexpr (is_mask_v<T>)
            return _mm512_mask_mov_pd(m, a.k, _mm512_set1_pd(memcpy_cast<Value>(int64_t(-1))));
        else
            return _mm512_or_pd(m, a.m);
    }

    template <typename T> ENOKI_INLINE Derived and_(const T &a) const {
        if constexpr (is_mask_v<T>)
            return _mm512_maskz_mov_pd(a.k, m);
        else
            return _mm512_and_pd(m, a.m);
    }

    template <typename T> ENOKI_INLINE Derived andnot_(const T &a) const {
        if constexpr (is_mask_v<T>)
            return _mm512_mask_mov_pd(m, a.k, _mm512_setzero_pd());
        else
            return _mm512_andnot_pd(a.m, m);
    }

    template <typename T> ENOKI_INLINE Derived xor_(const T &a) const {
        if constexpr (is_mask_v<T>) {
            const __m512d c = _mm512_set1_pd(memcpy_cast<Value>(int64_t(-1)));
            return _mm512_mask_xor_pd(m, a.k, m, c);
        } else {
            return _mm512_xor_pd(m, a.m);
        }
    }

    ENOKI_INLINE auto lt_ (Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_pd_mask(m, a.m, _CMP_LT_OQ));  }
    ENOKI_INLINE auto gt_ (Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_pd_mask(m, a.m, _CMP_GT_OQ));  }
    ENOKI_INLINE auto le_ (Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_pd_mask(m, a.m, _CMP_LE_OQ));  }
    ENOKI_INLINE auto ge_ (Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_pd_mask(m, a.m, _CMP_GE_OQ));  }
    ENOKI_INLINE auto eq_ (Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_pd_mask(m, a.m, _CMP_EQ_OQ));  }
    ENOKI_INLINE auto neq_(Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_pd_mask(m, a.m, _CMP_NEQ_UQ)); }

    ENOKI_INLINE Derived abs_() const { return andnot_(Derived(_mm512_set1_pd(-0.0))); }

    ENOKI_INLINE Derived min_(Ref b) const { return _mm512_min_pd(b.m, m); }
    ENOKI_INLINE Derived max_(Ref b) const { return _mm512_max_pd(b.m, m); }
    ENOKI_INLINE Derived sqrt_()     const { return _mm512_sqrt_pd(m); }

    ENOKI_INLINE Derived ceil_()     const { return _mm512_ceil_pd(m);     }
    ENOKI_INLINE Derived floor_()    const { return _mm512_floor_pd(m);    }
    ENOKI_INLINE Derived trunc_() const {
        return _mm512_roundscale_pd(m, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
    }
    ENOKI_INLINE Derived round_() const {
        return _mm512_roundscale_pd(m, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
    }

    #define ENOKI_ROUND_OP(name, flag)                                        \
        template <typename T> ENOKI_INLINE T name() const {                   \
            if constexpr (sizeof(scalar_t<T>) == 4) {                         \
                if constexpr (std::is_signed_v<scalar_t<T>>)                  \
                    return _mm512_cvt_roundpd_epi32(m, flag);                 \
                else                                                          \
                    return _mm512_cvt_roundpd_epu32(m, flag);                 \
            } else {                                                          \
                if constexpr (std::is_signed_v<scalar_t<T>>)                  \
                    return _mm512_cvt_roundpd_epi64(m, flag);                 \
                else                                                          \
                    return _mm512_cvt_roundpd_epu64(m, flag);                 \
            }                                                                 \
        }

    ENOKI_ROUND_OP(ceil2int_,  _MM_FROUND_TO_POS_INF | _MM_FROUND_NO_EXC)
    ENOKI_ROUND_OP(floor2int_, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC)
    ENOKI_ROUND_OP(trunc2int_, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC)
    ENOKI_ROUND_OP(round2int,  _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC)

    #undef ENOKI_ROUND_OP

    ENOKI_INLINE Derived fmadd_   (Ref b, Ref c) const { return _mm512_fmadd_pd   (m, b.m, c.m); }
    ENOKI_INLINE Derived fmsub_   (Ref b, Ref c) const { return _mm512_fmsub_pd   (m, b.m, c.m); }
    ENOKI_INLINE Derived fnmadd_  (Ref b, Ref c) const { return _mm512_fnmadd_pd  (m, b.m, c.m); }
    ENOKI_INLINE Derived fnmsub_  (Ref b, Ref c) const { return _mm512_fnmsub_pd  (m, b.m, c.m); }

    template <typename Mask>
    static ENOKI_INLINE Derived select_(const Mask &m, Ref t, Ref f) {
        return _mm512_mask_blend_pd(m.k, f.m, t.m);
    }

    template <size_t I0, size_t I1, size_t I2, size_t I3, size_t I4, size_t I5,
              size_t I6, size_t I7>
    ENOKI_INLINE Derived shuffle_() const {
        const __m512i idx =
            _mm512_setr_epi64(I0, I1, I2, I3, I4, I5, I6, I7);
        return _mm512_permutexvar_pd(idx, m);
    }

    template <typename Index> ENOKI_INLINE Derived shuffle_(const Index &index) const {
        return _mm512_permutexvar_pd(index.m, m);
    }

    ENOKI_INLINE Derived rcp_() const {
        __m512d r = _mm512_rcp14_pd(m); // rel error < 2^-14

        // Refine using two Newton-Raphson iterations
        ENOKI_UNROLL for (int i = 0; i < 2; ++i) {
            __m512d t0 = _mm512_add_pd(r, r);
            __m512d t1 = _mm512_mul_pd(r, m);

            r = _mm512_fnmadd_pd(t1, r, t0);
        }

        return _mm512_fixupimm_pd(r, m,
            _mm512_set1_epi32(0x0087A622), 0);
    }

    ENOKI_INLINE Derived rsqrt_() const {
        __m512d r = _mm512_rsqrt14_pd(m); // rel error < 2^-14

        const __m512d c0 = _mm512_set1_pd(0.5),
                      c1 = _mm512_set1_pd(3.0);

        // Refine using two Newton-Raphson iterations
        ENOKI_UNROLL for (int i = 0; i < 2; ++i) {
            __m512d t0 = _mm512_mul_pd(r, c0);
            __m512d t1 = _mm512_mul_pd(r, m);

            r = _mm512_mul_pd(_mm512_fnmadd_pd(t1, r, c1), t0);
        }

        return _mm512_fixupimm_pd(r, m,
            _mm512_set1_epi32(0x0383A622), 0);
    }


    ENOKI_INLINE Derived ldexp_(Ref arg) const { return _mm512_scalef_pd(m, arg.m); }

    ENOKI_INLINE std::pair<Derived, Derived> frexp_() const {
        return std::make_pair<Derived, Derived>(
            _mm512_getmant_pd(m, _MM_MANT_NORM_p5_1, _MM_MANT_SIGN_src),
            _mm512_getexp_pd(m));
    }

    // -----------------------------------------------------------------------
    //! @{ \name Horizontal operations
    // -----------------------------------------------------------------------

    ENOKI_INLINE Value hsum_()  const { return hsum(low_() + high_()); }
    ENOKI_INLINE Value hprod_() const { return hprod(low_() * high_()); }
    ENOKI_INLINE Value hmin_()  const { return hmin(min(low_(), high_())); }
    ENOKI_INLINE Value hmax_()  const { return hmax(max(low_(), high_())); }

    //! @}
    // -----------------------------------------------------------------------

    ENOKI_INLINE void store_(void *ptr) const {
        _mm512_store_pd((Value *) ENOKI_ASSUME_ALIGNED(ptr, 64), m);
    }

    ENOKI_INLINE void store_unaligned_(void *ptr) const {
        _mm512_storeu_pd((Value *) ptr, m);
    }

    static ENOKI_INLINE Derived load_(const void *ptr, size_t) {
        return _mm512_load_pd((const Value *) ENOKI_ASSUME_ALIGNED(ptr, 64));
    }

    static ENOKI_INLINE Derived load_unaligned_(const void *ptr, size_t) {
        return _mm512_loadu_pd((const Value *) ptr);
    }

    static ENOKI_INLINE Derived zero_(size_t) { return _mm512_setzero_pd(); }
    static ENOKI_INLINE Derived empty_(size_t) { return _mm512_undefined_pd(); }

    template <bool, typename Index, typename Mask>
    static ENOKI_INLINE Derived gather_(const void *ptr, const Index &index, const Mask &mask) {
        if constexpr (sizeof(scalar_t<Index>) == 4)
            return _mm512_mask_i32gather_pd(_mm512_setzero_pd(), mask.k, index.m, (const double *) ptr, 8);
        else
            return _mm512_mask_i64gather_pd(_mm512_setzero_pd(), mask.k, index.m, (const double *) ptr, 8);
    }

    template <bool, typename Index, typename Mask>
    ENOKI_INLINE void scatter_(void *ptr, const Index &index, const Mask &mask) const {
        if constexpr (sizeof(scalar_t<Index>) == 4)
            _mm512_mask_i32scatter_pd(ptr, mask.k, index.m, m, 8);
        else
            _mm512_mask_i64scatter_pd(ptr, mask.k, index.m, m, 8);
    }

    //! @}
    // -----------------------------------------------------------------------
} ENOKI_MAY_ALIAS;

/// Partial overload of StaticArrayImpl using AVX512 intrinsics (32 bit integers)
template <typename Value_, bool IsMask_, typename Derived_> struct alignas(64)
    StaticArrayImpl<Value_, 16, IsMask_, Derived_, enable_if_int32_t<Value_>>
  : StaticArrayBase<Value_, 16, IsMask_, Derived_> {

    ENOKI_PACKET_TYPE(Value_, 16, __m512i)

    // -----------------------------------------------------------------------
    //! @{ \name Value constructors
    // -----------------------------------------------------------------------

    ENOKI_INLINE StaticArrayImpl(Value value) : m(_mm512_set1_epi32((int32_t) value)) { }

    ENOKI_INLINE StaticArrayImpl(Value f0,  Value f1,  Value f2,  Value f3,
                                 Value f4,  Value f5,  Value f6,  Value f7,
                                 Value f8,  Value f9,  Value f10, Value f11,
                                 Value f12, Value f13, Value f14, Value f15)
        : m(_mm512_setr_epi32(
              (int32_t) f0,  (int32_t) f1,  (int32_t) f2,  (int32_t) f3,
              (int32_t) f4,  (int32_t) f5,  (int32_t) f6,  (int32_t) f7,
              (int32_t) f8,  (int32_t) f9,  (int32_t) f10, (int32_t) f11,
              (int32_t) f12, (int32_t) f13, (int32_t) f14, (int32_t) f15)) { }

    // -----------------------------------------------------------------------
    //! @{ \name Type converting constructors
    // -----------------------------------------------------------------------

    ENOKI_CONVERT(int32_t) : m(a.derived().m) { }
    ENOKI_CONVERT(uint32_t) : m(a.derived().m) { }

    ENOKI_CONVERT(float) {
        m = std::is_signed_v<Value> ? _mm512_cvttps_epi32(a.derived().m)
                                    : _mm512_cvttps_epu32(a.derived().m);
    }

    ENOKI_CONVERT(double) {
        m = std::is_signed_v<Value>
                ? detail::concat(_mm512_cvttpd_epi32(low(a).m),
                                 _mm512_cvttpd_epi32(high(a).m))
                : detail::concat(_mm512_cvttpd_epu32(low(a).m),
                                 _mm512_cvttpd_epu32(high(a).m));
    }

    ENOKI_CONVERT(int64_t)
        : m(detail::concat(_mm512_cvtepi64_epi32(low(a).m),
                           _mm512_cvtepi64_epi32(high(a).m))) { }

    ENOKI_CONVERT(uint64_t)
        : m(detail::concat(_mm512_cvtepi64_epi32(low(a).m),
                           _mm512_cvtepi64_epi32(high(a).m))) { }

    //! @}
    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------
    //! @{ \name Reinterpreting constructors, mask converters
    // -----------------------------------------------------------------------

    ENOKI_REINTERPRET(float) : m(_mm512_castps_si512(a.derived().m)) { }
    ENOKI_REINTERPRET(int32_t) : m(a.derived().m) { }
    ENOKI_REINTERPRET(uint32_t) : m(a.derived().m) { }

    //! @}
    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------
    //! @{ \name Converting from/to half size vectors
    // -----------------------------------------------------------------------

    StaticArrayImpl(const Array1 &a1, const Array2 &a2)
        : m(detail::concat(a1.m, a2.m)) { }

    ENOKI_INLINE Array1 low_()  const { return _mm512_castsi512_si256(m); }
    ENOKI_INLINE Array2 high_() const { return _mm512_extracti32x8_epi32(m, 1); }

    //! @}
    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------
    //! @{ \name Vertical operations
    // -----------------------------------------------------------------------

    ENOKI_INLINE Derived add_(Ref a) const { return _mm512_add_epi32(m, a.m); }
    ENOKI_INLINE Derived sub_(Ref a) const { return _mm512_sub_epi32(m, a.m); }
    ENOKI_INLINE Derived mul_(Ref a) const { return _mm512_mullo_epi32(m, a.m); }

    ENOKI_INLINE Derived neg_() const { return _mm512_sub_epi32(_mm512_setzero_si512(), m); }
    ENOKI_INLINE Derived not_() const { return _mm512_ternarylogic_epi32(m, m, m, 0b01010101); }

    template <typename T>
    ENOKI_INLINE Derived or_ (const T &a) const {
        if constexpr (is_mask_v<T>)
            return _mm512_mask_mov_epi32(m, a.k, _mm512_set1_epi32(int32_t(-1)));
        else
            return _mm512_or_epi32(m, a.m);
    }

    template <typename T>
    ENOKI_INLINE Derived and_ (const T &a) const {
        if constexpr (is_mask_v<T>)
            return _mm512_maskz_mov_epi32(a.k, m);
        else
            return _mm512_and_epi32(m, a.m);
    }

    template <typename T>
    ENOKI_INLINE Derived andnot_ (const T &a) const {
        if constexpr (is_mask_v<T>)
            return _mm512_mask_mov_epi32(m, a.k, _mm512_setzero_si512());
        else
            return _mm512_andnot_epi32(m, a.m);
    }

    template <typename T>
    ENOKI_INLINE Derived xor_ (const T &a) const {
        if constexpr (is_mask_v<T>)
            return _mm512_mask_xor_epi32(m, a.k, m, _mm512_set1_epi32(int32_t(-1)));
        else
            return _mm512_xor_epi32(m, a.m);
    }

    template <int Imm> ENOKI_INLINE Derived sl_() const {
        return _mm512_slli_epi32(m, Imm);
    }

    template <int Imm> ENOKI_INLINE Derived sr_() const {
        return std::is_signed_v<Value> ? _mm512_srai_epi32(m, Imm)
                                       : _mm512_srli_epi32(m, Imm);
    }

    ENOKI_INLINE Derived sl_(Ref k) const {
        return _mm512_sllv_epi32(m, k.m);
    }

    ENOKI_INLINE Derived sr_(Ref k) const {
        return std::is_signed_v<Value> ? _mm512_srav_epi32(m, k.m)
                                       : _mm512_srlv_epi32(m, k.m);
    }

    ENOKI_INLINE auto lt_ (Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_epi32_mask(m, a.m, _MM_CMPINT_LT));  }
    ENOKI_INLINE auto gt_ (Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_epi32_mask(m, a.m, _MM_CMPINT_GT));  }
    ENOKI_INLINE auto le_ (Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_epi32_mask(m, a.m, _MM_CMPINT_LE));  }
    ENOKI_INLINE auto ge_ (Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_epi32_mask(m, a.m, _MM_CMPINT_GE));  }
    ENOKI_INLINE auto eq_ (Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_epi32_mask(m, a.m, _MM_CMPINT_EQ));  }
    ENOKI_INLINE auto neq_(Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_epi32_mask(m, a.m, _MM_CMPINT_NE)); }

    ENOKI_INLINE Derived min_(Ref a) const {
        return std::is_signed_v<Value> ? _mm512_min_epi32(a.m, m)
                                       : _mm512_min_epu32(a.m, m);
    }

    ENOKI_INLINE Derived max_(Ref a) const {
        return std::is_signed_v<Value> ? _mm512_max_epi32(a.m, m)
                                       : _mm512_max_epu32(a.m, m);
    }

    ENOKI_INLINE Derived abs_() const {
        return std::is_signed_v<Value> ? _mm512_abs_epi32(m) : m;
    }

    template <typename Mask>
    static ENOKI_INLINE Derived select_(const Mask &m, Ref t, Ref f) {
        return _mm512_mask_blend_epi32(m.k, f.m, t.m);
    }

    template <size_t I0,  size_t I1,  size_t I2,  size_t I3,  size_t I4,
              size_t I5,  size_t I6,  size_t I7,  size_t I8,  size_t I9,
              size_t I10, size_t I11, size_t I12, size_t I13, size_t I14,
              size_t I15>
    ENOKI_INLINE Derived shuffle_() const {
        const __m512i idx =
            _mm512_setr_epi32(I0, I1, I2, I3, I4, I5, I6, I7, I8,
                              I9, I10, I11, I12, I13, I14, I15);
        return _mm512_permutexvar_epi32(idx, m);
    }

    template <typename Index> ENOKI_INLINE Derived shuffle_(const Index &index) const {
        return _mm512_permutexvar_epi32(index.m, m);
    }

    ENOKI_INLINE Derived mulhi_(Ref a) const {
        auto blend = mask_t<Derived>::from_k(0b0101010101010101);
        Derived even, odd;

        if constexpr (std::is_signed_v<Value>) {
            even.m = _mm512_srli_epi64(_mm512_mul_epi32(m, a.m), 32);
            odd.m = _mm512_mul_epi32(_mm512_srli_epi64(m, 32),
                                     _mm512_srli_epi64(a.m, 32));
        } else {
            even.m = _mm512_srli_epi64(_mm512_mul_epu32(m, a.m), 32);
            odd.m = _mm512_mul_epu32(_mm512_srli_epi64(m, 32),
                                     _mm512_srli_epi64(a.m, 32));
        }

        return select(blend, even, odd);
    }

    ENOKI_INLINE Derived lzcnt_() const { return _mm512_lzcnt_epi32(m); }
    ENOKI_INLINE Derived tzcnt_() const { return Value(32) - lzcnt(~derived() & (derived() - Value(1))); }

#if defined(ENOKI_X86_AVX512VPOPCNTDQ)
    ENOKI_INLINE Derived popcnt_() const { return _mm512_popcnt_epi32(m); }
#endif

    // -----------------------------------------------------------------------
    //! @{ \name Horizontal operations
    // -----------------------------------------------------------------------

    ENOKI_INLINE Value hsum_()  const { return hsum(low_() + high_()); }
    ENOKI_INLINE Value hprod_() const { return hprod(low_() * high_()); }
    ENOKI_INLINE Value hmin_()  const { return hmin(min(low_(), high_())); }
    ENOKI_INLINE Value hmax_()  const { return hmax(max(low_(), high_())); }

    //! @}
    // -----------------------------------------------------------------------

    ENOKI_INLINE void store_(void *ptr) const {
        _mm512_store_si512((__m512i *) ENOKI_ASSUME_ALIGNED(ptr, 64), m);
    }

    ENOKI_INLINE void store_unaligned_(void *ptr) const {
        _mm512_storeu_si512((__m512i *) ptr, m);
    }

    static ENOKI_INLINE Derived load_(const void *ptr, size_t) {
        return _mm512_load_si512((const __m512i *) ENOKI_ASSUME_ALIGNED(ptr, 64));
    }

    static ENOKI_INLINE Derived load_unaligned_(const void *ptr, size_t) {
        return _mm512_loadu_si512((const __m512i *) ptr);
    }

    static ENOKI_INLINE Derived zero_(size_t) { return _mm512_setzero_si512(); }
    static ENOKI_INLINE Derived empty_(size_t) { return _mm512_undefined_epi32(); }


    template <bool, typename Index, typename Mask>
    static ENOKI_INLINE Derived gather_(const void *ptr, const Index &index, const Mask &mask) {
        if constexpr (sizeof(scalar_t<Index>) == 4) {
            return _mm512_mask_i32gather_epi32(_mm512_setzero_si512(), mask.k, index.m, (const float *) ptr, 4);
        } else {
            return detail::concat(
                _mm512_mask_i64gather_epi32(_mm256_setzero_si256(),  low(mask).k,  low(index).m, (const float *) ptr, 4),
                _mm512_mask_i64gather_epi32(_mm256_setzero_si256(), high(mask).k, high(index).m, (const float *) ptr, 4));
        }
    }

    template <bool, typename Index, typename Mask>
    ENOKI_INLINE void scatter_(void *ptr, const Index &index, const Mask &mask) const {
        if constexpr (sizeof(scalar_t<Index>) == 4) {
            _mm512_mask_i32scatter_epi32(ptr, mask.k, index.m, m, 4);
        } else {
            _mm512_mask_i64scatter_epi32(ptr, low(mask).k,   low(index).m,  low(derived()).m,  4);
            _mm512_mask_i64scatter_epi32(ptr, high(mask).k, high(index).m, high(derived()).m, 4);
        }
    }

    template <typename Mask>
    ENOKI_INLINE Value extract_(const Mask &mask) const {
        return (Value) _mm_cvtsi128_si32(_mm512_castsi512_si128(_mm512_maskz_compress_epi32(mask.k, m)));
    }

    //! @}
    // -----------------------------------------------------------------------
} ENOKI_MAY_ALIAS;

/// Partial overload of StaticArrayImpl using AVX512 intrinsics (64 bit integers)
template <typename Value_, bool IsMask_, typename Derived_> struct alignas(64)
    StaticArrayImpl<Value_, 8, IsMask_, Derived_, enable_if_int64_t<Value_>>
  : StaticArrayBase<Value_, 8, IsMask_, Derived_> {

    ENOKI_PACKET_TYPE(Value_, 8, __m512i)

    // -----------------------------------------------------------------------
    //! @{ \name Value constructors
    // -----------------------------------------------------------------------

    ENOKI_INLINE StaticArrayImpl(const Value &value) : m(_mm512_set1_epi64((long long) value)) { }
    ENOKI_INLINE StaticArrayImpl(Value f0, Value f1, Value f2, Value f3,
                                 Value f4, Value f5, Value f6, Value f7)
        : m(_mm512_setr_epi64((long long) f0, (long long) f1, (long long) f2,
                              (long long) f3, (long long) f4, (long long) f5,
                              (long long) f6, (long long) f7)) { }

    // -----------------------------------------------------------------------
    //! @{ \name Type converting constructors
    // -----------------------------------------------------------------------

    ENOKI_CONVERT(float) {
        m = std::is_signed_v<Value> ? _mm512_cvttps_epi64(a.derived().m)
                                    : _mm512_cvttps_epu64(a.derived().m);
    }

    ENOKI_CONVERT(int32_t)
        : m(_mm512_cvtepi32_epi64(a.derived().m)) { }

    ENOKI_CONVERT(uint32_t)
        : m(_mm512_cvtepu32_epi64(a.derived().m)) { }

    ENOKI_CONVERT(double) {
        m = std::is_signed_v<Value> ? _mm512_cvttpd_epi64(a.derived().m)
                                    : _mm512_cvttpd_epu64(a.derived().m);
    }

    ENOKI_CONVERT(int64_t) : m(a.derived().m) { }
    ENOKI_CONVERT(uint64_t) : m(a.derived().m) { }

    //! @}
    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------
    //! @{ \name Reinterpreting constructors, mask converters
    // -----------------------------------------------------------------------

    ENOKI_REINTERPRET(double) : m(_mm512_castpd_si512(a.derived().m)) { }
    ENOKI_REINTERPRET(int64_t) : m(a.derived().m) { }
    ENOKI_REINTERPRET(uint64_t) : m(a.derived().m) { }

    //! @}
    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------
    //! @{ \name Converting from/to half size vectors
    // -----------------------------------------------------------------------

    StaticArrayImpl(const Array1 &a1, const Array2 &a2)
        : m(detail::concat(a1.m, a2.m)) { }

    ENOKI_INLINE Array1 low_()  const { return _mm512_castsi512_si256(m); }
    ENOKI_INLINE Array2 high_() const { return _mm512_extracti64x4_epi64(m, 1); }

    //! @}
    // -----------------------------------------------------------------------

    // -----------------------------------------------------------------------
    //! @{ \name Vertical operations
    // -----------------------------------------------------------------------

    ENOKI_INLINE Derived add_(Ref a) const { return _mm512_add_epi64(m, a.m); }
    ENOKI_INLINE Derived sub_(Ref a) const { return _mm512_sub_epi64(m, a.m); }
    ENOKI_INLINE Derived mul_(Ref a) const { return _mm512_mullo_epi64(m, a.m); }

    ENOKI_INLINE Derived neg_() const { return _mm512_sub_epi64(_mm512_setzero_si512(), m); }
    ENOKI_INLINE Derived not_() const { return _mm512_ternarylogic_epi64(m, m, m, 0b01010101); }

    template <typename T>
    ENOKI_INLINE Derived or_ (const T &a) const {
        if constexpr (is_mask_v<T>)
            return _mm512_mask_mov_epi64(m, a.k, _mm512_set1_epi64(int64_t(-1)));
        else
            return _mm512_or_epi64(m, a.m);
    }

    template <typename T>
    ENOKI_INLINE Derived and_ (const T &a) const {
        if constexpr (is_mask_v<T>)
            return _mm512_maskz_mov_epi64(a.k, m);
        else
            return _mm512_and_epi64(m, a.m);
    }

    template <typename T>
    ENOKI_INLINE Derived andnot_ (const T &a) const {
        if constexpr (is_mask_v<T>)
            return _mm512_mask_mov_epi64(m, a.k, _mm512_setzero_si512());
        else
            return _mm512_andnot_epi64(m, a.m);
    }

    template <typename T>
    ENOKI_INLINE Derived xor_ (const T &a) const {
        if constexpr (is_mask_v<T>)
            return _mm512_mask_xor_epi64(m, a.k, m, _mm512_set1_epi64(int64_t(-1)));
        else
            return _mm512_xor_epi64(m, a.m);
    }

    template <int Imm> ENOKI_INLINE Derived sl_() const {
        return _mm512_slli_epi64(m, Imm);
    }

    template <int Imm> ENOKI_INLINE Derived sr_() const {
        return std::is_signed_v<Value> ? _mm512_srai_epi64(m, Imm)
                                       : _mm512_srli_epi64(m, Imm);
    }

    ENOKI_INLINE Derived sl_(Ref k) const {
        return _mm512_sllv_epi64(m, k.m);
    }

    ENOKI_INLINE Derived sr_(Ref k) const {
        return std::is_signed_v<Value> ? _mm512_srav_epi64(m, k.m)
                                       : _mm512_srlv_epi64(m, k.m);
    }

    ENOKI_INLINE auto lt_ (Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_epi64_mask(m, a.m, _MM_CMPINT_LT)); }
    ENOKI_INLINE auto gt_ (Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_epi64_mask(m, a.m, _MM_CMPINT_GT)); }
    ENOKI_INLINE auto le_ (Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_epi64_mask(m, a.m, _MM_CMPINT_LE)); }
    ENOKI_INLINE auto ge_ (Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_epi64_mask(m, a.m, _MM_CMPINT_GE)); }
    ENOKI_INLINE auto eq_ (Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_epi64_mask(m, a.m, _MM_CMPINT_EQ)); }
    ENOKI_INLINE auto neq_(Ref a) const { return mask_t<Derived>::from_k(_mm512_cmp_epi64_mask(m, a.m, _MM_CMPINT_NE)); }

    ENOKI_INLINE Derived min_(Ref a) const {
        return std::is_signed_v<Value> ? _mm512_min_epi64(a.m, m)
                                       : _mm512_min_epu64(a.m, m);
    }

    ENOKI_INLINE Derived max_(Ref a) const {
        return std::is_signed_v<Value> ? _mm512_max_epi64(a.m, m)
                                       : _mm512_max_epu64(a.m, m);
    }

    ENOKI_INLINE Derived abs_() const {
        return std::is_signed_v<Value> ? _mm512_abs_epi64(m) : m;
    }

    template <typename Mask>
    static ENOKI_INLINE Derived select_(const Mask &m, Ref t, Ref f) {
        return _mm512_mask_blend_epi64(m.k, f.m, t.m);
    }

    template <size_t I0, size_t I1, size_t I2, size_t I3, size_t I4, size_t I5,
              size_t I6, size_t I7>
    ENOKI_INLINE Derived shuffle_() const {
        const __m512i idx =
            _mm512_setr_epi64(I0, I1, I2, I3, I4, I5, I6, I7);
        return _mm512_permutexvar_epi64(idx, m);
    }

    template <typename Index> ENOKI_INLINE Derived shuffle_(const Index &index) const {
        return _mm512_permutexvar_epi64(index.m, m);
    }

    ENOKI_INLINE Derived mulhi_(Ref b) const {
        if (std::is_unsigned_v<Value>) {
            const __m512i low_bits = _mm512_set1_epi64(0xffffffffu);
            __m512i al = m, bl = b.m;
            __m512i ah = _mm512_srli_epi64(al, 32);
            __m512i bh = _mm512_srli_epi64(bl, 32);

            // 4x unsigned 32x32->64 bit multiplication
            __m512i albl = _mm512_mul_epu32(al, bl);
            __m512i albh = _mm512_mul_epu32(al, bh);
            __m512i ahbl = _mm512_mul_epu32(ah, bl);
            __m512i ahbh = _mm512_mul_epu32(ah, bh);

            // Calculate a possible carry from the low bits of the multiplication.
            __m512i carry = _mm512_add_epi64(
                _mm512_srli_epi64(albl, 32),
                _mm512_add_epi64(_mm512_and_epi64(albh, low_bits),
                                 _mm512_and_epi64(ahbl, low_bits)));

            __m512i s0 = _mm512_add_epi64(ahbh, _mm512_srli_epi64(carry, 32));
            __m512i s1 = _mm512_add_epi64(_mm512_srli_epi64(albh, 32),
                                          _mm512_srli_epi64(ahbl, 32));

            return _mm512_add_epi64(s0, s1);
        } else {
            const Derived mask(0xffffffff);
            const Derived a = derived();
            Derived ah = sr<32>(a), bh = sr<32>(b),
                    al = a & mask, bl = b & mask;

            Derived albl_hi = _mm512_srli_epi64(_mm512_mul_epu32(m, b.m), 32);

            Derived t = ah * bl + albl_hi;
            Derived w1 = al * bh + (t & mask);

            return ah * bh + sr<32>(t) + sr<32>(w1);
        }
    }

    ENOKI_INLINE Derived lzcnt_() const { return _mm512_lzcnt_epi64(m); }
    ENOKI_INLINE Derived tzcnt_() const { return Value(64) - lzcnt(~derived() & (derived() - Value(1))); }

#if defined(ENOKI_X86_AVX512VPOPCNTDQ)
    ENOKI_INLINE Derived popcnt_() const { return _mm512_popcnt_epi64(m); }
#endif

    // -----------------------------------------------------------------------
    //! @{ \name Horizontal operations
    // -----------------------------------------------------------------------

    ENOKI_INLINE Value hsum_()  const { return hsum(low_() + high_()); }
    ENOKI_INLINE Value hprod_() const { return hprod(low_() * high_()); }
    ENOKI_INLINE Value hmin_()  const { return hmin(min(low_(), high_())); }
    ENOKI_INLINE Value hmax_()  const { return hmax(max(low_(), high_())); }

    //! @}
    // -----------------------------------------------------------------------

    ENOKI_INLINE void store_(void *ptr) const {
        _mm512_store_si512((__m512i *) ENOKI_ASSUME_ALIGNED(ptr, 64), m);
    }

    ENOKI_INLINE void store_unaligned_(void *ptr) const {
        _mm512_storeu_si512((__m512i *) ptr, m);
    }

    static ENOKI_INLINE Derived load_(const void *ptr, size_t) {
        return _mm512_load_si512((const __m512i *) ENOKI_ASSUME_ALIGNED(ptr, 64));
    }

    static ENOKI_INLINE Derived load_unaligned_(const void *ptr, size_t) {
        return _mm512_loadu_si512((const __m512i *) ptr);
    }

    static ENOKI_INLINE Derived zero_(size_t) { return _mm512_setzero_si512(); }
    static ENOKI_INLINE Derived empty_(size_t) { return _mm512_undefined_epi32(); }

    template <bool, typename Index, typename Mask>
    static ENOKI_INLINE Derived gather_(const void *ptr, const Index &index, const Mask &mask) {
        if constexpr (sizeof(scalar_t<Index>) == 4)
            return _mm512_mask_i32gather_epi64(_mm512_setzero_si512(), mask.k, index.m, (const float *) ptr, 8);
        else
            return _mm512_mask_i64gather_epi64(_mm512_setzero_si512(), mask.k, index.m, (const float *) ptr, 8);
    }


    template <bool, typename Index, typename Mask>
    ENOKI_INLINE void scatter_(void *ptr, const Index &index, const Mask &mask) const {
        if constexpr (sizeof(scalar_t<Index>) == 4)
            _mm512_mask_i32scatter_epi64(ptr, mask.k, index.m, m, 8);
        else
            _mm512_mask_i64scatter_epi64(ptr, mask.k, index.m, m, 8);
    }

    template <typename Mask>
    ENOKI_INLINE Value extract_(const Mask &mask) const {
        return (Value) _mm_cvtsi128_si64(_mm512_castsi512_si128(_mm512_maskz_compress_epi64(mask.k, m)));
    }

    //! @}
    // -----------------------------------------------------------------------

} ENOKI_MAY_ALIAS;

template <typename Derived_>
ENOKI_DECLARE_KMASK(float, 16, Derived_, int)
template <typename Derived_>
ENOKI_DECLARE_KMASK(double, 8, Derived_, int)
template <typename Value_, typename Derived_>
ENOKI_DECLARE_KMASK(Value_, 16, Derived_, enable_if_int32_t<Value_>)
template <typename Value_, typename Derived_>
ENOKI_DECLARE_KMASK(Value_, 8, Derived_, enable_if_int64_t<Value_>)

NAMESPACE_END(enoki)
