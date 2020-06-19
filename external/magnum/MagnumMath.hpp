/*
    Magnum::Math
        — a graphics-focused vector math library

    https://doc.magnum.graphics/magnum/namespaceMagnum_1_1Math.html
    https://doc.magnum.graphics/magnum/namespaceMagnum_1_1EigenIntegration.html
    https://doc.magnum.graphics/magnum/namespaceMagnum_1_1GlmIntegration.html

    This is a single-header library generated from the Magnum project. With the
    goal being easy integration, it's deliberately free of all comments to keep
    the file size small. More info, changelogs and full docs here:

    -   Project homepage — https://magnum.graphics/magnum/
    -   Documentation — https://doc.magnum.graphics/
    -   GitHub project page — https://github.com/mosra/magnum
    -   GitHub Singles repository — https://github.com/mosra/magnum-singles

    v2019.10-0-g8412e8f99 (2019-10-24)
    -   New IsScalar, IsVector, IsIntegral, IsFloatingPoint type traits,
        correct handling of Deg and Rad types in all APIs
    -   Guaranteed NaN handling semantic in min()/max()/minmax() APIs
    -   Using a GCC compiler builtin in sincos()
    -   swizzle() is replaced with gather() and scatter()
    -   Added Matrix::{cofactor,comatrix,adjugate}(), Matrix4::normalMatrix()
    -   New Matrix4::perspectiveProjection() overload taking corner positions
    -   Handling also Eigen::Ref types; EigenIntegration::eigenCast() is now
        just EigenIntegration::cast()
    v2019.01-241-g93686746a (2019-04-03)
    -   Initial release

    Generated from Corrade v2019.10-0-g162d6a7d (2019-10-24),
        Magnum v2019.10-0-g8412e8f99 (2019-10-24) and
        Magnum Integration v2019.10 (2019-10-24), 7499 / 9608 LoC
*/

/*
    This file is part of Magnum.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019 Vladimír Vondruš <mosra@centrum.cz>
    Copyright © 2016 Ashwin Ravichandran <ashwinravichandran24@gmail.com>
    Copyright © 2016, 2018 Jonathan Hale <squareys@googlemail.com>
    Copyright © 2017 sigman78 <sigman78@gmail.com>
    Copyright © 2018 Borislav Stanimirov <b.stanimirov@abv.bg>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include <ciso646>
#ifdef _GLIBCXX_USE_STD_SPEC_FUNCS
#undef _GLIBCXX_USE_STD_SPEC_FUNCS
#define _GLIBCXX_USE_STD_SPEC_FUNCS 0
#endif
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <utility>
#if (!defined(CORRADE_ASSERT) || !defined(CORRADE_CONSTEXPR_ASSERT) || !defined(CORRADE_INTERNAL_ASSERT_OUTPUT) || !defined(CORRADE_ASSERT_UNREACHABLE)) && !defined(NDEBUG)
#include <cassert>
#endif

#if defined(_MSC_VER) && _MSC_VER <= 1920
#define CORRADE_MSVC2017_COMPATIBILITY
#endif
#if defined(_MSC_VER) && _MSC_VER <= 1910
#define CORRADE_MSVC2015_COMPATIBILITY
#endif
#ifdef _WIN32
#define CORRADE_TARGET_WINDOWS
#endif
#ifdef __EMSCRIPTEN__
#define CORRADE_TARGET_EMSCRIPTEN
#endif
#ifdef __ANDROID__
#define CORRADE_TARGET_ANDROID
#endif

#ifndef MAGNUM_EXPORT
#define MAGNUM_EXPORT
#endif

#ifndef Magnum_Types_h
#define Magnum_Types_h

namespace Magnum {

typedef std::uint8_t UnsignedByte;
typedef std::int8_t Byte;
typedef std::uint16_t UnsignedShort;
typedef std::int16_t Short;
typedef std::uint32_t UnsignedInt;
typedef std::int32_t Int;
typedef std::uint64_t UnsignedLong;
typedef std::int64_t Long;

typedef float Float;
typedef double Double;

}

#endif
#ifndef Magnum_Math_Math_h
#define Magnum_Math_Math_h

namespace Magnum { namespace Math {

template<std::size_t> class BoolVector;

template<class> struct Constants;

template<class> class Complex;
template<class> class Dual;
template<class> class DualComplex;
template<class> class DualQuaternion;

template<class> class Frustum;

template<std::size_t, class> class Matrix;
template<class T> using Matrix2x2 = Matrix<2, T>;
template<class T> using Matrix3x3 = Matrix<3, T>;
template<class T> using Matrix4x4 = Matrix<4, T>;

template<class> class Matrix3;
template<class> class Matrix4;

template<class> class Quaternion;

template<std::size_t, std::size_t, class> class RectangularMatrix;
template<class T> using Matrix2x3 = RectangularMatrix<2, 3, T>;
template<class T> using Matrix3x2 = RectangularMatrix<3, 2, T>;
template<class T> using Matrix2x4 = RectangularMatrix<2, 4, T>;
template<class T> using Matrix4x2 = RectangularMatrix<4, 2, T>;
template<class T> using Matrix3x4 = RectangularMatrix<3, 4, T>;
template<class T> using Matrix4x3 = RectangularMatrix<4, 3, T>;

template<template<class> class, class> class Unit;
template<class> class Deg;
template<class> class Rad;

class Half;

template<std::size_t, class> class Vector;
template<class> class Vector2;
template<class> class Vector3;
template<class> class Vector4;

template<class> struct ColorHsv;
template<class> class Color3;
template<class> class Color4;

template<UnsignedInt, UnsignedInt, class> class Bezier;
template<UnsignedInt dimensions, class T> using QuadraticBezier = Bezier<2, dimensions, T>;
template<UnsignedInt dimensions, class T> using CubicBezier = Bezier<3, dimensions, T>;
template<class T> using QuadraticBezier2D = QuadraticBezier<2, T>;
template<class T> using QuadraticBezier3D = QuadraticBezier<3, T>;
template<class T> using CubicBezier2D = CubicBezier<2, T>;
template<class T> using CubicBezier3D = CubicBezier<3, T>;

template<class> class CubicHermite;
template<class T> using CubicHermite1D = CubicHermite<T>;
template<class T> using CubicHermite2D = CubicHermite<Vector2<T>>;
template<class T> using CubicHermite3D = CubicHermite<Vector3<T>>;
template<class T> using CubicHermiteComplex = CubicHermite<Complex<T>>;
template<class T> using CubicHermiteQuaternion = CubicHermite<Quaternion<T>>;

template<UnsignedInt, class> class Range;
template<class T> using Range1D = Range<1, T>;
template<class> class Range2D;
template<class> class Range3D;

namespace Implementation {
    template<class> struct StrictWeakOrdering;
}

}}

#endif
#ifndef MagnumMath_hpp
#define MagnumMath_hpp

namespace Magnum {

typedef Math::Half Half;
typedef Math::Vector2<Float> Vector2;
typedef Math::Vector3<Float> Vector3;
typedef Math::Vector4<Float> Vector4;
typedef Math::Vector2<UnsignedInt> Vector2ui;
typedef Math::Vector3<UnsignedInt> Vector3ui;
typedef Math::Vector4<UnsignedInt> Vector4ui;
typedef Math::Vector2<Int> Vector2i;
typedef Math::Vector3<Int> Vector3i;
typedef Math::Vector4<Int> Vector4i;
typedef Math::Color3<Float> Color3;
typedef Math::Color4<Float> Color4;
typedef Math::Color3<UnsignedByte> Color3ub;
typedef Math::Color4<UnsignedByte> Color4ub;
typedef Math::Matrix3<Float> Matrix3;
typedef Math::Matrix4<Float> Matrix4;
typedef Math::Matrix2x2<Float> Matrix2x2;
typedef Math::Matrix3x3<Float> Matrix3x3;
typedef Math::Matrix4x4<Float> Matrix4x4;
typedef Math::Matrix2x3<Float> Matrix2x3;
typedef Math::Matrix3x2<Float> Matrix3x2;
typedef Math::Matrix2x4<Float> Matrix2x4;
typedef Math::Matrix4x2<Float> Matrix4x2;
typedef Math::Matrix3x4<Float> Matrix3x4;
typedef Math::Matrix4x3<Float> Matrix4x3;
typedef Math::QuadraticBezier2D<Float> QuadraticBezier2D;
typedef Math::QuadraticBezier3D<Float> QuadraticBezier3D;
typedef Math::CubicBezier2D<Float> CubicBezier2D;
typedef Math::CubicBezier3D<Float> CubicBezier3D;
typedef Math::CubicHermite1D<Float> CubicHermite1D;
typedef Math::CubicHermite2D<Float> CubicHermite2D;
typedef Math::CubicHermite3D<Float> CubicHermite3D;
typedef Math::CubicHermiteComplex<Float> CubicHermiteComplex;
typedef Math::CubicHermiteQuaternion<Float> CubicHermiteQuaternion;
typedef Math::Complex<Float> Complex;
typedef Math::DualComplex<Float> DualComplex;
typedef Math::Quaternion<Float> Quaternion;
typedef Math::DualQuaternion<Float> DualQuaternion;
typedef Math::Constants<Float> Constants;
typedef Math::Deg<Float> Deg;
typedef Math::Rad<Float> Rad;
typedef Math::Range1D<Float> Range1D;
typedef Math::Range2D<Float> Range2D;
typedef Math::Range3D<Float> Range3D;
typedef Math::Range1D<Int> Range1Di;
typedef Math::Range2D<Int> Range2Di;
typedef Math::Range3D<Int> Range3Di;
typedef Math::Frustum<Float> Frustum;
typedef Math::Vector2<Double> Vector2d;
typedef Math::Vector3<Double> Vector3d;
typedef Math::Vector4<Double> Vector4d;
typedef Math::Matrix3<Double> Matrix3d;
typedef Math::Matrix4<Double> Matrix4d;
typedef Math::Matrix2x2<Double> Matrix2x2d;
typedef Math::Matrix3x3<Double> Matrix3x3d;
typedef Math::Matrix4x4<Double> Matrix4x4d;
typedef Math::Matrix2x3<Double> Matrix2x3d;
typedef Math::Matrix3x2<Double> Matrix3x2d;
typedef Math::Matrix2x4<Double> Matrix2x4d;
typedef Math::Matrix4x2<Double> Matrix4x2d;
typedef Math::Matrix3x4<Double> Matrix3x4d;
typedef Math::Matrix4x3<Double> Matrix4x3d;
typedef Math::QuadraticBezier2D<Float> QuadraticBezier2Dd;
typedef Math::QuadraticBezier3D<Float> QuadraticBezier3Dd;
typedef Math::CubicBezier2D<Float> CubicBezier2Dd;
typedef Math::CubicBezier3D<Float> CubicBezier3Dd;
typedef Math::CubicHermite1D<Double> CubicHermite1Dd;
typedef Math::CubicHermite2D<Double> CubicHermite2Dd;
typedef Math::CubicHermite3D<Double> CubicHermite3Dd;
typedef Math::CubicHermiteComplex<Double> CubicHermiteComplexd;
typedef Math::CubicHermiteQuaternion<Double> CubicHermiteQuaterniond;
typedef Math::Complex<Double> Complexd;
typedef Math::DualComplex<Double> DualComplexd;
typedef Math::Quaternion<Double> Quaterniond;
typedef Math::DualQuaternion<Double> DualQuaterniond;
typedef Math::Constants<Double> Constantsd;
typedef Math::Deg<Double> Degd;
typedef Math::Rad<Double> Radd;
typedef Math::Range1D<Double> Range1Dd;
typedef Math::Range2D<Double> Range2Dd;
typedef Math::Range3D<Double> Range3Dd;
typedef Math::Frustum<Double> Frustumd;

}

#endif
#ifndef Magnum_Math_Constants_h
#define Magnum_Math_Constants_h

namespace Magnum { namespace Math {

template<class> struct Constants;

#ifndef CORRADE_TARGET_EMSCRIPTEN
template<> struct Constants<long double> {
    static constexpr long double pi()   { return 3.14159265358979323846l; }
};
#endif

template<> struct Constants<Double> {
    Constants() = delete;

    static constexpr Double pi()        { return 3.1415926535897932; }
    static constexpr Double piHalf()    { return 1.5707963267948966; }
    static constexpr Double piQuarter() { return 0.7853981633974483; }
    static constexpr Double tau()       { return 6.2831853071795864; }
    static constexpr Double e()         { return 2.7182818284590452; }
    static constexpr Double sqrt2()     { return 1.4142135623730950; }
    static constexpr Double sqrt3()     { return 1.7320508075688773; }
    static constexpr Double sqrtHalf()  { return 0.7071067811865475; }

    static constexpr Double nan()   { return Double(NAN); }
    static constexpr Double inf()   { return HUGE_VAL; }
};
template<> struct Constants<Float> {
    Constants() = delete;

    static constexpr Float pi()         { return 3.141592654f; }
    static constexpr Float piHalf()     { return 1.570796327f; }
    static constexpr Float piQuarter()  { return 0.785398163f; }
    static constexpr Float tau()        { return 6.283185307f; }
    static constexpr Float e()          { return 2.718281828f; }
    static constexpr Float sqrt2()      { return 1.414213562f; }
    static constexpr Float sqrt3()      { return 1.732050808f; }
    static constexpr Float sqrtHalf()   { return 0.707106781f; }

    static constexpr Float nan()    { return NAN; }
    static constexpr Float inf()    { return HUGE_VALF; }
};

}}

#endif
#ifndef Magnum_Math_TypeTraits_h
#define Magnum_Math_TypeTraits_h

#ifndef FLOAT_EQUALITY_PRECISION
#define FLOAT_EQUALITY_PRECISION 1.0e-5f
#endif

#ifndef DOUBLE_EQUALITY_PRECISION
#define DOUBLE_EQUALITY_PRECISION 1.0e-14
#endif

#ifndef CORRADE_TARGET_EMSCRIPTEN
#ifndef LONG_DOUBLE_EQUALITY_PRECISION
#if !defined(_MSC_VER) && (!defined(CORRADE_TARGET_ANDROID) || __LP64__)
#define LONG_DOUBLE_EQUALITY_PRECISION 1.0e-17l
#else
#define LONG_DOUBLE_EQUALITY_PRECISION 1.0e-14
#endif
#endif
#endif

namespace Magnum { namespace Math {

template<class T> struct IsScalar
    : std::false_type
    {};

template<> struct IsScalar<char>: std::true_type {};
template<> struct IsScalar<signed char>: std::true_type {};
template<> struct IsScalar<unsigned char>: std::true_type {};
template<> struct IsScalar<short>: std::true_type {};
template<> struct IsScalar<unsigned short>: std::true_type {};
template<> struct IsScalar<int>: std::true_type {};
template<> struct IsScalar<unsigned int>: std::true_type {};
template<> struct IsScalar<long>: std::true_type {};
template<> struct IsScalar<unsigned long>: std::true_type {};
template<> struct IsScalar<long long>: std::true_type {};
template<> struct IsScalar<unsigned long long>: std::true_type {};
template<> struct IsScalar<float>: std::true_type {};
template<> struct IsScalar<double>: std::true_type {};
#ifndef CORRADE_TARGET_EMSCRIPTEN
template<> struct IsScalar<long double>: std::true_type {};
#endif
template<template<class> class Derived, class T> struct IsScalar<Unit<Derived, T>>: std::true_type {};
template<class T> struct IsScalar<Deg<T>>: std::true_type {};
template<class T> struct IsScalar<Rad<T>>: std::true_type {};

template<class T> struct IsVector
    : std::false_type
    {};

template<std::size_t size, class T> struct IsVector<Vector<size, T>>: std::true_type {};
template<class T> struct IsVector<Vector2<T>>: std::true_type {};
template<class T> struct IsVector<Vector3<T>>: std::true_type {};
template<class T> struct IsVector<Vector4<T>>: std::true_type {};
template<class T> struct IsVector<Color3<T>>: std::true_type {};
template<class T> struct IsVector<Color4<T>>: std::true_type {};

template<class T> struct IsIntegral
    : std::false_type
    {};

template<> struct IsIntegral<char>: std::true_type {};
template<> struct IsIntegral<signed char>: std::true_type {};
template<> struct IsIntegral<unsigned char>: std::true_type {};
template<> struct IsIntegral<short>: std::true_type {};
template<> struct IsIntegral<unsigned short>: std::true_type {};
template<> struct IsIntegral<int>: std::true_type {};
template<> struct IsIntegral<unsigned int>: std::true_type {};
template<> struct IsIntegral<long>: std::true_type {};
template<> struct IsIntegral<unsigned long>: std::true_type {};
template<> struct IsIntegral<long long>: std::true_type {};
template<> struct IsIntegral<unsigned long long>: std::true_type {};
template<std::size_t size, class T> struct IsIntegral<Vector<size, T>>: IsIntegral<T> {};
template<class T> struct IsIntegral<Vector2<T>>: IsIntegral<T> {};
template<class T> struct IsIntegral<Vector3<T>>: IsIntegral<T> {};
template<class T> struct IsIntegral<Vector4<T>>: IsIntegral<T> {};
template<class T> struct IsIntegral<Color3<T>>: IsIntegral<T> {};
template<class T> struct IsIntegral<Color4<T>>: IsIntegral<T> {};

template<class T> struct IsFloatingPoint
    : std::false_type
    {};

template<> struct IsFloatingPoint<Float>: std::true_type {};
template<> struct IsFloatingPoint<Double>: std::true_type {};
#ifndef CORRADE_TARGET_EMSCRIPTEN
template<> struct IsFloatingPoint<long double>: std::true_type {};
#endif
template<std::size_t size, class T> struct IsFloatingPoint<Vector<size, T>>: IsFloatingPoint<T> {};
template<class T> struct IsFloatingPoint<Vector2<T>>: IsFloatingPoint<T> {};
template<class T> struct IsFloatingPoint<Vector3<T>>: IsFloatingPoint<T> {};
template<class T> struct IsFloatingPoint<Vector4<T>>: IsFloatingPoint<T> {};
template<class T> struct IsFloatingPoint<Color3<T>>: IsFloatingPoint<T> {};
template<class T> struct IsFloatingPoint<Color4<T>>: IsFloatingPoint<T> {};
template<template<class> class Derived, class T> struct IsFloatingPoint<Unit<Derived, T>>: IsFloatingPoint<T> {};
template<class T> struct IsFloatingPoint<Deg<T>>: IsFloatingPoint<T> {};
template<class T> struct IsFloatingPoint<Rad<T>>: IsFloatingPoint<T> {};

template<class T> struct IsUnitless
    : std::integral_constant<bool, IsScalar<T>::value || IsVector<T>::value>
    {};
template<template<class> class Derived, class T> struct IsUnitless<Unit<Derived, T>>: std::false_type {};
template<class T> struct IsUnitless<Deg<T>>: std::false_type {};
template<class T> struct IsUnitless<Rad<T>>: std::false_type {};

namespace Implementation {
    template<class T> struct UnderlyingType {
        static_assert(IsScalar<T>::value, "type is not scalar");
        typedef T Type;
    };
    template<template<class> class Derived, class T> struct UnderlyingType<Unit<Derived, T>> {
        typedef T Type;
    };
    template<class T> struct UnderlyingType<Deg<T>> { typedef T Type; };
    template<class T> struct UnderlyingType<Rad<T>> { typedef T Type; };
    template<std::size_t size, class T> struct UnderlyingType<Vector<size, T>> {
        typedef T Type;
    };
    template<class T> struct UnderlyingType<Vector2<T>> { typedef T Type; };
    template<class T> struct UnderlyingType<Vector3<T>> { typedef T Type; };
    template<class T> struct UnderlyingType<Vector4<T>> { typedef T Type; };
    template<class T> struct UnderlyingType<Color3<T>> { typedef T Type; };
    template<class T> struct UnderlyingType<Color4<T>> { typedef T Type; };
    template<std::size_t cols, std::size_t rows, class T> struct UnderlyingType<RectangularMatrix<cols, rows, T>> {
        typedef T Type;
    };
    template<std::size_t size, class T> struct UnderlyingType<Matrix<size, T>> {
        typedef T Type;
    };
    template<class T> struct UnderlyingType<Matrix3<T>> { typedef T Type; };
    template<class T> struct UnderlyingType<Matrix4<T>> { typedef T Type; };
}

template<class T> using UnderlyingTypeOf = typename Implementation::UnderlyingType<T>::Type;

namespace Implementation {
    template<class T> struct TypeTraitsDefault {
        TypeTraitsDefault() = delete;

        constexpr static bool equals(T a, T b) {
            return a == b;
        }

        constexpr static bool equalsZero(T a, T) {
            return !a;
        }
    };
}

template<class T> struct TypeTraits: Implementation::TypeTraitsDefault<T> {

};

template<class T> inline typename std::enable_if<IsScalar<T>::value, bool>::type equal(T a, T b) {
    return TypeTraits<T>::equals(a, b);
}

template<class T> inline typename std::enable_if<IsScalar<T>::value, bool>::type notEqual(T a, T b) {
    return !TypeTraits<T>::equals(a, b);
}

namespace Implementation {
    template<class> struct TypeTraitsName;
    #define _c(type) template<> struct TypeTraitsName<type> { \
        constexpr static const char* name() { return #type; } \
    };
    _c(UnsignedByte)
    _c(Byte)
    _c(UnsignedShort)
    _c(Short)
    _c(UnsignedInt)
    _c(Int)
    #ifndef CORRADE_TARGET_EMSCRIPTEN
    _c(UnsignedLong)
    _c(Long)
    #endif
    _c(Float)
    _c(Double)
    #ifndef CORRADE_TARGET_EMSCRIPTEN
    _c(long double)
    #endif
    #undef _c

    template<class T> struct TypeTraitsIntegral: TypeTraitsDefault<T>, TypeTraitsName<T> {
        constexpr static T epsilon() { return T(1); }
    };
}

template<> struct TypeTraits<UnsignedByte>: Implementation::TypeTraitsIntegral<UnsignedByte> {
    typedef Float FloatingPointType;
};
template<> struct TypeTraits<Byte>: Implementation::TypeTraitsIntegral<Byte> {
    typedef Float FloatingPointType;
};
template<> struct TypeTraits<UnsignedShort>: Implementation::TypeTraitsIntegral<UnsignedShort> {
    typedef Float FloatingPointType;
};
template<> struct TypeTraits<Short>: Implementation::TypeTraitsIntegral<Short> {
    typedef Float FloatingPointType;
};
template<> struct TypeTraits<UnsignedInt>: Implementation::TypeTraitsIntegral<UnsignedInt> {
    typedef Double FloatingPointType;
};
template<> struct TypeTraits<Int>: Implementation::TypeTraitsIntegral<Int> {
    typedef Double FloatingPointType;
};
#ifndef CORRADE_TARGET_EMSCRIPTEN
template<> struct TypeTraits<UnsignedLong>: Implementation::TypeTraitsIntegral<UnsignedLong> {
    typedef long double FloatingPointType;
};
template<> struct TypeTraits<Long>: Implementation::TypeTraitsIntegral<Long> {
    typedef long double FloatingPointType;
};
#endif

namespace Implementation {

template<class T> struct TypeTraitsFloatingPoint: TypeTraitsName<T> {
    TypeTraitsFloatingPoint() = delete;

    static bool equals(T a, T b);
    static bool equalsZero(T a, T epsilon);
};

template<class T> bool TypeTraitsFloatingPoint<T>::equals(const T a, const T b) {
    if(a == b) return true;

    const T absA = std::abs(a);
    const T absB = std::abs(b);
    const T difference = std::abs(a - b);

    if(a == T{} || b == T{} || difference < TypeTraits<T>::epsilon())
        return difference < TypeTraits<T>::epsilon();

    return difference/(absA + absB) < TypeTraits<T>::epsilon();
}

template<class T> bool TypeTraitsFloatingPoint<T>::equalsZero(const T a, const T magnitude) {
    if(a == T(0.0)) return true;

    const T absA = std::abs(a);

    if(absA < TypeTraits<T>::epsilon())
        return absA < TypeTraits<T>::epsilon();

    return absA*T(0.5)/magnitude < TypeTraits<T>::epsilon();
}

}

template<> struct TypeTraits<Float>: Implementation::TypeTraitsFloatingPoint<Float> {
    typedef Float FloatingPointType;

    constexpr static Float epsilon() { return FLOAT_EQUALITY_PRECISION; }
};
template<> struct TypeTraits<Double>: Implementation::TypeTraitsFloatingPoint<Double> {
    typedef Double FloatingPointType;

    constexpr static Double epsilon() { return DOUBLE_EQUALITY_PRECISION; }
};
#ifndef CORRADE_TARGET_EMSCRIPTEN
template<> struct TypeTraits<long double>: Implementation::TypeTraitsFloatingPoint<long double> {
    typedef long double FloatingPointType;

    constexpr static long double epsilon() { return LONG_DOUBLE_EQUALITY_PRECISION; }
};
#endif

namespace Implementation {

template<class T> inline bool isNormalizedSquared(T lengthSquared) {
    return std::abs(lengthSquared - T(1)) < T(2)*TypeTraits<T>::epsilon();
}

}

}}

#endif
#ifndef Corrade_Containers_Tags_h
#define Corrade_Containers_Tags_h

namespace Corrade { namespace Containers {

struct DefaultInitT {
    struct Init{};
    constexpr explicit DefaultInitT(Init) {}
};

struct ValueInitT {
    struct Init{};
    constexpr explicit ValueInitT(Init) {}
};

struct NoInitT {
    struct Init{};
    constexpr explicit NoInitT(Init) {}
};

struct NoCreateT {
    struct Init{};
    constexpr explicit NoCreateT(Init) {}
};

struct DirectInitT {
    struct Init{};
    constexpr explicit DirectInitT(Init) {}
};

struct InPlaceInitT {
    struct Init{};
    constexpr explicit InPlaceInitT(Init) {}
};

constexpr DefaultInitT DefaultInit{DefaultInitT::Init{}};

constexpr ValueInitT ValueInit{ValueInitT::Init{}};

constexpr NoInitT NoInit{NoInitT::Init{}};

constexpr NoCreateT NoCreate{NoCreateT::Init{}};

constexpr DirectInitT DirectInit{DirectInitT::Init{}};

constexpr InPlaceInitT InPlaceInit{InPlaceInitT::Init{}};

}}

#endif
#ifndef Magnum_Math_Tags_h
#define Magnum_Math_Tags_h

namespace Magnum { namespace Math {

typedef Corrade::Containers::NoInitT NoInitT;

struct ZeroInitT {
    struct Init{};
    constexpr explicit ZeroInitT(Init) {}
};

struct IdentityInitT {
    struct Init{};
    constexpr explicit IdentityInitT(Init) {}
};

using Corrade::Containers::NoInit;

constexpr ZeroInitT ZeroInit{ZeroInitT::Init{}};

constexpr IdentityInitT IdentityInit{IdentityInitT::Init{}};

}}

#endif
#ifndef Magnum_Math_Unit_h
#define Magnum_Math_Unit_h

namespace Magnum { namespace Math {

template<template<class> class Derived, class T> class Unit {
    template<template<class> class, class> friend class Unit;

    public:
        typedef T Type;

        constexpr /*implicit*/ Unit() noexcept: _value(T(0)) {}

        constexpr explicit Unit(ZeroInitT) noexcept: _value(T(0)) {}

        explicit Unit(NoInitT) noexcept {}

        constexpr explicit Unit(T value) noexcept: _value(value) {}

        template<class U> constexpr explicit Unit(Unit<Derived, U> value) noexcept: _value(T(value._value)) {}

        constexpr /*implicit*/ Unit(const Unit<Derived, T>& other) noexcept = default;

        constexpr explicit operator T() const { return _value; }

        constexpr bool operator==(Unit<Derived, T> other) const {
            return TypeTraits<T>::equals(_value, other._value);
        }

        constexpr bool operator!=(Unit<Derived, T> other) const {
            return !operator==(other);
        }

        constexpr bool operator<(Unit<Derived, T> other) const {
            return _value < other._value;
        }

        constexpr bool operator>(Unit<Derived, T> other) const {
            return _value > other._value;
        }

        constexpr bool operator<=(Unit<Derived, T> other) const {
            return !operator>(other);
        }

        constexpr bool operator>=(Unit<Derived, T> other) const {
            return !operator<(other);
        }

        constexpr Unit<Derived, T> operator-() const {
            return Unit<Derived, T>(-_value);
        }

        Unit<Derived, T>& operator+=(Unit<Derived, T> other) {
            _value += other._value;
            return *this;
        }

        constexpr Unit<Derived, T> operator+(Unit<Derived, T> other) const {
            return Unit<Derived, T>(_value + other._value);
        }

        Unit<Derived, T>& operator-=(Unit<Derived, T> other) {
            _value -= other._value;
            return *this;
        }

        constexpr Unit<Derived, T> operator-(Unit<Derived, T> other) const {
            return Unit<Derived, T>(_value - other._value);
        }

        Unit<Derived, T>& operator*=(T number) {
            _value *= number;
            return *this;
        }

        constexpr Unit<Derived, T> operator*(T number) const {
            return Unit<Derived, T>(_value*number);
        }

        Unit<Derived, T>& operator/=(T number) {
            _value /= number;
            return *this;
        }

        constexpr Unit<Derived, T> operator/(T number) const {
            return Unit<Derived, T>(_value/number);
        }

        constexpr T operator/(Unit<Derived, T> other) const {
            return _value/other._value;
        }

    private:
        T _value;
};

template<template<class> class Derived, class T> constexpr Unit<Derived, T> operator*(typename std::common_type<T>::type number, const Unit<Derived, T>& value) {
    return value*number;
}

}}

#endif
#ifndef Magnum_Math_Angle_h
#define Magnum_Math_Angle_h

namespace Magnum { namespace Math {

template<class T> class Deg: public Unit<Deg, T> {
    public:
        constexpr /*implicit*/ Deg() noexcept: Unit<Math::Deg, T>{ZeroInit} {}

        constexpr explicit Deg(ZeroInitT) noexcept: Unit<Math::Deg, T>{ZeroInit} {}

        explicit Deg(NoInitT) noexcept: Unit<Math::Deg, T>{NoInit} {}

        constexpr explicit Deg(T value) noexcept: Unit<Math::Deg, T>(value) {}

        template<class U> constexpr explicit Deg(Unit<Math::Deg, U> value) noexcept: Unit<Math::Deg, T>(value) {}

        constexpr /*implicit*/ Deg(Unit<Math::Deg, T> other) noexcept: Unit<Math::Deg, T>(other) {}

        constexpr /*implicit*/ Deg(Unit<Rad, T> value);
};

namespace Literals {

constexpr Deg<Double> operator "" _deg(long double value) { return Deg<Double>(Double(value)); }

constexpr Deg<Float> operator "" _degf(long double value) { return Deg<Float>(Float(value)); }

}

template<class T> class Rad: public Unit<Rad, T> {
    public:
        constexpr /*implicit*/ Rad() noexcept: Unit<Math::Rad, T>{ZeroInit} {}

        constexpr explicit Rad(ZeroInitT) noexcept: Unit<Math::Rad, T>{ZeroInit} {}

        explicit Rad(NoInitT) noexcept: Unit<Math::Rad, T>{NoInit} {}

        constexpr explicit Rad(T value) noexcept: Unit<Math::Rad, T>(value) {}

        template<class U> constexpr explicit Rad(Unit<Math::Rad, U> value) noexcept: Unit<Math::Rad, T>(value) {}

        constexpr /*implicit*/ Rad(Unit<Math::Rad, T> value) noexcept: Unit<Math::Rad, T>(value) {}

        constexpr /*implicit*/ Rad(Unit<Deg, T> value);
};

namespace Literals {

constexpr Rad<Double> operator "" _rad(long double value) { return Rad<Double>(Double(value)); }

constexpr Rad<Float> operator "" _radf(long double value) { return Rad<Float>(Float(value)); }

}

template<class T> constexpr Deg<T>::Deg(Unit<Rad, T> value): Unit<Math::Deg, T>(T(180)*T(value)/Math::Constants<T>::pi()) {}
template<class T> constexpr Rad<T>::Rad(Unit<Deg, T> value): Unit<Math::Rad, T>(T(value)*Math::Constants<T>::pi()/T(180)) {}

}}

namespace Corrade { namespace Utility {

}}

#endif
#ifndef CORRADE_ASSERT
#ifdef NDEBUG
#define CORRADE_ASSERT(condition, message, returnValue) do {} while(0)
#else
#define CORRADE_ASSERT(condition, message, returnValue) assert(condition)
#endif
#endif

#ifndef CORRADE_CONSTEXPR_ASSERT
#ifdef NDEBUG
#define CORRADE_CONSTEXPR_ASSERT(condition, message) static_cast<void>(0)
#else
#define CORRADE_CONSTEXPR_ASSERT(condition, message)                        \
    static_cast<void>((condition) ? 0 : ([&]() {                            \
        assert(!#condition);                                                \
    }(), 0))
#endif
#endif

#ifndef CORRADE_INTERNAL_ASSERT_OUTPUT
#ifdef NDEBUG
#define CORRADE_INTERNAL_ASSERT_OUTPUT(call)                                \
    static_cast<void>(call)
#else
#define CORRADE_INTERNAL_ASSERT_OUTPUT(call) assert(call)
#endif
#endif

#ifndef CORRADE_ASSERT_UNREACHABLE
#ifdef NDEBUG
#ifdef __GNUC__
#define CORRADE_ASSERT_UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
#define CORRADE_ASSERT_UNREACHABLE() __assume(0)
#else
#define CORRADE_ASSERT_UNREACHABLE() std::abort()
#endif
#else
#define CORRADE_ASSERT_UNREACHABLE() assert(!"unreachable code")
#endif
#endif
#ifndef Magnum_Math_BoolVector_h
#define Magnum_Math_BoolVector_h

namespace Magnum { namespace Math {

namespace Implementation {
    template<std::size_t, class> struct BoolVectorConverter;

    template<std::size_t ...> struct Sequence {};

    template<std::size_t N, std::size_t ...sequence> struct GenerateSequence:
        GenerateSequence<N-1, N-1, sequence...> {};

    template<std::size_t ...sequence> struct GenerateSequence<0, sequence...> {
        typedef Sequence<sequence...> Type;
    };

    template<class T> constexpr T repeat(T value, std::size_t) { return value; }
}

template<std::size_t size> class BoolVector {
    static_assert(size != 0, "BoolVector cannot have zero elements");

    public:
        enum: std::size_t {
            Size = size,
            DataSize = (size-1)/8+1
        };

        constexpr /*implicit*/ BoolVector() noexcept: _data{} {}

        constexpr explicit BoolVector(ZeroInitT) noexcept: _data{} {}

        explicit BoolVector(NoInitT) noexcept {}

        template<class ...T, class U = typename std::enable_if<sizeof...(T)+1 == DataSize, bool>::type> constexpr /*implicit*/ BoolVector(UnsignedByte first, T... next) noexcept: _data{first, UnsignedByte(next)...} {}

        template<class T, class U = typename std::enable_if<std::is_same<bool, T>::value && size != 1, bool>::type> constexpr explicit BoolVector(T value) noexcept: BoolVector(typename Implementation::GenerateSequence<DataSize>::Type(), value ? FullSegmentMask : 0) {}

        template<class U, class V = decltype(Implementation::BoolVectorConverter<size, U>::from(std::declval<U>()))> constexpr explicit BoolVector(const U& other) noexcept: BoolVector{Implementation::BoolVectorConverter<size, U>::from(other)} {}

        constexpr /*implicit*/ BoolVector(const BoolVector<size>&) noexcept = default;

        template<class U, class V = decltype(Implementation::BoolVectorConverter<size, U>::to(std::declval<BoolVector<size>>()))> constexpr explicit operator U() const {
            return Implementation::BoolVectorConverter<size, U>::to(*this);
        }

        UnsignedByte* data() { return _data; }
        constexpr const UnsignedByte* data() const { return _data; }

        constexpr bool operator[](std::size_t i) const {
            return (_data[i/8] >> i%8) & 0x01;
        }

        BoolVector<size>& set(std::size_t i, bool value) {
            value ? _data[i/8] |=  (1 << i%8) :
                    _data[i/8] &= ~(1 << i%8);
            return *this;
        }

        bool operator==(const BoolVector<size>& other) const;

        bool operator!=(const BoolVector<size>& other) const {
            return !operator==(other);
        }

        explicit operator bool() const { return all(); }

        bool all() const;

        bool none() const;

        bool any() const { return !none(); }

        BoolVector<size> operator~() const;

        BoolVector<size> operator!() const { return operator~(); }

        BoolVector<size>& operator&=(const BoolVector<size>& other) {
            for(std::size_t i = 0; i != DataSize; ++i)
                _data[i] &= other._data[i];

            return *this;
        }

        BoolVector<size> operator&(const BoolVector<size>& other) const {
            return BoolVector<size>(*this) &= other;
        }

        BoolVector<size> operator&&(const BoolVector<size>& other) const {
            return BoolVector<size>(*this) &= other;
        }

        BoolVector<size>& operator|=(const BoolVector<size>& other) {
            for(std::size_t i = 0; i != DataSize; ++i)
                _data[i] |= other._data[i];

            return *this;
        }

        BoolVector<size> operator|(const BoolVector<size>& other) const {
            return BoolVector<size>(*this) |= other;
        }

        BoolVector<size> operator||(const BoolVector<size>& other) const {
            return BoolVector<size>(*this) |= other;
        }

        BoolVector<size>& operator^=(const BoolVector<size>& other) {
            for(std::size_t i = 0; i != DataSize; ++i)
                _data[i] ^= other._data[i];

            return *this;
        }

        BoolVector<size> operator^(const BoolVector<size>& other) const {
            return BoolVector<size>(*this) ^= other;
        }

    private:
        enum: UnsignedByte {
            FullSegmentMask = 0xFF,
            LastSegmentMask = (1 << size%8) - 1
        };

        template<std::size_t ...sequence> constexpr explicit BoolVector(Implementation::Sequence<sequence...>, UnsignedByte value): _data{Implementation::repeat(value, sequence)...} {}

        UnsignedByte _data[(size-1)/8+1];
};

template<std::size_t size> inline bool BoolVector<size>::operator==(const BoolVector<size>& other) const {
    for(std::size_t i = 0; i != size/8; ++i)
        if(_data[i] != other._data[i]) return false;

    if(size%8 && (_data[DataSize-1] & LastSegmentMask) != (other._data[DataSize-1] & LastSegmentMask))
        return false;

    return true;
}

template<std::size_t size> inline bool BoolVector<size>::all() const {
    for(std::size_t i = 0; i != size/8; ++i)
        if(_data[i] != FullSegmentMask) return false;

    if(size%8 && (_data[DataSize-1] & LastSegmentMask) != LastSegmentMask)
        return false;

    return true;
}

template<std::size_t size> inline bool BoolVector<size>::none() const {
    for(std::size_t i = 0; i != size/8; ++i)
        if(_data[i]) return false;

    if(size%8 && (_data[DataSize-1] & LastSegmentMask))
        return false;

    return true;
}

template<std::size_t size> inline BoolVector<size> BoolVector<size>::operator~() const {
    BoolVector<size> out{NoInit};

    for(std::size_t i = 0; i != DataSize; ++i)
        out._data[i] = ~_data[i];

    return out;
}

namespace Implementation {

template<std::size_t size> struct StrictWeakOrdering<BoolVector<size>> {
    bool operator()(const BoolVector<size>& a, const BoolVector<size>& b) const {
        auto ad = a.data();
        auto bd = b.data();
        for(std::size_t i = 0; i < BoolVector<size>::DataSize - 1; ++i) {
            if(ad[i] < bd[i])
                return true;
            if(ad[i] > bd[i])
                return false;
        }

        constexpr UnsignedByte mask = UnsignedByte(0xFF) >> (BoolVector<size>::DataSize * 8 - size);
        constexpr std::size_t i = BoolVector<size>::DataSize - 1;
        return (ad[i] & mask) < (bd[i] & mask);
    }
};

}

}}

#endif
#ifndef Magnum_Math_Vector_h
#define Magnum_Math_Vector_h

namespace Magnum { namespace Math {

template<class T> inline typename std::enable_if<IsScalar<T>::value, bool>::type isNan(T value) {
    return std::isnan(UnderlyingTypeOf<T>(value));
}
template<class T> constexpr typename std::enable_if<IsScalar<T>::value, T>::type min(T value, T min) {
    return min < value ? min : value;
}
template<class T> constexpr typename std::enable_if<IsScalar<T>::value, T>::type max(T value, T max) {
    return value < max ? max : value;
}

namespace Implementation {
    template<std::size_t, class, class> struct VectorConverter;
    template<class T, class U> T lerp(const T& a, const T& b, U t) {
        return T((U(1) - t)*a + t*b);
    }

    template<bool integral> struct IsZero;
    template<> struct IsZero<false> {
        template<std::size_t size, class T> bool operator()(const Vector<size, T>& vec) const {
            return std::abs(vec.dot()) < TypeTraits<T>::epsilon();
        }
    };
    template<> struct IsZero<true> {
        template<std::size_t size, class T> bool operator()(const Vector<size, T>& vec) const {
            return vec == Vector<size, T>{};
        }
    };

    template<std::size_t, class> struct MatrixDeterminant;
    template<std::size_t, std::size_t> struct GatherComponentAt;
    template<std::size_t, std::size_t, bool> struct ScatterComponentOr;
    template<class T, std::size_t valueSize, char, char...> constexpr T scatterRecursive(const T&, const Vector<valueSize, typename T::Type>&, std::size_t);
}

template<std::size_t size, class T> inline T dot(const Vector<size, T>& a, const Vector<size, T>& b) {
    return (a*b).sum();
}

template<std::size_t size, class FloatingPoint> inline
typename std::enable_if<std::is_floating_point<FloatingPoint>::value, Rad<FloatingPoint>>::type
angle(const Vector<size, FloatingPoint>& normalizedA, const Vector<size, FloatingPoint>& normalizedB) {
    CORRADE_ASSERT(normalizedA.isNormalized() && normalizedB.isNormalized(),
        "Math::angle(): vectors" << normalizedA << "and" << normalizedB << "are not normalized", {});
    return Rad<FloatingPoint>(std::acos(dot(normalizedA, normalizedB)));
}

template<std::size_t size, class T> class Vector {
    static_assert(size != 0, "Vector cannot have zero elements");

    public:
        typedef T Type;

        enum: std::size_t {
            Size = size
        };

        static Vector<size, T>& from(T* data) {
            return *reinterpret_cast<Vector<size, T>*>(data);
        }
        static const Vector<size, T>& from(const T* data) {
            return *reinterpret_cast<const Vector<size, T>*>(data);
        }

        template<std::size_t otherSize> constexpr static Vector<size, T> pad(const Vector<otherSize, T>& a, T value = T(0)) {
            return padInternal<otherSize>(typename Implementation::GenerateSequence<size>::Type(), a, value);
        }

        constexpr /*implicit*/ Vector() noexcept: _data{} {}

        constexpr explicit Vector(ZeroInitT) noexcept: _data{} {}

        explicit Vector(NoInitT) noexcept {}

        template<class ...U, class V = typename std::enable_if<sizeof...(U)+1 == size, T>::type> constexpr /*implicit*/ Vector(T first, U... next) noexcept: _data{first, next...} {}

        template<class U, class V = typename std::enable_if<std::is_same<T, U>::value && size != 1, T>::type> constexpr explicit Vector(U value) noexcept: Vector(typename Implementation::GenerateSequence<size>::Type(), value) {}

        template<class U> constexpr explicit Vector(const Vector<size, U>& other) noexcept: Vector(typename Implementation::GenerateSequence<size>::Type(), other) {}

        template<class U, class V = decltype(Implementation::VectorConverter<size, T, U>::from(std::declval<U>()))> constexpr explicit Vector(const U& other) noexcept: Vector(Implementation::VectorConverter<size, T, U>::from(other)) {}

        constexpr /*implicit*/ Vector(const Vector<size, T>&) noexcept = default;

        template<class U, class V = decltype(Implementation::VectorConverter<size, T, U>::to(std::declval<Vector<size, T>>()))> constexpr explicit operator U() const {
            return Implementation::VectorConverter<size, T, U>::to(*this);
        }

        T* data() { return _data; }
        constexpr const T* data() const { return _data; }

        T& operator[](std::size_t pos) { return _data[pos]; }
        constexpr T operator[](std::size_t pos) const { return _data[pos]; }

        bool operator==(const Vector<size, T>& other) const {
            for(std::size_t i = 0; i != size; ++i)
                if(!TypeTraits<T>::equals(_data[i], other._data[i])) return false;

            return true;
        }

        bool operator!=(const Vector<size, T>& other) const {
            return !operator==(other);
        }

        BoolVector<size> operator<(const Vector<size, T>& other) const;

        BoolVector<size> operator<=(const Vector<size, T>& other) const;

        BoolVector<size> operator>=(const Vector<size, T>& other) const;

        BoolVector<size> operator>(const Vector<size, T>& other) const;

        bool isZero() const {
            return Implementation::IsZero<std::is_integral<T>::value>{}(*this);
        }

        bool isNormalized() const {
            return Implementation::isNormalizedSquared(dot());
        }

        template<class U = T> typename std::enable_if<std::is_signed<U>::value, Vector<size, T>>::type
        operator-() const;

        Vector<size, T>& operator+=(const Vector<size, T>& other) {
            for(std::size_t i = 0; i != size; ++i)
                _data[i] += other._data[i];

            return *this;
        }

        Vector<size, T> operator+(const Vector<size, T>& other) const {
            return Vector<size, T>(*this) += other;
        }

        Vector<size, T>& operator-=(const Vector<size, T>& other) {
            for(std::size_t i = 0; i != size; ++i)
                _data[i] -= other._data[i];

            return *this;
        }

        Vector<size, T> operator-(const Vector<size, T>& other) const {
            return Vector<size, T>(*this) -= other;
        }

        Vector<size, T>& operator*=(T scalar) {
            for(std::size_t i = 0; i != size; ++i)
                _data[i] *= scalar;

            return *this;
        }

        Vector<size, T> operator*(T scalar) const {
            return Vector<size, T>(*this) *= scalar;
        }

        Vector<size, T>& operator/=(T scalar) {
            for(std::size_t i = 0; i != size; ++i)
                _data[i] /= scalar;

            return *this;
        }

        Vector<size, T> operator/(T scalar) const {
            return Vector<size, T>(*this) /= scalar;
        }

        Vector<size, T>& operator*=(const Vector<size, T>& other) {
            for(std::size_t i = 0; i != size; ++i)
                _data[i] *= other._data[i];

            return *this;
        }

        Vector<size, T> operator*(const Vector<size, T>& other) const {
            return Vector<size, T>(*this) *= other;
        }

        Vector<size, T>& operator/=(const Vector<size, T>& other) {
            for(std::size_t i = 0; i != size; ++i)
                _data[i] /= other._data[i];

            return *this;
        }

        Vector<size, T> operator/(const Vector<size, T>& other) const {
            return Vector<size, T>(*this) /= other;
        }

        T dot() const { return Math::dot(*this, *this); }

        T length() const { return std::sqrt(dot()); }

        template<class U = T> typename std::enable_if<std::is_floating_point<U>::value, T>::type
        lengthInverted() const { return T(1)/length(); }

        template<class U = T> typename std::enable_if<std::is_floating_point<U>::value, Vector<size, T>>::type
        normalized() const { return *this*lengthInverted(); }

        template<class U = T> typename std::enable_if<std::is_floating_point<U>::value, Vector<size, T>>::type
        resized(T length) const {
            return *this*(lengthInverted()*length);
        }

        template<class U = T> typename std::enable_if<std::is_floating_point<U>::value, Vector<size, T>>::type
        projected(const Vector<size, T>& line) const {
            return line*Math::dot(*this, line)/line.dot();
        }

        template<class U = T> typename std::enable_if<std::is_floating_point<U>::value, Vector<size, T>>::type
        projectedOntoNormalized(const Vector<size, T>& line) const;

        constexpr Vector<size, T> flipped() const {
            return flippedInternal(typename Implementation::GenerateSequence<size>::Type{});
        }

        T sum() const;

        T product() const;

        T min() const;

        T max() const;

        std::pair<T, T> minmax() const;

    protected:
        T _data[size];

    private:
        template<std::size_t, class> friend class Vector;
        template<std::size_t, std::size_t, class> friend class RectangularMatrix;
        template<std::size_t, class> friend class Matrix;
        template<std::size_t, class> friend struct Implementation::MatrixDeterminant;
        template<std::size_t, std::size_t> friend struct Implementation::GatherComponentAt;
        template<std::size_t, std::size_t, bool> friend struct Implementation::ScatterComponentOr;
        template<class T_, std::size_t valueSize, char, char...> friend constexpr T_ Implementation::scatterRecursive(const T_&, const Vector<valueSize, typename T_::Type>&, std::size_t);

        template<std::size_t size_, class T_> friend BoolVector<size_> equal(const Vector<size_, T_>&, const Vector<size_, T_>&);
        template<std::size_t size_, class T_> friend BoolVector<size_> notEqual(const Vector<size_, T_>&, const Vector<size_, T_>&);

        template<class U, std::size_t ...sequence> constexpr explicit Vector(Implementation::Sequence<sequence...>, const Vector<size, U>& vector) noexcept: _data{T(vector._data[sequence])...} {}

        template<std::size_t ...sequence> constexpr explicit Vector(Implementation::Sequence<sequence...>, T value) noexcept: _data{Implementation::repeat(value, sequence)...} {}

        template<std::size_t otherSize, std::size_t ...sequence> constexpr static Vector<size, T> padInternal(Implementation::Sequence<sequence...>, const Vector<otherSize, T>& a, T value) {
            return {sequence < otherSize ? a[sequence] : value...};
        }

        template<std::size_t ...sequence> constexpr Vector<size, T> flippedInternal(Implementation::Sequence<sequence...>) const {
            return {_data[size - 1 - sequence]...};
        }
};

template<std::size_t size, class T> inline BoolVector<size> equal(const Vector<size, T>& a, const Vector<size, T>& b) {
    BoolVector<size> out;

    for(std::size_t i = 0; i != size; ++i)
        out.set(i, TypeTraits<T>::equals(a._data[i], b._data[i]));

    return out;
}

template<std::size_t size, class T> inline BoolVector<size> notEqual(const Vector<size, T>& a, const Vector<size, T>& b) {
    BoolVector<size> out;

    for(std::size_t i = 0; i != size; ++i)
        out.set(i, !TypeTraits<T>::equals(a._data[i], b._data[i]));

    return out;
}

template<std::size_t size, class T> inline Vector<size, T> operator*(
    typename std::common_type<T>::type
    scalar, const Vector<size, T>& vector)
{
    return vector*scalar;
}

template<std::size_t size, class T> inline Vector<size, T> operator/(
    typename std::common_type<T>::type
    scalar, const Vector<size, T>& vector)
{
    Vector<size, T> out;

    for(std::size_t i = 0; i != size; ++i)
        out[i] = scalar/vector[i];

    return out;
}

template<std::size_t size, class Integral> inline
typename std::enable_if<std::is_integral<Integral>::value, Vector<size, Integral>&>::type
operator%=(Vector<size, Integral>& a, Integral b) {
    for(std::size_t i = 0; i != size; ++i)
        a[i] %= b;

    return a;
}

template<std::size_t size, class Integral> inline
typename std::enable_if<std::is_integral<Integral>::value, Vector<size, Integral>>::type
operator%(const Vector<size, Integral>& a, Integral b) {
    Vector<size, Integral> copy(a);
    return copy %= b;
}

template<std::size_t size, class Integral> inline
typename std::enable_if<std::is_integral<Integral>::value, Vector<size, Integral>&>::type
operator%=(Vector<size, Integral>& a, const Vector<size, Integral>& b) {
    for(std::size_t i = 0; i != size; ++i)
        a[i] %= b[i];

    return a;
}

template<std::size_t size, class Integral> inline
typename std::enable_if<std::is_integral<Integral>::value, Vector<size, Integral>>::type
operator%(const Vector<size, Integral>& a, const Vector<size, Integral>& b) {
    Vector<size, Integral> copy(a);
    return copy %= b;
}

template<std::size_t size, class Integral> inline
typename std::enable_if<std::is_integral<Integral>::value, Vector<size, Integral>>::type
operator~(const Vector<size, Integral>& vector) {
    Vector<size, Integral> out;

    for(std::size_t i = 0; i != size; ++i)
        out[i] = ~vector[i];

    return out;
}

template<std::size_t size, class Integral> inline
typename std::enable_if<std::is_integral<Integral>::value, Vector<size, Integral>&>::type
operator&=(Vector<size, Integral>& a, const Vector<size, Integral>& b) {
    for(std::size_t i = 0; i != size; ++i)
        a[i] &= b[i];

    return a;
}

template<std::size_t size, class Integral> inline
typename std::enable_if<std::is_integral<Integral>::value, Vector<size, Integral>>::type
operator&(const Vector<size, Integral>& a, const Vector<size, Integral>& b) {
    Vector<size, Integral> copy(a);
    return copy &= b;
}

template<std::size_t size, class Integral> inline
typename std::enable_if<std::is_integral<Integral>::value, Vector<size, Integral>&>::type
operator|=(Vector<size, Integral>& a, const Vector<size, Integral>& b) {
    for(std::size_t i = 0; i != size; ++i)
        a[i] |= b[i];

    return a;
}

template<std::size_t size, class Integral> inline
typename std::enable_if<std::is_integral<Integral>::value, Vector<size, Integral>>::type
operator|(const Vector<size, Integral>& a, const Vector<size, Integral>& b) {
    Vector<size, Integral> copy(a);
    return copy |= b;
}

template<std::size_t size, class Integral> inline
typename std::enable_if<std::is_integral<Integral>::value, Vector<size, Integral>&>::type
operator^=(Vector<size, Integral>& a, const Vector<size, Integral>& b) {
    for(std::size_t i = 0; i != size; ++i)
        a[i] ^= b[i];

    return a;
}

template<std::size_t size, class Integral> inline
typename std::enable_if<std::is_integral<Integral>::value, Vector<size, Integral>>::type
operator^(const Vector<size, Integral>& a, const Vector<size, Integral>& b) {
    Vector<size, Integral> copy(a);
    return copy ^= b;
}

template<std::size_t size, class Integral> inline
typename std::enable_if<std::is_integral<Integral>::value, Vector<size, Integral>&>::type
operator<<=(Vector<size, Integral>& vector,
    typename std::common_type<Integral>::type
    shift)
{
    for(std::size_t i = 0; i != size; ++i)
        vector[i] <<= shift;

    return vector;
}

template<std::size_t size, class Integral> inline
typename std::enable_if<std::is_integral<Integral>::value, Vector<size, Integral>>::type
operator<<(const Vector<size, Integral>& vector,
    typename std::common_type<Integral>::type
    shift)
{
    Vector<size, Integral> copy(vector);
    return copy <<= shift;
}

template<std::size_t size, class Integral> inline
typename std::enable_if<std::is_integral<Integral>::value, Vector<size, Integral>&>::type
operator>>=(Vector<size, Integral>& vector,
    typename std::common_type<Integral>::type
    shift) {
    for(std::size_t i = 0; i != size; ++i)
        vector[i] >>= shift;

    return vector;
}

template<std::size_t size, class Integral> inline
typename std::enable_if<std::is_integral<Integral>::value, Vector<size, Integral>>::type
operator>>(const Vector<size, Integral>& vector,
    typename std::common_type<Integral>::type
    shift) {
    Vector<size, Integral> copy(vector);
    return copy >>= shift;
}

template<std::size_t size, class Integral, class FloatingPoint> inline
typename std::enable_if<std::is_integral<Integral>::value && std::is_floating_point<FloatingPoint>::value, Vector<size, Integral>&>::type
operator*=(Vector<size, Integral>& vector, FloatingPoint scalar) {
    for(std::size_t i = 0; i != size; ++i)
        vector[i] = Integral(vector[i]*scalar);

    return vector;
}

template<std::size_t size, class Integral, class FloatingPoint> inline
typename std::enable_if<std::is_integral<Integral>::value && std::is_floating_point<FloatingPoint>::value, Vector<size, Integral>>::type
operator*(const Vector<size, Integral>& vector, FloatingPoint scalar) {
    Vector<size, Integral> copy(vector);
    return copy *= scalar;
}

template<std::size_t size, class FloatingPoint, class Integral> inline
typename std::enable_if<std::is_integral<Integral>::value && std::is_floating_point<FloatingPoint>::value, Vector<size, Integral>>::type
operator*(FloatingPoint scalar, const Vector<size, Integral>& vector) {
    return vector*scalar;
}

template<std::size_t size, class Integral, class FloatingPoint> inline
typename std::enable_if<std::is_integral<Integral>::value && std::is_floating_point<FloatingPoint>::value, Vector<size, Integral>&>::type
operator/=(Vector<size, Integral>& vector, FloatingPoint scalar) {
    for(std::size_t i = 0; i != size; ++i)
        vector[i] = Integral(vector[i]/scalar);

    return vector;
}

template<std::size_t size, class Integral, class FloatingPoint> inline
typename std::enable_if<std::is_integral<Integral>::value && std::is_floating_point<FloatingPoint>::value, Vector<size, Integral>>::type
operator/(const Vector<size, Integral>& vector, FloatingPoint scalar) {
    Vector<size, Integral> copy(vector);
    return copy /= scalar;
}

template<std::size_t size, class Integral, class FloatingPoint> inline
typename std::enable_if<std::is_integral<Integral>::value && std::is_floating_point<FloatingPoint>::value, Vector<size, Integral>&>::type
operator*=(Vector<size, Integral>& a, const Vector<size, FloatingPoint>& b) {
    for(std::size_t i = 0; i != size; ++i)
        a[i] = Integral(a[i]*b[i]);

    return a;
}

template<std::size_t size, class Integral, class FloatingPoint> inline
typename std::enable_if<std::is_integral<Integral>::value && std::is_floating_point<FloatingPoint>::value, Vector<size, Integral>>::type
operator*(const Vector<size, Integral>& a, const Vector<size, FloatingPoint>& b) {
    Vector<size, Integral> copy(a);
    return copy *= b;
}

template<std::size_t size, class FloatingPoint, class Integral> inline
typename std::enable_if<std::is_integral<Integral>::value && std::is_floating_point<FloatingPoint>::value, Vector<size, Integral>>::type
operator*(const Vector<size, FloatingPoint>& a, const Vector<size, Integral>& b) {
    return b*a;
}

template<std::size_t size, class Integral, class FloatingPoint> inline
typename std::enable_if<std::is_integral<Integral>::value && std::is_floating_point<FloatingPoint>::value, Vector<size, Integral>&>::type
operator/=(Vector<size, Integral>& a, const Vector<size, FloatingPoint>& b) {
    for(std::size_t i = 0; i != size; ++i)
        a[i] = Integral(a[i]/b[i]);

    return a;
}

template<std::size_t size, class Integral, class FloatingPoint> inline
typename std::enable_if<std::is_integral<Integral>::value && std::is_floating_point<FloatingPoint>::value, Vector<size, Integral>>::type
operator/(const Vector<size, Integral>& a, const Vector<size, FloatingPoint>& b) {
    Vector<size, Integral> copy(a);
    return copy /= b;
}

#define MAGNUM_VECTOR_SUBCLASS_IMPLEMENTATION(size, Type)                   \
    static Type<T>& from(T* data) {                                         \
        return *reinterpret_cast<Type<T>*>(data);                           \
    }                                                                       \
    static const Type<T>& from(const T* data) {                             \
        return *reinterpret_cast<const Type<T>*>(data);                     \
    }                                                                       \
    template<std::size_t otherSize> constexpr static Type<T> pad(const Math::Vector<otherSize, T>& a, T value = T(0)) { \
        return Math::Vector<size, T>::pad(a, value);                        \
    }                                                                       \
                                                                            \
    template<class U = T> typename std::enable_if<std::is_signed<U>::value, Type<T>>::type \
    operator-() const {                                                     \
        return Math::Vector<size, T>::operator-();                          \
    }                                                                       \
    Type<T>& operator+=(const Math::Vector<size, T>& other) {               \
        Math::Vector<size, T>::operator+=(other);                           \
        return *this;                                                       \
    }                                                                       \
    Type<T> operator+(const Math::Vector<size, T>& other) const {           \
        return Math::Vector<size, T>::operator+(other);                     \
    }                                                                       \
    Type<T>& operator-=(const Math::Vector<size, T>& other) {               \
        Math::Vector<size, T>::operator-=(other);                           \
        return *this;                                                       \
    }                                                                       \
    Type<T> operator-(const Math::Vector<size, T>& other) const {           \
        return Math::Vector<size, T>::operator-(other);                     \
    }                                                                       \
    Type<T>& operator*=(T number) {                                         \
        Math::Vector<size, T>::operator*=(number);                          \
        return *this;                                                       \
    }                                                                       \
    Type<T> operator*(T number) const {                                     \
        return Math::Vector<size, T>::operator*(number);                    \
    }                                                                       \
    Type<T>& operator/=(T number) {                                         \
        Math::Vector<size, T>::operator/=(number);                          \
        return *this;                                                       \
    }                                                                       \
    Type<T> operator/(T number) const {                                     \
        return Math::Vector<size, T>::operator/(number);                    \
    }                                                                       \
    Type<T>& operator*=(const Math::Vector<size, T>& other) {               \
        Math::Vector<size, T>::operator*=(other);                           \
        return *this;                                                       \
    }                                                                       \
    Type<T> operator*(const Math::Vector<size, T>& other) const {           \
        return Math::Vector<size, T>::operator*(other);                     \
    }                                                                       \
    Type<T>& operator/=(const Math::Vector<size, T>& other) {               \
        Math::Vector<size, T>::operator/=(other);                           \
        return *this;                                                       \
    }                                                                       \
    Type<T> operator/(const Math::Vector<size, T>& other) const {           \
        return Math::Vector<size, T>::operator/(other);                     \
    }                                                                       \
                                                                            \
    template<class U = T> typename std::enable_if<std::is_floating_point<U>::value, Type<T>>::type normalized() const { \
        return Math::Vector<size, T>::normalized();                         \
    }                                                                       \
    template<class U = T> typename std::enable_if<std::is_floating_point<U>::value, Type<T>>::type resized(T length) const { \
        return Math::Vector<size, T>::resized(length);                      \
    }                                                                       \
    template<class U = T> typename std::enable_if<std::is_floating_point<U>::value, Type<T>>::type projected(const Math::Vector<size, T>& other) const {           \
        return Math::Vector<size, T>::projected(other);                     \
    }                                                                       \
    template<class U = T> typename std::enable_if<std::is_floating_point<U>::value, Type<T>>::type projectedOntoNormalized(const Math::Vector<size, T>& other) const { \
        return Math::Vector<size, T>::projectedOntoNormalized(other);       \
    }                                                                       \
    constexpr Type<T> flipped() const {                                     \
        return Math::Vector<size, T>::flipped();                            \
    }

#define MAGNUM_VECTORn_OPERATOR_IMPLEMENTATION(size, Type)                  \
    template<class T> inline Type<T> operator*(typename std::common_type<T>::type number, const Type<T>& vector) { \
        return number*static_cast<const Math::Vector<size, T>&>(vector);    \
    }                                                                       \
    template<class T> inline Type<T> operator/(typename std::common_type<T>::type number, const Type<T>& vector) { \
        return number/static_cast<const Math::Vector<size, T>&>(vector);    \
    }                                                                       \
                                                                            \
    template<class Integral> inline typename std::enable_if<std::is_integral<Integral>::value, Type<Integral>&>::type operator%=(Type<Integral>& a, Integral b) { \
        static_cast<Math::Vector<size, Integral>&>(a) %= b;                 \
        return a;                                                           \
    }                                                                       \
    template<class Integral> inline typename std::enable_if<std::is_integral<Integral>::value, Type<Integral>>::type operator%(const Type<Integral>& a, Integral b) { \
        return static_cast<const Math::Vector<size, Integral>&>(a) % b;     \
    }                                                                       \
    template<class Integral> inline typename std::enable_if<std::is_integral<Integral>::value, Type<Integral>&>::type operator%=(Type<Integral>& a, const Math::Vector<size, Integral>& b) { \
        static_cast<Math::Vector<size, Integral>&>(a) %= b;                 \
        return a;                                                           \
    }                                                                       \
    template<class Integral> inline typename std::enable_if<std::is_integral<Integral>::value, Type<Integral>>::type operator%(const Type<Integral>& a, const Math::Vector<size, Integral>& b) { \
        return static_cast<const Math::Vector<size, Integral>&>(a) % b;     \
    }                                                                       \
                                                                            \
    template<class Integral> inline typename std::enable_if<std::is_integral<Integral>::value, Type<Integral>>::type operator~(const Type<Integral>& vector) { \
        return ~static_cast<const Math::Vector<size, Integral>&>(vector);   \
    }                                                                       \
    template<class Integral> inline typename std::enable_if<std::is_integral<Integral>::value, Type<Integral>&>::type operator&=(Type<Integral>& a, const Math::Vector<size, Integral>& b) { \
        static_cast<Math::Vector<size, Integral>&>(a) &= b;                 \
        return a;                                                           \
    }                                                                       \
    template<class Integral> inline typename std::enable_if<std::is_integral<Integral>::value, Type<Integral>>::type operator&(const Type<Integral>& a, const Math::Vector<size, Integral>& b) { \
        return static_cast<const Math::Vector<size, Integral>&>(a) & b;     \
    }                                                                       \
    template<class Integral> inline typename std::enable_if<std::is_integral<Integral>::value, Type<Integral>&>::type operator|=(Type<Integral>& a, const Math::Vector<size, Integral>& b) { \
        static_cast<Math::Vector<size, Integral>&>(a) |= b;                 \
        return a;                                                           \
    }                                                                       \
    template<class Integral> inline typename std::enable_if<std::is_integral<Integral>::value, Type<Integral>>::type operator|(const Type<Integral>& a, const Math::Vector<size, Integral>& b) { \
        return static_cast<const Math::Vector<size, Integral>&>(a) | b;     \
    }                                                                       \
    template<class Integral> inline typename std::enable_if<std::is_integral<Integral>::value, Type<Integral>&>::type operator^=(Type<Integral>& a, const Math::Vector<size, Integral>& b) { \
        static_cast<Math::Vector<size, Integral>&>(a) ^= b;                 \
        return a;                                                           \
    }                                                                       \
    template<class Integral> inline typename std::enable_if<std::is_integral<Integral>::value, Type<Integral>>::type operator^(const Type<Integral>& a, const Math::Vector<size, Integral>& b) { \
        return static_cast<const Math::Vector<size, Integral>&>(a) ^ b;     \
    }                                                                       \
    template<class Integral> inline typename std::enable_if<std::is_integral<Integral>::value, Type<Integral>&>::type operator<<=(Type<Integral>& vector, typename std::common_type<Integral>::type shift) { \
        static_cast<Math::Vector<size, Integral>&>(vector) <<= shift;       \
        return vector;                                                      \
    }                                                                       \
    template<class Integral> inline typename std::enable_if<std::is_integral<Integral>::value, Type<Integral>>::type operator<<(const Type<Integral>& vector, typename std::common_type<Integral>::type shift) { \
        return static_cast<const Math::Vector<size, Integral>&>(vector) << shift; \
    }                                                                       \
    template<class Integral> inline typename std::enable_if<std::is_integral<Integral>::value, Type<Integral>&>::type operator>>=(Type<Integral>& vector, typename std::common_type<Integral>::type shift) { \
        static_cast<Math::Vector<size, Integral>&>(vector) >>= shift;       \
        return vector;                                                      \
    }                                                                       \
    template<class Integral> inline typename std::enable_if<std::is_integral<Integral>::value, Type<Integral>>::type operator>>(const Type<Integral>& vector, typename std::common_type<Integral>::type shift) { \
        return static_cast<const Math::Vector<size, Integral>&>(vector) >> shift; \
    }                                                                       \
    template<class Integral, class FloatingPoint> inline typename std::enable_if<std::is_integral<Integral>::value && std::is_floating_point<FloatingPoint>::value, Type<Integral>&>::type operator*=(Type<Integral>& vector, FloatingPoint number) { \
        static_cast<Math::Vector<size, Integral>&>(vector) *= number;       \
        return vector;                                                      \
    }                                                                       \
    template<class Integral, class FloatingPoint> inline typename std::enable_if<std::is_integral<Integral>::value && std::is_floating_point<FloatingPoint>::value, Type<Integral>>::type operator*(const Type<Integral>& vector, FloatingPoint number) { \
        return static_cast<const Math::Vector<size, Integral>&>(vector)*number; \
    }                                                                       \
    template<class FloatingPoint, class Integral> inline typename std::enable_if<std::is_integral<Integral>::value && std::is_floating_point<FloatingPoint>::value, Type<Integral>>::type operator*(FloatingPoint number, const Type<Integral>& vector) { \
        return number*static_cast<const Math::Vector<size, Integral>&>(vector); \
    }                                                                       \
    template<class Integral, class FloatingPoint> inline typename std::enable_if<std::is_integral<Integral>::value && std::is_floating_point<FloatingPoint>::value, Type<Integral>&>::type operator/=(Type<Integral>& vector, FloatingPoint number) { \
        static_cast<Math::Vector<size, Integral>&>(vector) /= number;       \
        return vector;                                                      \
    }                                                                       \
    template<class Integral, class FloatingPoint> inline typename std::enable_if<std::is_integral<Integral>::value && std::is_floating_point<FloatingPoint>::value, Type<Integral>>::type operator/(const Type<Integral>& vector, FloatingPoint number) { \
        return static_cast<const Math::Vector<size, Integral>&>(vector)/number; \
    }                                                                       \
                                                                            \
    template<class Integral, class FloatingPoint> inline typename std::enable_if<std::is_integral<Integral>::value && std::is_floating_point<FloatingPoint>::value, Type<Integral>&>::type operator*=(Type<Integral>& a, const Math::Vector<size, FloatingPoint>& b) { \
        static_cast<Math::Vector<size, Integral>&>(a) *= b;                 \
        return a;                                                           \
    }                                                                       \
    template<class Integral, class FloatingPoint> inline typename std::enable_if<std::is_integral<Integral>::value && std::is_floating_point<FloatingPoint>::value, Type<Integral>>::type operator*(const Type<Integral>& a, const Math::Vector<size, FloatingPoint>& b) { \
        return static_cast<const Math::Vector<size, Integral>&>(a)*b;       \
    }                                                                       \
    template<class FloatingPoint, class Integral> inline typename std::enable_if<std::is_integral<Integral>::value && std::is_floating_point<FloatingPoint>::value, Type<Integral>>::type operator*(const Math::Vector<size, FloatingPoint>& a, const Type<Integral>& b) { \
        return a*static_cast<const Math::Vector<size, Integral>&>(b);       \
    }                                                                       \
    template<class Integral, class FloatingPoint> inline typename std::enable_if<std::is_integral<Integral>::value && std::is_floating_point<FloatingPoint>::value, Type<Integral>&>::type operator/=(Type<Integral>& a, const Math::Vector<size, FloatingPoint>& b) { \
        static_cast<Math::Vector<size, Integral>&>(a) /= b;                 \
        return a;                                                           \
    }                                                                       \
    template<class Integral, class FloatingPoint> inline typename std::enable_if<std::is_integral<Integral>::value && std::is_floating_point<FloatingPoint>::value, Type<Integral>>::type operator/(const Type<Integral>& a, const Math::Vector<size, FloatingPoint>& b) { \
        return static_cast<const Math::Vector<size, Integral>&>(a)/b;       \
    }

template<std::size_t size, class T> inline BoolVector<size> Vector<size, T>::operator<(const Vector<size, T>& other) const {
    BoolVector<size> out;

    for(std::size_t i = 0; i != size; ++i)
        out.set(i, _data[i] < other._data[i]);

    return out;
}

template<std::size_t size, class T> inline BoolVector<size> Vector<size, T>::operator<=(const Vector<size, T>& other) const {
    BoolVector<size> out;

    for(std::size_t i = 0; i != size; ++i)
        out.set(i, _data[i] <= other._data[i]);

    return out;
}

template<std::size_t size, class T> inline BoolVector<size> Vector<size, T>::operator>=(const Vector<size, T>& other) const {
    BoolVector<size> out;

    for(std::size_t i = 0; i != size; ++i)
        out.set(i, _data[i] >= other._data[i]);

    return out;
}

template<std::size_t size, class T> inline BoolVector<size> Vector<size, T>::operator>(const Vector<size, T>& other) const {
    BoolVector<size> out;

    for(std::size_t i = 0; i != size; ++i)
        out.set(i, _data[i] > other._data[i]);

    return out;
}

template<std::size_t size, class T>
template<class U> inline typename std::enable_if<std::is_signed<U>::value, Vector<size, T>>::type
Vector<size, T>::operator-() const {
    Vector<size, T> out;

    for(std::size_t i = 0; i != size; ++i)
        out._data[i] = -_data[i];

    return out;
}

template<std::size_t size, class T>
template<class U> inline typename std::enable_if<std::is_floating_point<U>::value, Vector<size, T>>::type
Vector<size, T>::projectedOntoNormalized(const Vector<size, T>& line) const {
    CORRADE_ASSERT(line.isNormalized(),
        "Math::Vector::projectedOntoNormalized(): line" << line << "is not normalized", {});
    return line*Math::dot(*this, line);
}

template<std::size_t size, class T> inline T Vector<size, T>::sum() const {
    T out(_data[0]);

    for(std::size_t i = 1; i != size; ++i)
        out += _data[i];

    return out;
}

template<std::size_t size, class T> inline T Vector<size, T>::product() const {
    T out(_data[0]);

    for(std::size_t i = 1; i != size; ++i)
        out *= _data[i];

    return out;
}

namespace Implementation {
    template<std::size_t size, class T> constexpr std::size_t firstNonNan(const T(&)[size], std::false_type) {
        return 0;
    }
    template<std::size_t size, class T> inline std::size_t firstNonNan(const T(&data)[size], std::true_type) {
        for(std::size_t i = 0; i != size; ++i)
            if(!isNan(data[i])) return i;
        return size - 1;
    }
}

template<std::size_t size, class T> inline T Vector<size, T>::min() const {
    std::size_t i = Implementation::firstNonNan(_data, IsFloatingPoint<T>{});
    T out(_data[i]);

    for(++i; i != size; ++i)
        out = Math::min(out, _data[i]);

    return out;
}

template<std::size_t size, class T> inline T Vector<size, T>::max() const {
    std::size_t i = Implementation::firstNonNan(_data, IsFloatingPoint<T>{});
    T out(_data[i]);

    for(++i; i != size; ++i)
        out = Math::max(out, _data[i]);

    return out;
}

template<std::size_t size, class T> inline std::pair<T, T> Vector<size, T>::minmax() const {
    std::size_t i = Implementation::firstNonNan(_data, IsFloatingPoint<T>{});
    T min{_data[i]}, max{_data[i]};

    for(++i; i != size; ++i) {
        if(_data[i] < min)
            min = _data[i];
        else if(_data[i] > max)
            max = _data[i];
    }

    return {min, max};
}

namespace Implementation {

template<std::size_t size, class T> struct StrictWeakOrdering<Vector<size, T>> {
    bool operator()(const Vector<size, T>& a, const Vector<size, T>& b) const {
        for(std::size_t i = 0; i < size; ++i) {
            if(a[i] < b[i])
                return true;
            if(a[i] > b[i])
                return false;
        }

        return false;
    }
};

}

}}

#endif
#ifndef Magnum_Math_Bezier_h
#define Magnum_Math_Bezier_h

namespace Magnum { namespace Math {

namespace Implementation {
    template<UnsignedInt, UnsignedInt, class, class> struct BezierConverter;
}

template<UnsignedInt order, UnsignedInt dimensions, class T> class Bezier {
    static_assert(order != 0, "Bezier cannot have zero order");

    template<UnsignedInt, UnsignedInt, class> friend class Bezier;

    public:
        typedef T Type;

        enum: UnsignedInt {
            Order = order,
            Dimensions = dimensions
        };

        template<class VectorType> static
        typename std::enable_if<std::is_base_of<Vector<dimensions, T>, VectorType>::value && order == 3, Bezier<order, dimensions, T>>::type
        fromCubicHermite(const CubicHermite<VectorType>& a, const CubicHermite<VectorType>& b) {
            return {a.point(), a.outTangent()/T(3) - a.point(), b.point() - b.inTangent()/T(3), b.point()};
        }

        constexpr /*implicit*/ Bezier() noexcept: Bezier<order, dimensions, T>{typename Implementation::GenerateSequence<order + 1>::Type{}, ZeroInit} {}

        constexpr explicit Bezier(ZeroInitT) noexcept: Bezier<order, dimensions, T>{typename Implementation::GenerateSequence<order + 1>::Type{}, ZeroInit} {}

        explicit Bezier(NoInitT) noexcept: Bezier<order, dimensions, T>{typename Implementation::GenerateSequence<order + 1>::Type{}, NoInit} {}

        template<typename... U> constexpr /*implicit*/ Bezier(const Vector<dimensions, T>& first, U... next) noexcept: _data{first, next...} {
            static_assert(sizeof...(U) + 1 == order + 1, "Wrong number of arguments");
        }

        template<class U> constexpr explicit Bezier(const Bezier<order, dimensions, U>& other) noexcept: Bezier{typename Implementation::GenerateSequence<order + 1>::Type(), other} {}

        template<class U, class V = decltype(Implementation::BezierConverter<order, dimensions, T, U>::from(std::declval<U>()))> constexpr explicit Bezier(const U& other) noexcept: Bezier<order, dimensions, T>{Implementation::BezierConverter<order, dimensions, T, U>::from(other)} {}

        template<class U, class V = decltype(Implementation::BezierConverter<order, dimensions, T, U>::to(std::declval<Bezier<order, dimensions, T>>()))> constexpr explicit operator U() const {
            return Implementation::BezierConverter<order, dimensions, T, U>::to(*this);
        }

        Vector<dimensions, T>* data() { return _data; }
        constexpr const Vector<dimensions, T>* data() const { return _data; }

        bool operator==(const Bezier<order, dimensions, T>& other) const {
            for(std::size_t i = 0; i != order + 1; ++i)
                if(_data[i] != other._data[i]) return false;
            return true;
        }

        bool operator!=(const Bezier<order, dimensions, T>& other) const {
            return !operator==(other);
        }

        Vector<dimensions, T>& operator[](std::size_t i) { return _data[i]; }
        constexpr const Vector<dimensions, T>& operator[](std::size_t i) const { return _data[i]; }

        Vector<dimensions, T> value(Float t) const {
            Bezier<order, dimensions, T> iPoints[order + 1];
            calculateIntermediatePoints(iPoints, t);
            return iPoints[0][order];
        }

        std::pair<Bezier<order, dimensions, T>, Bezier<order, dimensions, T>> subdivide(Float t) const {
            Bezier<order, dimensions, T> iPoints[order + 1];
            calculateIntermediatePoints(iPoints, t);
            Bezier<order, dimensions, T> left, right;
            for(std::size_t i = 0; i <= order; ++i)
                left[i] = iPoints[0][i];
            for(std::size_t i = 0, j = order; i <= order; --j, ++i)
                right[i] = iPoints[i][j];
            return {left, right};
        }

    private:
        template<class U, std::size_t ...sequence> constexpr explicit Bezier(Implementation::Sequence<sequence...>, const Bezier<order, dimensions, U>& other) noexcept: _data{Vector<dimensions, T>(other._data[sequence])...} {}

        template<class U, std::size_t ...sequence> constexpr explicit Bezier(Implementation::Sequence<sequence...>, U): _data{Vector<dimensions, T>((static_cast<void>(sequence), U{typename U::Init{}}))...} {}

        void calculateIntermediatePoints(Bezier<order, dimensions, T>(&iPoints)[order + 1], Float t) const {
            for(std::size_t i = 0; i <= order; ++i) {
                iPoints[i][0] = _data[i];
            }
            for(std::size_t r = 1; r <= order; ++r) {
                for(std::size_t i = 0; i <= order - r; ++i) {
                    iPoints[i][r] = (1 - t)*iPoints[i][r - 1] + t*iPoints[i + 1][r - 1];
                }
            }
        }

        Vector<dimensions, T> _data[order + 1];
};

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<UnsignedInt dimensions, class T> using QuadraticBezier = Bezier<2, dimensions, T>;
#endif

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<class T> using QuadraticBezier2D = QuadraticBezier<2, T>;
#endif

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<class T> using QuadraticBezier3D = QuadraticBezier<3, T>;
#endif

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<UnsignedInt dimensions, class T> using CubicBezier = Bezier<3, dimensions, T>;
#endif

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<class T> using CubicBezier2D = CubicBezier<2, T>;
#endif

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<class T> using CubicBezier3D = CubicBezier<3, T>;
#endif

namespace Implementation {

template<UnsignedInt order, UnsignedInt dimensions, class T> struct StrictWeakOrdering<Bezier<order, dimensions, T>> {
    bool operator()(const Bezier<order, dimensions, T>& a, const Bezier<order, dimensions, T>& b) const {
        StrictWeakOrdering<Vector<dimensions, T>> o;
        for(std::size_t i = 0; i < order + 1; ++i) {
            if(o(a[i], b[i]))
                return true;
            if(o(b[i], a[i]))
                return false;
        }

        return false;
    }
};

}

}}

#endif
#ifndef Magnum_Math_RectangularMatrix_h
#define Magnum_Math_RectangularMatrix_h

namespace Magnum { namespace Math {

namespace Implementation {
    template<std::size_t, std::size_t, class, class> struct RectangularMatrixConverter;
}

template<std::size_t cols, std::size_t rows, class T> class RectangularMatrix {
    static_assert(cols != 0 && rows != 0, "RectangularMatrix cannot have zero elements");

    template<std::size_t, std::size_t, class> friend class RectangularMatrix;

    public:
        typedef T Type;

        enum: std::size_t {
            Cols = cols,
            Rows = rows,

            DiagonalSize = (cols < rows ? cols : rows)
        };

        static RectangularMatrix<cols, rows, T>& from(T* data) {
            return *reinterpret_cast<RectangularMatrix<cols, rows, T>*>(data);
        }
        static const RectangularMatrix<cols, rows, T>& from(const T* data) {
            return *reinterpret_cast<const RectangularMatrix<cols, rows, T>*>(data);
        }

        static RectangularMatrix<cols, rows, T> fromVector(const Vector<cols*rows, T>& vector) {
            return *reinterpret_cast<const RectangularMatrix<cols, rows, T>*>(vector.data());
        }

        constexpr static RectangularMatrix<cols, rows, T> fromDiagonal(const Vector<DiagonalSize, T>& diagonal) noexcept {
            return RectangularMatrix(typename Implementation::GenerateSequence<cols>::Type(), diagonal);
        }

        constexpr /*implicit*/ RectangularMatrix() noexcept: RectangularMatrix<cols, rows, T>{typename Implementation::GenerateSequence<cols>::Type{}, ZeroInit} {}

        constexpr explicit RectangularMatrix(ZeroInitT) noexcept: RectangularMatrix<cols, rows, T>{typename Implementation::GenerateSequence<cols>::Type{}, ZeroInit} {}

        explicit RectangularMatrix(NoInitT) noexcept: RectangularMatrix<cols, rows, T>{typename Implementation::GenerateSequence<cols>::Type{}, NoInit} {}

        template<class ...U> constexpr /*implicit*/ RectangularMatrix(const Vector<rows, T>& first, const U&... next) noexcept: _data{first, next...} {
            static_assert(sizeof...(next)+1 == cols, "Improper number of arguments passed to RectangularMatrix constructor");
        }

        constexpr explicit RectangularMatrix(T value) noexcept: RectangularMatrix{typename Implementation::GenerateSequence<cols>::Type(), value} {}

        template<class U> constexpr explicit RectangularMatrix(const RectangularMatrix<cols, rows, U>& other) noexcept: RectangularMatrix(typename Implementation::GenerateSequence<cols>::Type(), other) {}

        template<class U, class V = decltype(Implementation::RectangularMatrixConverter<cols, rows, T, U>::from(std::declval<U>()))> constexpr explicit RectangularMatrix(const U& other): RectangularMatrix(Implementation::RectangularMatrixConverter<cols, rows, T, U>::from(other)) {}

        constexpr /*implicit*/ RectangularMatrix(const RectangularMatrix<cols, rows, T>&) noexcept = default;

        template<class U, class V = decltype(Implementation::RectangularMatrixConverter<cols, rows, T, U>::to(std::declval<RectangularMatrix<cols, rows, T>>()))> constexpr explicit operator U() const {
            return Implementation::RectangularMatrixConverter<cols, rows, T, U>::to(*this);
        }

        T* data() { return _data[0].data(); }
        constexpr const T* data() const { return _data[0].data(); }

        Vector<rows, T>& operator[](std::size_t col) { return _data[col]; }
        constexpr const Vector<rows, T>& operator[](std::size_t col) const { return _data[col]; }

        Vector<cols, T> row(std::size_t row) const;

        void setRow(std::size_t row, const Vector<cols, T>& data);

        bool operator==(const RectangularMatrix<cols, rows, T>& other) const {
            for(std::size_t i = 0; i != cols; ++i)
                if(_data[i] != other._data[i]) return false;

            return true;
        }

        bool operator!=(const RectangularMatrix<cols, rows, T>& other) const {
            return !operator==(other);
        }

        BoolVector<cols*rows> operator<(const RectangularMatrix<cols, rows, T>& other) const {
            return toVector() < other.toVector();
        }

        BoolVector<cols*rows> operator<=(const RectangularMatrix<cols, rows, T>& other) const {
            return toVector() <= other.toVector();
        }

        BoolVector<cols*rows> operator>=(const RectangularMatrix<cols, rows, T>& other) const {
            return toVector() >= other.toVector();
        }

        BoolVector<cols*rows> operator>(const RectangularMatrix<cols, rows, T>& other) const {
            return toVector() > other.toVector();
        }

        RectangularMatrix<cols, rows, T> operator-() const;

        RectangularMatrix<cols, rows, T>& operator+=(const RectangularMatrix<cols, rows, T>& other) {
            for(std::size_t i = 0; i != cols; ++i)
                _data[i] += other._data[i];

            return *this;
        }

        RectangularMatrix<cols, rows, T> operator+(const RectangularMatrix<cols, rows, T>& other) const {
            return RectangularMatrix<cols, rows, T>(*this)+=other;
        }

        RectangularMatrix<cols, rows, T>& operator-=(const RectangularMatrix<cols, rows, T>& other) {
            for(std::size_t i = 0; i != cols; ++i)
                _data[i] -= other._data[i];

            return *this;
        }

        RectangularMatrix<cols, rows, T> operator-(const RectangularMatrix<cols, rows, T>& other) const {
            return RectangularMatrix<cols, rows, T>(*this)-=other;
        }

        RectangularMatrix<cols, rows, T>& operator*=(T scalar) {
            for(std::size_t i = 0; i != cols; ++i)
                _data[i] *= scalar;

            return *this;
        }

        RectangularMatrix<cols, rows, T> operator*(T scalar) const {
            return RectangularMatrix<cols, rows, T>(*this) *= scalar;
        }

        RectangularMatrix<cols, rows, T>& operator/=(T scalar) {
            for(std::size_t i = 0; i != cols; ++i)
                _data[i] /= scalar;

            return *this;
        }

        RectangularMatrix<cols, rows, T> operator/(T scalar) const {
            return RectangularMatrix<cols, rows, T>(*this) /= scalar;
        }

        template<std::size_t size> RectangularMatrix<size, rows, T> operator*(const RectangularMatrix<size, cols, T>& other) const;

        Vector<rows, T> operator*(const Vector<cols, T>& other) const {
            return operator*(RectangularMatrix<1, cols, T>(other))[0];
        }

        RectangularMatrix<rows, cols, T> transposed() const;

        constexpr RectangularMatrix<cols, rows, T> flippedCols() const {
            return flippedColsInternal(typename Implementation::GenerateSequence<cols>::Type{});
        }

        constexpr RectangularMatrix<cols, rows, T> flippedRows() const {
            return flippedRowsInternal(typename Implementation::GenerateSequence<cols>::Type{});
        }

        constexpr Vector<DiagonalSize, T> diagonal() const {
            return diagonalInternal(typename Implementation::GenerateSequence<DiagonalSize>::Type());
        }

        Vector<rows*cols, T> toVector() const {
            return *reinterpret_cast<const Vector<rows*cols, T>*>(data());
        }

    protected:
        template<std::size_t ...sequence> constexpr explicit RectangularMatrix(Implementation::Sequence<sequence...>, const Vector<DiagonalSize, T>& diagonal);

        template<std::size_t ...sequence> constexpr explicit RectangularMatrix(Implementation::Sequence<sequence...>, T value) noexcept: _data{Vector<rows, T>((static_cast<void>(sequence), value))...} {}

    private:
        template<std::size_t, class> friend class Matrix;
        template<std::size_t, class> friend struct Implementation::MatrixDeterminant;

        template<class U, std::size_t ...sequence> constexpr explicit RectangularMatrix(Implementation::Sequence<sequence...>, const RectangularMatrix<cols, rows, U>& matrix) noexcept: _data{Vector<rows, T>(matrix[sequence])...} {}

        template<class U, std::size_t ...sequence> constexpr explicit RectangularMatrix(Implementation::Sequence<sequence...>, U) noexcept: _data{Vector<rows, T>((static_cast<void>(sequence), U{typename U::Init{}}))...} {}

        template<std::size_t ...sequence> constexpr RectangularMatrix<cols, rows, T> flippedColsInternal(Implementation::Sequence<sequence...>) const {
            return {_data[cols - 1 - sequence]...};
        }

        template<std::size_t ...sequence> constexpr RectangularMatrix<cols, rows, T> flippedRowsInternal(Implementation::Sequence<sequence...>) const {
            return {_data[sequence].flipped()...};
        }

        template<std::size_t ...sequence> constexpr Vector<DiagonalSize, T> diagonalInternal(Implementation::Sequence<sequence...>) const;

        Vector<rows, T> _data[cols];
};

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<class T> using Matrix2x3 = RectangularMatrix<2, 3, T>;
#endif

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<class T> using Matrix3x2 = RectangularMatrix<3, 2, T>;
#endif

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<class T> using Matrix2x4 = RectangularMatrix<2, 4, T>;
#endif

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<class T> using Matrix4x2 = RectangularMatrix<4, 2, T>;
#endif

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<class T> using Matrix3x4 = RectangularMatrix<3, 4, T>;
#endif

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<class T> using Matrix4x3 = RectangularMatrix<4, 3, T>;
#endif

template<std::size_t cols, std::size_t rows, class T> inline RectangularMatrix<cols, rows, T> operator*(
    typename std::common_type<T>::type
    scalar, const RectangularMatrix<cols, rows, T>& matrix)
{
    return matrix*scalar;
}

template<std::size_t cols, std::size_t rows, class T> inline RectangularMatrix<cols, rows, T> operator/(
    typename std::common_type<T>::type
    scalar, const RectangularMatrix<cols, rows, T>& matrix)
{
    RectangularMatrix<cols, rows, T> out{NoInit};

    for(std::size_t i = 0; i != cols; ++i)
        out[i] = scalar/matrix[i];

    return out;
}

template<std::size_t size, std::size_t cols, class T> inline RectangularMatrix<cols, size, T> operator*(const Vector<size, T>& vector, const RectangularMatrix<cols, 1, T>& matrix) {
    return RectangularMatrix<1, size, T>(vector)*matrix;
}

#define MAGNUM_RECTANGULARMATRIX_SUBCLASS_IMPLEMENTATION(cols, rows, ...)   \
    static __VA_ARGS__& from(T* data) {                                     \
        return *reinterpret_cast<__VA_ARGS__*>(data);                       \
    }                                                                       \
    static const __VA_ARGS__& from(const T* data) {                         \
        return *reinterpret_cast<const __VA_ARGS__*>(data);                 \
    }                                                                       \
    constexpr static __VA_ARGS__ fromDiagonal(const Vector<Math::RectangularMatrix<cols, rows, T>::DiagonalSize, T>& diagonal) { \
        return Math::RectangularMatrix<cols, rows, T>::fromDiagonal(diagonal); \
    }                                                                       \
                                                                            \
    __VA_ARGS__ operator-() const {                                         \
        return Math::RectangularMatrix<cols, rows, T>::operator-();         \
    }                                                                       \
    __VA_ARGS__& operator+=(const Math::RectangularMatrix<cols, rows, T>& other) { \
        Math::RectangularMatrix<cols, rows, T>::operator+=(other);          \
        return *this;                                                       \
    }                                                                       \
    __VA_ARGS__ operator+(const Math::RectangularMatrix<cols, rows, T>& other) const { \
        return Math::RectangularMatrix<cols, rows, T>::operator+(other);    \
    }                                                                       \
    __VA_ARGS__& operator-=(const Math::RectangularMatrix<cols, rows, T>& other) { \
        Math::RectangularMatrix<cols, rows, T>::operator-=(other);          \
        return *this;                                                       \
    }                                                                       \
    __VA_ARGS__ operator-(const Math::RectangularMatrix<cols, rows, T>& other) const { \
        return Math::RectangularMatrix<cols, rows, T>::operator-(other);    \
    }                                                                       \
    __VA_ARGS__& operator*=(T number) {                                     \
        Math::RectangularMatrix<cols, rows, T>::operator*=(number);         \
        return *this;                                                       \
    }                                                                       \
    __VA_ARGS__ operator*(T number) const {                                 \
        return Math::RectangularMatrix<cols, rows, T>::operator*(number);   \
    }                                                                       \
    __VA_ARGS__& operator/=(T number) {                                     \
        Math::RectangularMatrix<cols, rows, T>::operator/=(number);         \
        return *this;                                                       \
    }                                                                       \
    __VA_ARGS__ operator/(T number) const {                                 \
        return Math::RectangularMatrix<cols, rows, T>::operator/(number);   \
    }                                                                       \
    constexpr __VA_ARGS__ flippedCols() const {                             \
        return Math::RectangularMatrix<cols, rows, T>::flippedCols();       \
    }                                                                       \
    constexpr __VA_ARGS__ flippedRows() const {                             \
        return Math::RectangularMatrix<cols, rows, T>::flippedRows();       \
    }                                                                       \

#define MAGNUM_MATRIX_OPERATOR_IMPLEMENTATION(...)                          \
    template<std::size_t size, class T> inline __VA_ARGS__ operator*(typename std::common_type<T>::type number, const __VA_ARGS__& matrix) { \
        return number*static_cast<const Math::RectangularMatrix<size, size, T>&>(matrix); \
    }                                                                       \
    template<std::size_t size, class T> inline __VA_ARGS__ operator/(typename std::common_type<T>::type number, const __VA_ARGS__& matrix) { \
        return number/static_cast<const Math::RectangularMatrix<size, size, T>&>(matrix); \
    }                                                                       \
    template<std::size_t size, class T> inline __VA_ARGS__ operator*(const Vector<size, T>& vector, const RectangularMatrix<size, 1, T>& matrix) { \
        return Math::RectangularMatrix<1, size, T>(vector)*matrix;          \
    }

#define MAGNUM_MATRIXn_OPERATOR_IMPLEMENTATION(size, Type)                  \
    template<class T> inline Type<T> operator*(typename std::common_type<T>::type number, const Type<T>& matrix) { \
        return number*static_cast<const Math::RectangularMatrix<size, size, T>&>(matrix); \
    }                                                                       \
    template<class T> inline Type<T> operator/(typename std::common_type<T>::type number, const Type<T>& matrix) { \
        return number/static_cast<const Math::RectangularMatrix<size, size, T>&>(matrix); \
    }                                                                       \
    template<class T> inline Type<T> operator*(const Vector<size, T>& vector, const RectangularMatrix<size, 1, T>& matrix) { \
        return Math::RectangularMatrix<1, size, T>(vector)*matrix;          \
    }

namespace Implementation {
    template<std::size_t rows, std::size_t i, class T, std::size_t ...sequence> constexpr Vector<rows, T> diagonalMatrixColumn2(Implementation::Sequence<sequence...>, const T& number) {
        return {(sequence == i ? number : T(0))...};
    }
    template<std::size_t rows, std::size_t i, class T> constexpr Vector<rows, T> diagonalMatrixColumn(const T& number) {
        return diagonalMatrixColumn2<rows, i, T>(typename Implementation::GenerateSequence<rows>::Type(), number);
    }
}

template<std::size_t cols, std::size_t rows, class T> template<std::size_t ...sequence> constexpr RectangularMatrix<cols, rows, T>::RectangularMatrix(Implementation::Sequence<sequence...>, const Vector<DiagonalSize, T>& diagonal): _data{Implementation::diagonalMatrixColumn<rows, sequence>(sequence < DiagonalSize ? diagonal[sequence] : T{})...} {}

template<std::size_t cols, std::size_t rows, class T> inline Vector<cols, T> RectangularMatrix<cols, rows, T>::row(std::size_t row) const {
    Vector<cols, T> out;

    for(std::size_t i = 0; i != cols; ++i)
        out[i] = _data[i]._data[row];

    return out;
}

template<std::size_t cols, std::size_t rows, class T> inline void RectangularMatrix<cols, rows, T>::setRow(std::size_t row, const Vector<cols, T>& data) {
    for(std::size_t i = 0; i != cols; ++i)
        _data[i]._data[row] = data._data[i];
}

template<std::size_t cols, std::size_t rows, class T> inline RectangularMatrix<cols, rows, T> RectangularMatrix<cols, rows, T>::operator-() const {
    RectangularMatrix<cols, rows, T> out;

    for(std::size_t i = 0; i != cols; ++i)
        out._data[i] = -_data[i];

    return out;
}

template<std::size_t cols, std::size_t rows, class T> template<std::size_t size> inline RectangularMatrix<size, rows, T> RectangularMatrix<cols, rows, T>::operator*(const RectangularMatrix<size, cols, T>& other) const {
    RectangularMatrix<size, rows, T> out{ZeroInit};

    for(std::size_t col = 0; col != size; ++col)
        for(std::size_t row = 0; row != rows; ++row)
            for(std::size_t pos = 0; pos != cols; ++pos)
                out._data[col]._data[row] += _data[pos]._data[row]*other._data[col]._data[pos];

    return out;
}

template<std::size_t cols, std::size_t rows, class T> inline RectangularMatrix<rows, cols, T> RectangularMatrix<cols, rows, T>::transposed() const {
    RectangularMatrix<rows, cols, T> out{NoInit};

    for(std::size_t col = 0; col != cols; ++col)
        for(std::size_t row = 0; row != rows; ++row)
            out._data[row]._data[col] = _data[col]._data[row];

    return out;
}

template<std::size_t cols, std::size_t rows, class T> template<std::size_t ...sequence> constexpr auto RectangularMatrix<cols, rows, T>::diagonalInternal(Implementation::Sequence<sequence...>) const -> Vector<DiagonalSize, T> {
    return {_data[sequence][sequence]...};
}

namespace Implementation {

template<std::size_t cols, std::size_t rows, class T> struct StrictWeakOrdering<RectangularMatrix<cols, rows, T>> {
    bool operator()(const RectangularMatrix<cols, rows, T>& a, const RectangularMatrix<cols, rows, T>& b) const {
        StrictWeakOrdering<Vector<rows, T>> o;
        for(std::size_t i = 0; i < cols; ++i) {
            if(o(a[i], b[i]))
                return true;
            if(o(b[i], a[i]))
                return false;
        }

        return false;
    }
};

}

}}

#endif
#ifndef Magnum_Math_Matrix_h
#define Magnum_Math_Matrix_h

namespace Magnum { namespace Math {

namespace Implementation {
    template<std::size_t, class> struct MatrixDeterminant;

    template<std::size_t size, std::size_t col, std::size_t otherSize, class T, std::size_t ...row> constexpr Vector<size, T> valueOrIdentityVector(Sequence<row...>, const RectangularMatrix<otherSize, otherSize, T>& other) {
        return {(col < otherSize && row < otherSize ? other[col][row] :
            col == row ? T{1} : T{0})...};
    }

    template<std::size_t size, std::size_t col, std::size_t otherSize, class T> constexpr Vector<size, T> valueOrIdentityVector(const RectangularMatrix<otherSize, otherSize, T>& other) {
        return valueOrIdentityVector<size, col>(typename Implementation::GenerateSequence<size>::Type(), other);
    }
}

template<std::size_t size, class T> class Matrix: public RectangularMatrix<size, size, T> {
    public:
        enum: std::size_t {
            Size = size
        };

        constexpr /*implicit*/ Matrix() noexcept: RectangularMatrix<size, size, T>{typename Implementation::GenerateSequence<size>::Type(), Vector<size, T>(T(1))} {}

        constexpr explicit Matrix(IdentityInitT, T value = T(1)) noexcept: RectangularMatrix<size, size, T>{typename Implementation::GenerateSequence<size>::Type(), Vector<size, T>(value)} {}

        constexpr explicit Matrix(ZeroInitT) noexcept: RectangularMatrix<size, size, T>{ZeroInit} {}

        constexpr explicit Matrix(NoInitT) noexcept: RectangularMatrix<size, size, T>{NoInit} {}

        template<class ...U> constexpr /*implicit*/ Matrix(const Vector<size, T>& first, const U&... next) noexcept: RectangularMatrix<size, size, T>(first, next...) {}

        constexpr explicit Matrix(T value) noexcept: RectangularMatrix<size, size, T>{typename Implementation::GenerateSequence<size>::Type(), value} {}

        template<class U> constexpr explicit Matrix(const RectangularMatrix<size, size, U>& other) noexcept: RectangularMatrix<size, size, T>(other) {}

        template<class U, class V = decltype(Implementation::RectangularMatrixConverter<size, size, T, U>::from(std::declval<U>()))> constexpr explicit Matrix(const U& other): RectangularMatrix<size, size, T>(Implementation::RectangularMatrixConverter<size, size, T, U>::from(other)) {}

        template<std::size_t otherSize> constexpr explicit Matrix(const RectangularMatrix<otherSize, otherSize, T>& other) noexcept: Matrix<size, T>{typename Implementation::GenerateSequence<size>::Type(), other} {}

        constexpr /*implicit*/ Matrix(const RectangularMatrix<size, size, T>& other) noexcept: RectangularMatrix<size, size, T>(other) {}

        bool isOrthogonal() const;

        T trace() const { return RectangularMatrix<size, size, T>::diagonal().sum(); }

        Matrix<size-1, T> ij(std::size_t skipCol, std::size_t skipRow) const;

        T cofactor(std::size_t col, std::size_t row) const;

        Matrix<size, T> comatrix() const;

        Matrix<size, T> adjugate() const;

        T determinant() const { return Implementation::MatrixDeterminant<size, T>()(*this); }

        Matrix<size, T> inverted() const;

        Matrix<size, T> invertedOrthogonal() const {
            CORRADE_ASSERT(isOrthogonal(),
                "Math::Matrix::invertedOrthogonal(): the matrix is not orthogonal:" << Corrade::Utility::Debug::Debug::newline << *this, {});
            return RectangularMatrix<size, size, T>::transposed();
        }

        Matrix<size, T> operator*(const Matrix<size, T>& other) const {
            return RectangularMatrix<size, size, T>::operator*(other);
        }
        template<std::size_t otherCols> RectangularMatrix<otherCols, size, T> operator*(const RectangularMatrix<otherCols, size, T>& other) const {
            return RectangularMatrix<size, size, T>::operator*(other);
        }
        Vector<size, T> operator*(const Vector<size, T>& other) const {
            return RectangularMatrix<size, size, T>::operator*(other);
        }
        Matrix<size, T> transposed() const {
            return RectangularMatrix<size, size, T>::transposed();
        }
        MAGNUM_RECTANGULARMATRIX_SUBCLASS_IMPLEMENTATION(size, size, Matrix<size, T>)

    private:
        friend struct Implementation::MatrixDeterminant<size, T>;

        template<std::size_t otherSize, std::size_t ...col> constexpr explicit Matrix(Implementation::Sequence<col...>, const RectangularMatrix<otherSize, otherSize, T>& other) noexcept: RectangularMatrix<size, size, T>{Implementation::valueOrIdentityVector<size, col>(other)...} {}
};

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<class T> using Matrix2x2 = Matrix<2, T>;
#endif

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<class T> using Matrix3x3 = Matrix<3, T>;
#endif

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<class T> using Matrix4x4 = Matrix<4, T>;
#endif

MAGNUM_MATRIX_OPERATOR_IMPLEMENTATION(Matrix<size, T>)

#define MAGNUM_MATRIX_SUBCLASS_IMPLEMENTATION(size, Type, VectorType)       \
    VectorType<T>& operator[](std::size_t col) {                            \
        return static_cast<VectorType<T>&>(Matrix<size, T>::operator[](col)); \
    }                                                                       \
    constexpr const VectorType<T> operator[](std::size_t col) const {       \
        return VectorType<T>(Matrix<size, T>::operator[](col));             \
    }                                                                       \
    VectorType<T> row(std::size_t row) const {                              \
        return VectorType<T>(Matrix<size, T>::row(row));                    \
    }                                                                       \
                                                                            \
    Type<T> operator*(const Matrix<size, T>& other) const {                 \
        return Matrix<size, T>::operator*(other);                           \
    }                                                                       \
    template<std::size_t otherCols> RectangularMatrix<otherCols, size, T> operator*(const RectangularMatrix<otherCols, size, T>& other) const { \
        return Matrix<size, T>::operator*(other);                           \
    }                                                                       \
    VectorType<T> operator*(const Vector<size, T>& other) const {           \
        return Matrix<size, T>::operator*(other);                           \
    }                                                                       \
                                                                            \
    Type<T> transposed() const { return Matrix<size, T>::transposed(); }    \
    constexpr VectorType<T> diagonal() const { return Matrix<size, T>::diagonal(); } \
    Type<T> inverted() const { return Matrix<size, T>::inverted(); }        \
    Type<T> invertedOrthogonal() const {                                    \
        return Matrix<size, T>::invertedOrthogonal();                       \
    }

namespace Implementation {

template<std::size_t size, class T> struct MatrixDeterminant {
    T operator()(const Matrix<size, T>& m) {
        T out(0);

        for(std::size_t col = 0; col != size; ++col)
            out += m._data[col]._data[0]*m.cofactor(col, 0);

        return out;
    }

    T operator()(const Matrix<size + 1, T>& m, const std::size_t skipCol, const std::size_t skipRow) {
        return m.ij(skipCol, skipRow).determinant();
    }
};

template<class T> struct MatrixDeterminant<3, T> {
    constexpr T operator()(const Matrix<3, T>& m) const {
        return m._data[0]._data[0]*((m._data[1]._data[1]*m._data[2]._data[2]) - (m._data[2]._data[1]*m._data[1]._data[2])) -
            m._data[0]._data[1]*(m._data[1]._data[0]*m._data[2]._data[2] - m._data[2]._data[0]*m._data[1]._data[2]) +
            m._data[0]._data[2]*(m._data[1]._data[0]*m._data[2]._data[1] - m._data[2]._data[0]*m._data[1]._data[1]);
    }

    constexpr T operator()(const Matrix<4, T>& m, const std::size_t skipCol, const std::size_t skipRow) const {
        #define _col(i) _data[i + (i >= skipCol)]
        #define _row(i) _data[i + (i >= skipRow)]
        return
            m._col(0)._row(0)*((m._col(1)._row(1)*m._col(2)._row(2)) - (m._col(2)._row(1)*m._col(1)._row(2))) -
            m._col(0)._row(1)*(m._col(1)._row(0)*m._col(2)._row(2) - m._col(2)._row(0)*m._col(1)._row(2)) +
            m._col(0)._row(2)*(m._col(1)._row(0)*m._col(2)._row(1) - m._col(2)._row(0)*m._col(1)._row(1));
        #undef _col
        #undef _row
    }
};

template<class T> struct MatrixDeterminant<2, T> {
    constexpr T operator()(const Matrix<2, T>& m) const {
        return m._data[0]._data[0]*m._data[1]._data[1] - m._data[1]._data[0]*m._data[0]._data[1];
    }

    constexpr T operator()(const Matrix<3, T>& m, const std::size_t skipCol, const std::size_t skipRow) const {
        #define _col(i) _data[i + (i >= skipCol)]
        #define _row(i) _data[i + (i >= skipRow)]
        return m._col(0)._row(0)*m._col(1)._row(1) - m._col(1)._row(0)*m._col(0)._row(1);
        #undef _col
        #undef _row
    }
};

template<class T> struct MatrixDeterminant<1, T> {
    constexpr T operator()(const Matrix<1, T>& m) const {
        return m._data[0]._data[0];
    }

    constexpr T operator()(const Matrix<2, T>& m, const std::size_t skipCol, const std::size_t skipRow) const {
        return m._data[0 + (0 >= skipCol)]._data[0 + (0 >= skipRow)];
    }
};

template<std::size_t size, class T> struct StrictWeakOrdering<Matrix<size, T>>: StrictWeakOrdering<RectangularMatrix<size, size, T>> {};

}

template<std::size_t size, class T> bool Matrix<size, T>::isOrthogonal() const {

    for(std::size_t i = 0; i != size; ++i)
        if(!RectangularMatrix<size, size, T>::_data[i].isNormalized()) return false;

    for(std::size_t i = 0; i != size-1; ++i)
        for(std::size_t j = i+1; j != size; ++j)
            if(dot(RectangularMatrix<size, size, T>::_data[i], RectangularMatrix<size, size, T>::_data[j]) > TypeTraits<T>::epsilon())
                return false;

    return true;
}

template<std::size_t size, class T> Matrix<size-1, T> Matrix<size, T>::ij(const std::size_t skipCol, const std::size_t skipRow) const {
    Matrix<size-1, T> out{NoInit};

    for(std::size_t col = 0; col != size-1; ++col)
        for(std::size_t row = 0; row != size-1; ++row)
            out._data[col]._data[row] = RectangularMatrix<size, size, T>::
                _data[col + (col >= skipCol)]
               ._data[row + (row >= skipRow)];

    return out;
}

template<std::size_t size, class T> T Matrix<size, T>::cofactor(std::size_t col, std::size_t row) const {
    return (((row+col) & 1) ? -1 : 1)*Implementation::MatrixDeterminant<size - 1, T>()(*this, col, row);
}

template<std::size_t size, class T> Matrix<size, T> Matrix<size, T>::comatrix() const {
    Matrix<size, T> out{NoInit};

    for(std::size_t col = 0; col != size; ++col)
        for(std::size_t row = 0; row != size; ++row)
            out._data[col]._data[row] = cofactor(col, row);

    return out;
}

template<std::size_t size, class T> Matrix<size, T> Matrix<size, T>::adjugate() const {
    Matrix<size, T> out{NoInit};

    for(std::size_t col = 0; col != size; ++col)
        for(std::size_t row = 0; row != size; ++row)
            out._data[col]._data[row] = cofactor(row, col);

    return out;
}

template<std::size_t size, class T> Matrix<size, T> Matrix<size, T>::inverted() const {
    return adjugate()/determinant();
}

}}

#endif
#ifndef Magnum_Math_Functions_h
#define Magnum_Math_Functions_h

namespace Magnum { namespace Math {

namespace Implementation {
    template<UnsignedInt exponent> struct Pow {
        Pow() = delete;

        template<class T> constexpr static T pow(T base) {
            return base*Pow<exponent-1>::pow(base);
        }
    };
    template<> struct Pow<0> {
        Pow() = delete;

        template<class T> constexpr static T pow(T) { return T(1); }
    };

    template<class> struct IsBoolVectorOrScalar: std::false_type {};
    template<> struct IsBoolVectorOrScalar<bool>: std::true_type {};
    template<std::size_t size> struct IsBoolVectorOrScalar<BoolVector<size>>: std::true_type {};
}

template<class Integral> inline std::pair<Integral, Integral> div(Integral x, Integral y) {
    static_assert(IsIntegral<Integral>::value && IsScalar<Integral>::value,
        "scalar integral type expected");
    const auto result = std::div(x, y);
    return {result.quot, result.rem};
}

template<class T> inline T sin(Unit<Rad, T> angle) { return std::sin(T(angle)); }
template<class T> inline T sin(Unit<Deg, T> angle) { return sin(Rad<T>(angle)); }

template<class T> inline T cos(Unit<Rad, T> angle) { return std::cos(T(angle)); }
template<class T> inline T cos(Unit<Deg, T> angle) { return cos(Rad<T>(angle)); }

#if defined(__GNUC__) && !defined(__clang__)
namespace Implementation {
    inline void sincos(Float rad, Float& sin, Float& cos) {
        __builtin_sincosf(rad, &sin, &cos);
    }
    inline void sincos(Double rad, Double& sin, Double& cos) {
        __builtin_sincos(rad, &sin, &cos);
    }
    inline void sincos(long double rad, long double& sin, long double& cos) {
        __builtin_sincosl(rad, &sin, &cos);
    }
}
#endif

template<class T> inline std::pair<T, T> sincos(Unit<Rad, T> angle) {
    #if defined(__GNUC__) && !defined(__clang__)
    std::pair<T, T> out;
    Implementation::sincos(T(angle), out.first, out.second);
    return out;
    #else
    return {std::sin(T(angle)), std::cos(T(angle))};
    #endif
}
template<class T> inline std::pair<T, T> sincos(Unit<Deg, T> angle) { return sincos(Rad<T>(angle)); }

template<class T> inline T tan(Unit<Rad, T> angle) { return std::tan(T(angle)); }
template<class T> inline T tan(Unit<Deg, T> angle) { return tan(Rad<T>(angle)); }

template<class T> inline Rad<T> asin(T value) { return Rad<T>(std::asin(value)); }

template<class T> inline Rad<T> acos(T value) { return Rad<T>(std::acos(value)); }

template<class T> inline Rad<T> atan(T value) { return Rad<T>(std::atan(value)); }

template<class T> inline typename std::enable_if<IsScalar<T>::value, bool>::type isInf(T value) {
    return std::isinf(UnderlyingTypeOf<T>(value));
}

template<std::size_t size, class T> inline BoolVector<size> isInf(const Vector<size, T>& value) {
    BoolVector<size> out;
    for(std::size_t i = 0; i != size; ++i)
        out.set(i, Math::isInf(value[i]));
    return out;
}

template<class T> typename std::enable_if<IsScalar<T>::value, bool>::type isNan(T value);

template<std::size_t size, class T> inline BoolVector<size> isNan(const Vector<size, T>& value) {
    BoolVector<size> out;
    for(std::size_t i = 0; i != size; ++i)
        out.set(i, Math::isNan(value[i]));
    return out;
}

template<class T> constexpr typename std::enable_if<IsScalar<T>::value, T>::type min(T value, T min);

template<std::size_t size, class T> inline Vector<size, T> min(const Vector<size, T>& value, const Vector<size, T>& min) {
    Vector<size, T> out{NoInit};
    for(std::size_t i = 0; i != size; ++i)
        out[i] = Math::min(value[i], min[i]);
    return out;
}

template<std::size_t size, class T> inline Vector<size, T> min(const Vector<size, T>& value, T min) {
    Vector<size, T> out{NoInit};
    for(std::size_t i = 0; i != size; ++i)
        out[i] = Math::min(value[i], min);
    return out;
}

template<class T> constexpr typename std::enable_if<IsScalar<T>::value, T>::type max(T a, T b);

template<std::size_t size, class T> Vector<size, T> max(const Vector<size, T>& value, const Vector<size, T>& max) {
    Vector<size, T> out{NoInit};
    for(std::size_t i = 0; i != size; ++i)
        out[i] = Math::max(value[i], max[i]);
    return out;
}

template<std::size_t size, class T> inline Vector<size, T> max(const Vector<size, T>& value, T max) {
    Vector<size, T> out{NoInit};
    for(std::size_t i = 0; i != size; ++i)
        out[i] = Math::max(value[i], max);
    return out;
}

template<class T> inline typename std::enable_if<IsScalar<T>::value, std::pair<T, T>>::type minmax(T a, T b) {
    return a < b ? std::make_pair(a, b) : std::make_pair(b, a);
}

template<std::size_t size, class T> inline std::pair<Vector<size, T>, Vector<size, T>> minmax(const Vector<size, T>& a, const Vector<size, T>& b) {
    using std::swap;
    std::pair<Vector<size, T>, Vector<size, T>> out{a, b};
    for(std::size_t i = 0; i != size; ++i)
        if(out.first[i] > out.second[i]) swap(out.first[i], out.second[i]);
    return out;
}

template<class T> inline typename std::enable_if<IsScalar<T>::value, T>::type clamp(T value, T min, T max) {
    return Math::min(Math::max(value, min), max);
}

template<std::size_t size, class T> inline Vector<size, T> clamp(const Vector<size, T>& value, const Vector<size, T>& min, const Vector<size, T>& max) {
    Vector<size, T> out{NoInit};
    for(std::size_t i = 0; i != size; ++i)
        out[i] = Math::clamp(value[i], min[i], max[i]);
    return out;
}

template<std::size_t size, class T> inline Vector<size, T> clamp(const Vector<size, T>& value, T min, T max) {
    Vector<size, T> out{NoInit};
    for(std::size_t i = 0; i != size; ++i)
        out[i] = Math::clamp(value[i], min, max);
    return out;
}

template<class T> inline typename std::enable_if<IsScalar<T>::value, T>::type sign(const T& scalar) {
    if(scalar > T(0)) return T(1);
    if(scalar < T(0)) return T(-1);
    return T(0);
}

template<std::size_t size, class T> inline Vector<size, T> sign(const Vector<size, T>& a) {
    Vector<size, T> out{NoInit};
    for(std::size_t i = 0; i != size; ++i)
        out[i] = Math::sign(a[i]);
    return out;
}

template<class T> inline typename std::enable_if<IsScalar<T>::value, T>::type abs(T a) {
    return T(std::abs(UnderlyingTypeOf<T>(a)));
}

template<std::size_t size, class T> inline Vector<size, T> abs(const Vector<size, T>& a) {
    Vector<size, T> out{NoInit};
    for(std::size_t i = 0; i != size; ++i)
        out[i] = Math::abs(a[i]);
    return out;
}

template<class T> inline typename std::enable_if<IsScalar<T>::value, T>::type floor(T a) {
    return T(std::floor(UnderlyingTypeOf<T>(a)));
}

template<std::size_t size, class T> inline Vector<size, T> floor(const Vector<size, T>& a) {
    Vector<size, T> out{NoInit};
    for(std::size_t i = 0; i != size; ++i)
        out[i] = Math::floor(a[i]);
    return out;
}

template<class T> inline typename std::enable_if<IsScalar<T>::value, T>::type round(T a) {
    return T(std::round(UnderlyingTypeOf<T>(a)));
}

template<std::size_t size, class T> inline Vector<size, T> round(const Vector<size, T>& a) {
    Vector<size, T> out{NoInit};
    for(std::size_t i = 0; i != size; ++i)
        out[i] = Math::round(a[i]);
    return out;
}

template<class T> inline typename std::enable_if<IsScalar<T>::value, T>::type ceil(T a) {
    return T(std::ceil(UnderlyingTypeOf<T>(a)));
}

template<std::size_t size, class T> inline Vector<size, T> ceil(const Vector<size, T>& a) {
    Vector<size, T> out{NoInit};
    for(std::size_t i = 0; i != size; ++i)
        out[i] = Math::ceil(a[i]);
    return out;
}

template<class T, class U> inline
    typename std::enable_if<(IsVector<T>::value || IsScalar<T>::value) && !Implementation::IsBoolVectorOrScalar<U>::value, T>::type
lerp(const T& a, const T& b, U t) {
    return Implementation::lerp(a, b, t);
}

template<class T> inline T lerp(const T& a, const T& b, bool t) {
    return t ? b : a;
}

template<std::size_t size, class T> inline Vector<size, T> lerp(const Vector<size, T>& a, const Vector<size, T>& b, const BoolVector<size>& t) {
    Vector<size, T> out{NoInit};
    for(std::size_t i = 0; i != size; ++i)
        out[i] = t[i] ? b[i] : a[i];
    return out;
}

template<std::size_t size> inline BoolVector<size> lerp(const BoolVector<size>& a, const BoolVector<size>& b, const BoolVector<size>& t) {
    BoolVector<size> out;
    for(std::size_t i = 0; i != size; ++i)
        out.set(i, t[i] ? b[i] : a[i]);
    return out;
}

template<class T> inline UnderlyingTypeOf<typename std::enable_if<IsScalar<T>::value, T>::type> lerpInverted(T a, T b, T lerp) {
    return (lerp - a)/(b - a);
}

template<std::size_t size, class T> inline Vector<size, UnderlyingTypeOf<T>> lerpInverted(const Vector<size, T>& a, const Vector<size, T>& b, const Vector<size, T>& lerp) {
    return (lerp - a)/(b - a);
}

template<class T, class U> constexpr T select(const T& a, const T& b, U t) {
    return lerp(a, b, t >= U(1));
}

template<class T> inline typename std::enable_if<IsScalar<T>::value, T>::type fma(T a, T b, T c) {
    static_assert(IsUnitless<T>::value, "expecting an unitless type");
    #ifndef CORRADE_TARGET_EMSCRIPTEN
    return std::fma(a, b, c);
    #else
    return a*b + c;
    #endif
}

template<std::size_t size, class T> inline Vector<size, T> fma(const Vector<size, T>& a, const Vector<size, T>& b, const Vector<size, T>& c) {
    static_assert(IsUnitless<T>::value, "expecting an unitless type");
    return a*b + c;
}

UnsignedInt MAGNUM_EXPORT log(UnsignedInt base, UnsignedInt number);

UnsignedInt MAGNUM_EXPORT log2(UnsignedInt number);

template<class T> inline T log(T number) { return std::log(number); }

template<class T> inline T exp(T exponent) { return std::exp(exponent); }

template<UnsignedInt exponent, class T> constexpr typename std::enable_if<IsScalar<T>::value, T>::type pow(T base) {
    static_assert(IsUnitless<T>::value, "expected an unitless type");
    return Implementation::Pow<exponent>::pow(base);
}

template<UnsignedInt exponent, std::size_t size, class T> inline Vector<size, T> pow(const Vector<size, T>& base) {
    Vector<size, T> out{NoInit};
    for(std::size_t i = 0; i != size; ++i)
        out[i] = Math::pow<exponent>(base[i]);
    return out;
}

template<class T> inline typename std::enable_if<IsScalar<T>::value, T>::type pow(T base, T exponent) {
    static_assert(IsUnitless<T>::value, "expected an unitless type");
    return std::pow(base, exponent);
}

template<std::size_t size, class T> inline Vector<size, T> pow(const Vector<size, T>& base, T exponent) {
    Vector<size, T> out{NoInit};
    for(std::size_t i = 0; i != size; ++i)
        out[i] = Math::pow(base[i], exponent);
    return out;
}

template<class T> inline typename std::enable_if<IsScalar<T>::value, T>::type sqrt(T a) {
    static_assert(IsUnitless<T>::value, "expecting an unitless type");
    return std::sqrt(a);
}

template<std::size_t size, class T> inline Vector<size, T> sqrt(const Vector<size, T>& a) {
    Vector<size, T> out{NoInit};
    for(std::size_t i = 0; i != size; ++i)
        out[i] = Math::sqrt(a[i]);
    return out;
}

template<class T> inline typename std::enable_if<IsScalar<T>::value, T>::type sqrtInverted(T a) {
    static_assert(IsUnitless<T>::value, "expecting an unitless type");
    return T(1)/std::sqrt(a);
}

template<std::size_t size, class T> inline Vector<size, T> sqrtInverted(const Vector<size, T>& a) {
    return Vector<size, T>(T(1))/Math::sqrt(a);
}

}}

#endif
#ifndef Magnum_Math_Packing_h
#define Magnum_Math_Packing_h

namespace Magnum { namespace Math {

namespace Implementation {

template<class T, UnsignedInt bits = sizeof(T)*8> inline constexpr T bitMax() {
    return T(typename std::make_unsigned<T>::type(~T{}) >> (sizeof(T)*8 - (std::is_signed<T>::value ? bits - 1 : bits)));
}

}

template<class FloatingPoint, class Integral, UnsignedInt bits = sizeof(Integral)*8> inline typename std::enable_if<IsScalar<Integral>::value && std::is_unsigned<Integral>::value, FloatingPoint>::type unpack(const Integral& value) {
    static_assert(IsFloatingPoint<FloatingPoint>::value && IsIntegral<Integral>::value,
        "unpacking must be done from integral to floating-point type");
    static_assert(bits <= sizeof(Integral)*8,
        "bit count larger than size of the integral type");
    return FloatingPoint(value/UnderlyingTypeOf<FloatingPoint>(Implementation::bitMax<Integral, bits>()));
}
template<class FloatingPoint, class Integral, UnsignedInt bits = sizeof(Integral)*8> inline typename std::enable_if<IsScalar<Integral>::value && std::is_signed<Integral>::value, FloatingPoint>::type unpack(const Integral& value) {
    static_assert(IsFloatingPoint<FloatingPoint>::value && IsIntegral<Integral>::value,
        "unpacking must be done from integral to floating-point type");
    static_assert(bits <= sizeof(Integral)*8,
        "bit count larger than size of the integral type");
    return FloatingPoint(Math::max(value/UnderlyingTypeOf<FloatingPoint>(Implementation::bitMax<Integral, bits>()), UnderlyingTypeOf<FloatingPoint>(-1.0)));
}
template<class FloatingPoint, std::size_t size, class Integral, UnsignedInt bits = sizeof(Integral)*8> FloatingPoint unpack(const Vector<size, Integral>& value) {
    static_assert(FloatingPoint::Size == size,
        "return vector type should have the same size as input vector type");
    FloatingPoint out{NoInit};
    for(std::size_t i = 0; i != size; ++i)
        out[i] = unpack<typename FloatingPoint::Type, Integral, bits>(value[i]);
    return out;
}

template<class FloatingPoint, UnsignedInt bits, class Integral> inline typename std::enable_if<IsScalar<Integral>::value, FloatingPoint>::type unpack(const Integral& value) {
    return unpack<FloatingPoint, Integral, bits>(value);
}
template<class FloatingPoint, UnsignedInt bits, std::size_t size, class Integral> inline FloatingPoint unpack(const Vector<size, Integral>& value) {
    return unpack<FloatingPoint, size, Integral, bits>(value);
}

template<class Integral, class FloatingPoint, UnsignedInt bits = sizeof(Integral)*8> inline typename std::enable_if<IsScalar<FloatingPoint>::value, Integral>::type pack(FloatingPoint value) {
    static_assert(IsFloatingPoint<FloatingPoint>::value && IsIntegral<Integral>::value,
        "packing must be done from floating-point to integral type");
    static_assert(bits <= sizeof(Integral)*8,
        "bit count larger than size of the integral type");
    return Integral(round(UnderlyingTypeOf<FloatingPoint>(value)*Implementation::bitMax<Integral, bits>()));
}
template<class Integral, std::size_t size, class FloatingPoint, UnsignedInt bits = sizeof(typename Integral::Type)*8> Integral pack(const Vector<size, FloatingPoint>& value) {
    static_assert(Integral::Size == size,
        "return vector type should have the same size as input vector type");
    Integral out{NoInit};
    for(std::size_t i = 0; i != size; ++i)
        out[i] = pack<typename Integral::Type, FloatingPoint, bits>(value[i]);
    return out;
}

template<class Integral, UnsignedInt bits, class FloatingPoint> inline typename std::enable_if<IsScalar<FloatingPoint>::value, Integral>::type pack(FloatingPoint value) {
    return pack<Integral, FloatingPoint, bits>(value);
}
template<class Integral, UnsignedInt bits, std::size_t size, class FloatingPoint> inline Integral pack(const Vector<size, FloatingPoint>& value) {
    return pack<Integral, size, FloatingPoint, bits>(value);
}

MAGNUM_EXPORT UnsignedShort packHalf(Float value);

template<std::size_t size> Vector<size, UnsignedShort> packHalf(const Vector<size, Float>& value) {
    Vector<size, UnsignedShort> out{NoInit};
    for(std::size_t i = 0; i != size; ++i)
        out[i] = packHalf(value[i]);
    return out;
}

MAGNUM_EXPORT Float unpackHalf(UnsignedShort value);

template<std::size_t size> Vector<size, Float> unpackHalf(const Vector<size, UnsignedShort>& value) {
    Vector<size, Float> out{NoInit};
    for(std::size_t i = 0; i != size; ++i)
        out[i] = unpackHalf(value[i]);
    return out;
}

}}

#endif
#ifndef Magnum_Math_Vector2_h
#define Magnum_Math_Vector2_h

namespace Magnum { namespace Math {

template<class T> inline T cross(const Vector2<T>& a, const Vector2<T>& b) {
    return dot(a.perpendicular(), b);
}

template<class T> class Vector2: public Vector<2, T> {
    public:
        constexpr static Vector2<T> xAxis(T length = T(1)) { return {length, T(0)}; }

        constexpr static Vector2<T> yAxis(T length = T(1)) { return {T(0), length}; }

        constexpr static Vector2<T> xScale(T scale) { return {scale, T(1)}; }

        constexpr static Vector2<T> yScale(T scale) { return {T(1), scale}; }

        constexpr /*implicit*/ Vector2() noexcept: Vector<2, T>{ZeroInit} {}

        constexpr explicit Vector2(ZeroInitT) noexcept: Vector<2, T>{ZeroInit} {}

        explicit Vector2(NoInitT) noexcept: Vector<2, T>{NoInit} {}

        constexpr explicit Vector2(T value) noexcept: Vector<2, T>(value) {}

        constexpr /*implicit*/ Vector2(T x, T y) noexcept: Vector<2, T>(x, y) {}

        template<class U> constexpr explicit Vector2(const Vector<2, U>& other) noexcept: Vector<2, T>(other) {}

        template<class U, class V =
            #ifndef CORRADE_MSVC2015_COMPATIBILITY /* Causes ICE */
            decltype(Implementation::VectorConverter<2, T, U>::from(std::declval<U>()))
            #else
            decltype(Implementation::VectorConverter<2, T, U>())
            #endif
            >
        constexpr explicit Vector2(const U& other): Vector<2, T>(Implementation::VectorConverter<2, T, U>::from(other)) {}

        constexpr /*implicit*/ Vector2(const Vector<2, T>& other) noexcept: Vector<2, T>(other) {}

        T& x() { return Vector<2, T>::_data[0]; }
        constexpr T x() const { return Vector<2, T>::_data[0]; }
        T& y() { return Vector<2, T>::_data[1]; }
        constexpr T y() const { return Vector<2, T>::_data[1]; }

        template<class U = T> typename std::enable_if<std::is_signed<U>::value, Vector2<T>>::type
        perpendicular() const { return {-y(), x()}; }

        template<class U = T> typename std::enable_if<std::is_floating_point<U>::value, T>::type
        aspectRatio() const { return x()/y(); }

        MAGNUM_VECTOR_SUBCLASS_IMPLEMENTATION(2, Vector2)
};

MAGNUM_VECTORn_OPERATOR_IMPLEMENTATION(2, Vector2)

namespace Implementation {
    template<std::size_t, class> struct TypeForSize;
    template<class T> struct TypeForSize<2, T> { typedef Math::Vector2<typename T::Type> Type; };

    template<class T> struct StrictWeakOrdering<Vector2<T>>: StrictWeakOrdering<Vector<2, T>> {};
}

}}

#endif
#ifndef Magnum_Math_Swizzle_h
#define Magnum_Math_Swizzle_h

namespace Magnum { namespace Math {

namespace Implementation {
    template<std::size_t size, std::size_t position> struct GatherComponentAt {
        static_assert(size > position, "numeric swizzle parameter out of range of gather vector, use either xyzw/rgba/0/1 letters or small enough numbers");

        template<class T> constexpr static T value(const Math::Vector<size, T>& vector) {
            return vector._data[position];
        }
    };

    template<std::size_t size, char component> struct GatherComponent: GatherComponentAt<size, component> {};
    template<std::size_t size> struct GatherComponent<size, 'x'>: public GatherComponentAt<size, 0> {};
    template<std::size_t size> struct GatherComponent<size, 'y'>: public GatherComponentAt<size, 1> {};
    template<std::size_t size> struct GatherComponent<size, 'z'>: public GatherComponentAt<size, 2> {};
    template<std::size_t size> struct GatherComponent<size, 'w'>: public GatherComponentAt<size, 3> {};
    template<std::size_t size> struct GatherComponent<size, 'r'>: public GatherComponentAt<size, 0> {};
    template<std::size_t size> struct GatherComponent<size, 'g'>: public GatherComponentAt<size, 1> {};
    template<std::size_t size> struct GatherComponent<size, 'b'>: public GatherComponentAt<size, 2> {};
    template<std::size_t size> struct GatherComponent<size, 'a'>: public GatherComponentAt<size, 3> {};
    template<std::size_t size> struct GatherComponent<size, '0'> {
        template<class T> constexpr static T value(const Math::Vector<size, T>&) { return T(0); }
    };
    template<std::size_t size> struct GatherComponent<size, '1'> {
        template<class T> constexpr static T value(const Math::Vector<size, T>&) { return T(1); }
    };

    template<std::size_t size, class T> struct TypeForSize {
        typedef Math::Vector<size, typename T::Type> Type;
    };

    template<std::size_t size, std::size_t i, bool = true> struct ScatterComponentOr {
        template<class T> constexpr static T value(const Math::Vector<size, T>&, const T& value) {
            return value;
        }
    };
    template<std::size_t size, std::size_t i> struct ScatterComponentOr<size, i, false> {
        template<class T> constexpr static T value(const Math::Vector<size, T>& vector, const T&) {
            return vector._data[i];
        }
    };
    template<std::size_t size, char component, std::size_t i> struct ScatterComponent: ScatterComponentOr<size, i, component == i> {
        static_assert(component == 'x' || component == 'r' ||
                    ((component == 'y' || component == 'g') && size > 1) ||
                    ((component == 'z' || component == 'b') && size > 2) ||
                    ((component == 'w' || component == 'a') && size > 3) ||
                     std::size_t(component) < size,
            "swizzle parameter out of range of scatter vector, use either xyzw/rgba letters or small enough numbers");
    };
    template<std::size_t size> struct ScatterComponent<size, 'x', 0>: ScatterComponentOr<size, 0> {};
    template<std::size_t size> struct ScatterComponent<size, 'y', 1>: ScatterComponentOr<size, 1> {};
    template<std::size_t size> struct ScatterComponent<size, 'z', 2>: ScatterComponentOr<size, 2> {};
    template<std::size_t size> struct ScatterComponent<size, 'w', 3>: ScatterComponentOr<size, 3> {};
    template<std::size_t size> struct ScatterComponent<size, 'r', 0>: ScatterComponentOr<size, 0> {};
    template<std::size_t size> struct ScatterComponent<size, 'g', 1>: ScatterComponentOr<size, 1> {};
    template<std::size_t size> struct ScatterComponent<size, 'b', 2>: ScatterComponentOr<size, 2> {};
    template<std::size_t size> struct ScatterComponent<size, 'a', 3>: ScatterComponentOr<size, 3> {};

    template<class T, char component, std::size_t ...sequence> constexpr T scatterComponentOr(const T& vector, const typename T::Type& value, Sequence<sequence...>) {
        return {ScatterComponent<T::Size, component, sequence>::value(vector, value)...};
    }
    template<class T, std::size_t valueSize> constexpr T scatterRecursive(const T& vector, const Vector<valueSize, typename T::Type>&, std::size_t) {
        return vector;
    }
    template<class T, std::size_t valueSize, char component, char ...next> constexpr T scatterRecursive(const T& vector, const Vector<valueSize, typename T::Type>& values, std::size_t valueIndex) {
        return scatterRecursive<T, valueSize, next...>(
            scatterComponentOr<T, component>(vector, values._data[valueIndex], typename GenerateSequence<T::Size>::Type{}),
            values, valueIndex + 1);
    }
}

template<char ...components, class T> constexpr typename Implementation::TypeForSize<sizeof...(components), T>::Type gather(const T& vector) {
    return {Implementation::GatherComponent<T::Size, components>::value(vector)...};
}

template<char ...components, class T> constexpr T scatter(const T& vector, const typename std::common_type<Vector<sizeof...(components), typename T::Type>>::type& values)
{
    return Implementation::scatterRecursive<T, sizeof...(components), components...>(vector, values, 0);
}

}}

#endif
#ifndef Magnum_Math_Vector3_h
#define Magnum_Math_Vector3_h

namespace Magnum { namespace Math {

template<class T> inline Vector3<T> cross(const Vector3<T>& a, const Vector3<T>& b) {
    return gather<'y', 'z', 'x'>(a*gather<'y', 'z', 'x'>(b) -
                                 b*gather<'y', 'z', 'x'>(a));
}

template<class T> class Vector3: public Vector<3, T> {
    public:
        constexpr static Vector3<T> xAxis(T length = T(1)) { return {length, T(0), T(0)}; }

        constexpr static Vector3<T> yAxis(T length = T(1)) { return {T(0), length, T(0)}; }

        constexpr static Vector3<T> zAxis(T length = T(1)) { return {T(0), T(0), length}; }

        constexpr static Vector3<T> xScale(T scale) { return {scale, T(1), T(1)}; }

        constexpr static Vector3<T> yScale(T scale) { return {T(1), scale, T(1)}; }

        constexpr static Vector3<T> zScale(T scale) { return {T(1), T(1), scale}; }

        constexpr /*implicit*/ Vector3() noexcept: Vector<3, T>{ZeroInit} {}

        constexpr explicit Vector3(ZeroInitT) noexcept: Vector<3, T>{ZeroInit} {}

        explicit Vector3(NoInitT) noexcept: Vector<3, T>{NoInit} {}

        constexpr explicit Vector3(T value) noexcept: Vector<3, T>(value) {}

        constexpr /*implicit*/ Vector3(T x, T y, T z) noexcept: Vector<3, T>(x, y, z) {}

        constexpr /*implicit*/ Vector3(const Vector2<T>& xy, T z) noexcept: Vector<3, T>(xy[0], xy[1], z) {}

        template<class U> constexpr explicit Vector3(const Vector<3, U>& other) noexcept: Vector<3, T>(other) {}

        template<class U, class V =
            #ifndef CORRADE_MSVC2015_COMPATIBILITY /* Causes ICE */
            decltype(Implementation::VectorConverter<3, T, U>::from(std::declval<U>()))
            #else
            decltype(Implementation::VectorConverter<3, T, U>())
            #endif
            >
        constexpr explicit Vector3(const U& other): Vector<3, T>(Implementation::VectorConverter<3, T, U>::from(other)) {}

        constexpr /*implicit*/ Vector3(const Vector<3, T>& other) noexcept: Vector<3, T>(other) {}

        T& x() { return Vector<3, T>::_data[0]; }
        constexpr T x() const { return Vector<3, T>::_data[0]; }

        T& y() { return Vector<3, T>::_data[1]; }
        constexpr T y() const { return Vector<3, T>::_data[1]; }

        T& z() { return Vector<3, T>::_data[2]; }
        constexpr T z() const { return Vector<3, T>::_data[2]; }

        T& r() { return Vector<3, T>::_data[0]; }
        constexpr T r() const { return Vector<3, T>::_data[0]; }

        T& g() { return Vector<3, T>::_data[1]; }
        constexpr T g() const { return Vector<3, T>::_data[1]; }

        T& b() { return Vector<3, T>::_data[2]; }
        constexpr T b() const { return Vector<3, T>::_data[2]; }

        Vector2<T>& xy() { return Vector2<T>::from(Vector<3, T>::data()); }
        constexpr const Vector2<T> xy() const {
            return {Vector<3, T>::_data[0], Vector<3, T>::_data[1]};
        }

        MAGNUM_VECTOR_SUBCLASS_IMPLEMENTATION(3, Vector3)
};

MAGNUM_VECTORn_OPERATOR_IMPLEMENTATION(3, Vector3)

namespace Implementation {
    template<class T> struct TypeForSize<3, T> { typedef Math::Vector3<typename T::Type> Type; };

    template<class T> struct StrictWeakOrdering<Vector3<T>>: StrictWeakOrdering<Vector<3, T>> {};
}

}}

#endif
#ifndef Magnum_Math_Vector4_h
#define Magnum_Math_Vector4_h

namespace Magnum { namespace Math {

template<class T> class Vector4: public Vector<4, T> {
    public:
        template<std::size_t otherSize> constexpr static Vector4<T> pad(const Vector<otherSize, T>& a, T xyz, T w) {
            return {0 < otherSize ? a[0] : xyz,
                    1 < otherSize ? a[1] : xyz,
                    2 < otherSize ? a[2] : xyz,
                    3 < otherSize ? a[3] : w};
        }

        constexpr /*implicit*/ Vector4() noexcept: Vector<4, T>{ZeroInit} {}

        constexpr explicit Vector4(ZeroInitT) noexcept: Vector<4, T>{ZeroInit} {}

        explicit Vector4(NoInitT) noexcept: Vector<4, T>{NoInit} {}

        constexpr explicit Vector4(T value) noexcept: Vector<4, T>(value) {}

        constexpr /*implicit*/ Vector4(T x, T y, T z, T w) noexcept: Vector<4, T>(x, y, z, w) {}

        constexpr /*implicit*/ Vector4(const Vector3<T>& xyz, T w) noexcept: Vector<4, T>(xyz[0], xyz[1], xyz[2], w) {}

        template<class U> constexpr explicit Vector4(const Vector<4, U>& other) noexcept: Vector<4, T>(other) {}

        template<class U, class V = decltype(Implementation::VectorConverter<4, T, U>::from(std::declval<U>()))> constexpr explicit Vector4(const U& other): Vector<4, T>(Implementation::VectorConverter<4, T, U>::from(other)) {}

        constexpr /*implicit*/ Vector4(const Vector<4, T>& other) noexcept: Vector<4, T>(other) {}

        T& x() { return Vector<4, T>::_data[0]; }
        constexpr T x() const { return Vector<4, T>::_data[0]; }

        T& y() { return Vector<4, T>::_data[1]; }
        constexpr T y() const { return Vector<4, T>::_data[1]; }

        T& z() { return Vector<4, T>::_data[2]; }
        constexpr T z() const { return Vector<4, T>::_data[2]; }

        T& w() { return Vector<4, T>::_data[3]; }
        constexpr T w() const { return Vector<4, T>::_data[3]; }

        T& r() { return Vector<4, T>::_data[0]; }
        constexpr T r() const { return Vector<4, T>::_data[0]; }

        T& g() { return Vector<4, T>::_data[1]; }
        constexpr T g() const { return Vector<4, T>::_data[1]; }

        T& b() { return Vector<4, T>::_data[2]; }
        constexpr T b() const { return Vector<4, T>::_data[2]; }

        T& a() { return Vector<4, T>::_data[3]; }
        constexpr T a() const { return Vector<4, T>::_data[3]; }

        Vector3<T>& xyz() { return Vector3<T>::from(Vector<4, T>::data()); }
        constexpr const Vector3<T> xyz() const {
            return {Vector<4, T>::_data[0], Vector<4, T>::_data[1], Vector<4, T>::_data[2]};
        }

        Vector3<T>& rgb() { return Vector3<T>::from(Vector<4, T>::data()); }
        constexpr const Vector3<T> rgb() const {
            return {Vector<4, T>::_data[0], Vector<4, T>::_data[1], Vector<4, T>::_data[2]};
        }

        Vector2<T>& xy() { return Vector2<T>::from(Vector<4, T>::data()); }
        constexpr const Vector2<T> xy() const {
            return {Vector<4, T>::_data[0], Vector<4, T>::_data[1]};
        }

        MAGNUM_VECTOR_SUBCLASS_IMPLEMENTATION(4, Vector4)
};

template<class T> Vector4<T> planeEquation(const Vector3<T>& p0, const Vector3<T>& p1, const Vector3<T>& p2) {
    const Vector3<T> normal = Math::cross(p1 - p0, p2 - p0).normalized();
    return {normal, -Math::dot(normal, p0)};
}

template<class T> Vector4<T> planeEquation(const Vector3<T>& normal, const Vector3<T>& point) {
    return {normal, -Math::dot(normal, point)};
}

MAGNUM_VECTORn_OPERATOR_IMPLEMENTATION(4, Vector4)

namespace Implementation {
    template<class T> struct TypeForSize<4, T> { typedef Math::Vector4<typename T::Type> Type; };

    template<class T> struct StrictWeakOrdering<Vector4<T>>: StrictWeakOrdering<Vector<4, T>> {};
}

}}

#endif
#ifndef Magnum_Math_Color_h
#define Magnum_Math_Color_h

namespace Magnum { namespace Math {

namespace Implementation {

template<class T> typename std::enable_if<std::is_floating_point<T>::value, Color3<T>>::type fromHsv(ColorHsv<T> hsv) {
    hsv.hue -= floor(T(hsv.hue)/T(360))*Deg<T>(360);
    if(hsv.hue < Deg<T>(0)) hsv.hue += Deg<T>(360);

    int h = int(T(hsv.hue)/T(60)) % 6;
    T f = T(hsv.hue)/T(60) - h;

    T p = hsv.value * (T(1) - hsv.saturation);
    T q = hsv.value * (T(1) - f*hsv.saturation);
    T t = hsv.value * (T(1) - (T(1) - f)*hsv.saturation);

    switch(h) {
        case 0: return {hsv.value, t, p};
        case 1: return {q, hsv.value, p};
        case 2: return {p, hsv.value, t};
        case 3: return {p, q, hsv.value};
        case 4: return {t, p, hsv.value};
        case 5: return {hsv.value, p, q};
        default: CORRADE_ASSERT_UNREACHABLE();
    }
}
template<class T> inline typename std::enable_if<std::is_integral<T>::value, Color3<T>>::type fromHsv(const ColorHsv<typename TypeTraits<T>::FloatingPointType>& hsv) {
    return pack<Color3<T>>(fromHsv<typename TypeTraits<T>::FloatingPointType>(hsv));
}

template<class T> Deg<T> hue(const Color3<T>& color, T max, T delta) {
    T deltaInv60 = T(60)/delta;

    T hue(0);
    if(delta != T(0)) {
        if(max == color.r())
            hue = (color.g()-color.b())*deltaInv60 + (color.g() < color.b() ? T(360) : T(0));
        else if(max == color.g())
            hue = (color.b()-color.r())*deltaInv60 + T(120);
        else
            hue = (color.r()-color.g())*deltaInv60 + T(240);
    }

    return Deg<T>(hue);
}

template<class T> inline Deg<T> hue(typename std::enable_if<std::is_floating_point<T>::value, const Color3<T>&>::type color) {
    T max = color.max();
    T delta = max - color.min();
    return hue(color, max, delta);
}
template<class T> inline T saturation(typename std::enable_if<std::is_floating_point<T>::value, const Color3<T>&>::type color) {
    T max = color.max();
    T delta = max - color.min();
    return max != T(0) ? delta/max : T(0);
}
template<class T> inline T value(typename std::enable_if<std::is_floating_point<T>::value, const Color3<T>&>::type color) {
    return color.max();
}

template<class T> inline Deg<typename Color3<T>::FloatingPointType> hue(typename std::enable_if<std::is_integral<T>::value, const Color3<T>&>::type color) {
    return hue<typename Color3<T>::FloatingPointType>(unpack<Color3<typename Color3<T>::FloatingPointType>>(color));
}
template<class T> inline typename Color3<T>::FloatingPointType saturation(typename std::enable_if<std::is_integral<T>::value, const Color3<T>&>::type& color) {
    return saturation<typename Color3<T>::FloatingPointType>(unpack<Color3<typename Color3<T>::FloatingPointType>>(color));
}
template<class T> inline typename Color3<T>::FloatingPointType value(typename std::enable_if<std::is_integral<T>::value, const Color3<T>&>::type color) {
    return unpack<typename Color3<T>::FloatingPointType>(color.max());
}

template<class T> inline ColorHsv<T> toHsv(typename std::enable_if<std::is_floating_point<T>::value, const Color3<T>&>::type color) {
    T max = color.max();
    T delta = max - color.min();

    return ColorHsv<T>{hue<typename Color3<T>::FloatingPointType>(color, max, delta), max != T(0) ? delta/max : T(0), max};
}
template<class T> inline ColorHsv<typename TypeTraits<T>::FloatingPointType> toHsv(typename std::enable_if<std::is_integral<T>::value, const Color3<T>&>::type color) {
    return toHsv<typename TypeTraits<T>::FloatingPointType>(unpack<Color3<typename TypeTraits<T>::FloatingPointType>>(color));
}

template<class T> typename std::enable_if<std::is_floating_point<T>::value, Color3<T>>::type fromSrgb(const Vector3<T>& srgb) {
    constexpr const T a(T(0.055));
    return lerp(srgb/T(12.92), pow((srgb + Vector3<T>{a})/(T(1.0) + a), T(2.4)), srgb > Vector3<T>(T(0.04045)));
}
template<class T> typename std::enable_if<std::is_floating_point<T>::value, Color4<T>>::type fromSrgbAlpha(const Vector4<T>& srgbAlpha) {
    return {fromSrgb<T>(srgbAlpha.rgb()), srgbAlpha.a()};
}
template<class T> inline typename std::enable_if<std::is_integral<T>::value, Color3<T>>::type fromSrgb(const Vector3<typename Color3<T>::FloatingPointType>& srgb) {
    return pack<Color3<T>>(fromSrgb<typename Color3<T>::FloatingPointType>(srgb));
}
template<class T> inline typename std::enable_if<std::is_integral<T>::value, Color4<T>>::type fromSrgbAlpha(const Vector4<typename Color4<T>::FloatingPointType>& srgbAlpha) {
    return {fromSrgb<T>(srgbAlpha.rgb()), pack<T>(srgbAlpha.a())};
}
template<class T, class Integral> inline Color3<T> fromSrgbIntegral(const Vector3<Integral>& srgb) {
    static_assert(std::is_integral<Integral>::value, "only conversion from different integral type is supported");
    return fromSrgb<T>(unpack<Vector3<typename Color3<T>::FloatingPointType>>(srgb));
}
template<class T, class Integral> inline Color4<T> fromSrgbAlphaIntegral(const Vector4<Integral>& srgbAlpha) {
    static_assert(std::is_integral<Integral>::value, "only conversion from different integral type is supported");
    return fromSrgbAlpha<T>(unpack<Vector4<typename Color4<T>::FloatingPointType>>(srgbAlpha));
}

template<class T> Vector3<typename Color3<T>::FloatingPointType> toSrgb(typename std::enable_if<std::is_floating_point<T>::value, const Color3<T>&>::type rgb) {
    constexpr const T a = T(0.055);
    return lerp(rgb*T(12.92), (T(1.0) + a)*pow(rgb, T(1.0)/T(2.4)) - Vector3<T>{a}, rgb > Vector3<T>(T(0.0031308)));
}
template<class T> Vector4<typename Color4<T>::FloatingPointType> toSrgbAlpha(typename std::enable_if<std::is_floating_point<T>::value, const Color4<T>&>::type rgba) {
    return {toSrgb<T>(rgba.rgb()), rgba.a()};
}
template<class T> inline Vector3<typename Color3<T>::FloatingPointType> toSrgb(typename std::enable_if<std::is_integral<T>::value, const Color3<T>&>::type rgb) {
    return toSrgb<typename Color3<T>::FloatingPointType>(unpack<Color3<typename Color3<T>::FloatingPointType>>(rgb));
}
template<class T> inline Vector4<typename Color4<T>::FloatingPointType> toSrgbAlpha(typename std::enable_if<std::is_integral<T>::value, const Color4<T>&>::type rgba) {
    return {toSrgb<T>(rgba.rgb()), unpack<typename Color3<T>::FloatingPointType>(rgba.a())};
}
template<class T, class Integral> inline Vector3<Integral> toSrgbIntegral(const Color3<T>& rgb) {
    static_assert(std::is_integral<Integral>::value, "only conversion from different integral type is supported");
    return pack<Vector3<Integral>>(toSrgb<T>(rgb));
}
template<class T, class Integral> inline Vector4<Integral> toSrgbAlphaIntegral(const Color4<T>& rgba) {
    static_assert(std::is_integral<Integral>::value, "only conversion from different integral type is supported");
    return pack<Vector4<Integral>>(toSrgbAlpha<T>(rgba));
}

template<class T> typename std::enable_if<std::is_floating_point<T>::value, Color3<T>>::type fromXyz(const Vector3<T>& xyz) {
    return Matrix3x3<T>{
        Vector3<T>{T(12831)/T(3959), T(-851781)/T(878810), T(705)/T(12673)},
        Vector3<T>{T(-329)/T(214), T(1648619)/T(878810), T(-2585)/T(12673)},
        Vector3<T>{T(-1974)/T(3959), T(36519)/T(878810), T(705)/T(667)}}*xyz;
}
template<class T> inline typename std::enable_if<std::is_integral<T>::value, Color3<T>>::type fromXyz(const Vector3<typename Color3<T>::FloatingPointType>& xyz) {
    return pack<Color3<T>>(fromXyz<typename Color3<T>::FloatingPointType>(xyz));
}

template<class T> Vector3<typename Color3<T>::FloatingPointType> toXyz(typename std::enable_if<std::is_floating_point<T>::value, const Color3<T>&>::type rgb) {
    return (Matrix3x3<T>{
        Vector3<T>{T(506752)/T(1228815), T(87098)/T(409605), T(7918)/T(409605)},
        Vector3<T>{T(87881)/T(245763), T(175762)/T(245763), T(87881)/T(737289)},
        Vector3<T>{T(12673)/T(70218), T(12673)/T(175545), T(1001167)/T(1053270)}})*rgb;
}
template<class T> inline Vector3<typename Color3<T>::FloatingPointType> toXyz(typename std::enable_if<std::is_integral<T>::value, const Color3<T>&>::type rgb) {
    return toXyz<typename Color3<T>::FloatingPointType>(unpack<Color3<typename Color3<T>::FloatingPointType>>(rgb));
}

#if !defined(CORRADE_MSVC2017_COMPATIBILITY) || defined(CORRADE_MSVC2015_COMPATIBILITY)
template<class T> constexpr typename std::enable_if<std::is_floating_point<T>::value, T>::type fullChannel() {
    return T(1);
}
template<class T> constexpr typename std::enable_if<std::is_integral<T>::value, T>::type fullChannel() {
    return Implementation::bitMax<T>();
}
#else
template<class T> constexpr T fullChannel() { return bitMax<T>(); }
template<> constexpr float fullChannel<float>() { return 1.0f; }
template<> constexpr double fullChannel<double>() { return 1.0; }
template<> constexpr long double fullChannel<long double>() { return 1.0l; }
#endif

}

template<class T> class Color3: public Vector3<T> {
    public:
        typedef typename TypeTraits<T>::FloatingPointType FloatingPointType;

        constexpr static Color3<T> red(T red = Implementation::fullChannel<T>()) {
            return Vector3<T>::xAxis(red);
        }

        constexpr static Color3<T> green(T green = Implementation::fullChannel<T>()) {
            return Vector3<T>::yAxis(green);
        }

        constexpr static Color3<T> blue(T blue = Implementation::fullChannel<T>()) {
            return Vector3<T>::zAxis(blue);
        }

        constexpr static Color3<T> cyan(T red = T(0)) {
            return {red, Implementation::fullChannel<T>(), Implementation::fullChannel<T>()};
        }

        constexpr static Color3<T> magenta(T green = T(0)) {
            return {Implementation::fullChannel<T>(), green, Implementation::fullChannel<T>()};
        }

        constexpr static Color3<T> yellow(T blue = T(0)) {
            return {Implementation::fullChannel<T>(), Implementation::fullChannel<T>(), blue};
        }

        static Color3<T> fromHsv(const ColorHsv<FloatingPointType>& hsv) {
            return Implementation::fromHsv<T>(hsv);
        }

        static Color3<T> fromSrgb(const Vector3<FloatingPointType>& srgb) {
            return Implementation::fromSrgb<T>(srgb);
        }

        template<class Integral> static Color3<T> fromSrgb(const Vector3<Integral>& srgb) {
            return Implementation::fromSrgbIntegral<T, Integral>(srgb);
        }

        static Color3<T> fromSrgb(UnsignedInt srgb) {
            return fromSrgb<UnsignedByte>({UnsignedByte(srgb >> 16),
                                           UnsignedByte(srgb >> 8),
                                           UnsignedByte(srgb)});
        }

        static Color3<T> fromXyz(const Vector3<FloatingPointType>& xyz) {
            return Implementation::fromXyz<T>(xyz);
        }

        constexpr /*implicit*/ Color3() noexcept: Vector3<T>{ZeroInit} {}

        constexpr explicit Color3(ZeroInitT) noexcept: Vector3<T>{ZeroInit} {}

        explicit Color3(NoInitT) noexcept: Vector3<T>{NoInit} {}

        constexpr explicit Color3(T rgb) noexcept: Vector3<T>(rgb) {}

        constexpr /*implicit*/ Color3(T r, T g, T b) noexcept: Vector3<T>(r, g, b) {}

        template<class U> constexpr explicit Color3(const Vector<3, U>& other) noexcept: Vector3<T>(other) {}

        template<class U, class V =
            #ifndef CORRADE_MSVC2015_COMPATIBILITY /* Causes ICE */
            decltype(Implementation::VectorConverter<3, T, U>::from(std::declval<U>()))
            #else
            decltype(Implementation::VectorConverter<3, T, U>())
            #endif
            >
        constexpr explicit Color3(const U& other): Vector3<T>(Implementation::VectorConverter<3, T, U>::from(other)) {}

        constexpr /*implicit*/ Color3(const Vector<3, T>& other) noexcept: Vector3<T>(other) {}

        ColorHsv<FloatingPointType> toHsv() const {
            return Implementation::toHsv<T>(*this);
        }

        Deg<FloatingPointType> hue() const {
            return Deg<FloatingPointType>(Implementation::hue<T>(*this));
        }

        FloatingPointType saturation() const {
            return Implementation::saturation<T>(*this);
        }

        FloatingPointType value() const {
            return Implementation::value<T>(*this);
        }

        Vector3<FloatingPointType> toSrgb() const {
            return Implementation::toSrgb<T>(*this);
        }

        template<class Integral> Vector3<Integral> toSrgb() const {
            return Implementation::toSrgbIntegral<T, Integral>(*this);
        }

        UnsignedInt toSrgbInt() const {
            const auto srgb = toSrgb<UnsignedByte>();
            return (srgb[0] << 16) | (srgb[1] << 8) | srgb[2];
        }

        Vector3<FloatingPointType> toXyz() const {
            return Implementation::toXyz<T>(*this);
        }

        MAGNUM_VECTOR_SUBCLASS_IMPLEMENTATION(3, Color3)
};

MAGNUM_VECTORn_OPERATOR_IMPLEMENTATION(3, Color3)

template<class T>
class Color4: public Vector4<T> {
    public:
        typedef typename Color3<T>::FloatingPointType FloatingPointType;

        constexpr static Color4<T> red(T red = Implementation::fullChannel<T>(), T alpha = Implementation::fullChannel<T>()) {
            return {red, T(0), T(0), alpha};
        }

        constexpr static Color4<T> green(T green = Implementation::fullChannel<T>(), T alpha = Implementation::fullChannel<T>()) {
            return {T(0), green, T(0), alpha};
        }

        constexpr static Color4<T> blue(T blue = Implementation::fullChannel<T>(), T alpha = Implementation::fullChannel<T>()) {
            return {T(0), T(0), blue, alpha};
        }

        constexpr static Color4<T> cyan(T red = T(0), T alpha = Implementation::fullChannel<T>()) {
            return {red, Implementation::fullChannel<T>(), Implementation::fullChannel<T>(), alpha};
        }

        constexpr static Color4<T> magenta(T green = T(0), T alpha = Implementation::fullChannel<T>()) {
            return {Implementation::fullChannel<T>(), green, Implementation::fullChannel<T>(), alpha};
        }

        constexpr static Color4<T> yellow(T blue = T(0), T alpha = Implementation::fullChannel<T>()) {
            return {Implementation::fullChannel<T>(), Implementation::fullChannel<T>(), blue, alpha};
        }

        static Color4<T> fromHsv(const ColorHsv<FloatingPointType>& hsv, T a = Implementation::fullChannel<T>()) {
            return Color4<T>(Implementation::fromHsv<T>(hsv), a);
        }

        static Color4<T> fromSrgbAlpha(const Vector4<FloatingPointType>& srgbAlpha) {
            return {Implementation::fromSrgbAlpha<T>(srgbAlpha)};
        }

        static Color4<T> fromSrgb(const Vector3<FloatingPointType>& srgb, T a = Implementation::fullChannel<T>()) {
            return {Implementation::fromSrgb<T>(srgb), a};
        }

        template<class Integral> static Color4<T> fromSrgbAlpha(const Vector4<Integral>& srgbAlpha) {
            return {Implementation::fromSrgbAlphaIntegral<T, Integral>(srgbAlpha)};
        }

        template<class Integral> static Color4<T> fromSrgb(const Vector3<Integral>& srgb, T a = Implementation::fullChannel<T>()) {
            return {Implementation::fromSrgbIntegral<T, Integral>(srgb), a};
        }

        static Color4<T> fromSrgbAlpha(UnsignedInt srgbAlpha) {
            return fromSrgbAlpha<UnsignedByte>({UnsignedByte(srgbAlpha >> 24),
                                                UnsignedByte(srgbAlpha >> 16),
                                                UnsignedByte(srgbAlpha >> 8),
                                                UnsignedByte(srgbAlpha)});
        }

        static Color4<T> fromSrgb(UnsignedInt srgb, T a = Implementation::fullChannel<T>()) {
            return fromSrgb<UnsignedByte>({UnsignedByte(srgb >> 16),
                                           UnsignedByte(srgb >> 8),
                                           UnsignedByte(srgb)}, a);
        }

        static Color4<T> fromXyz(const Vector3<FloatingPointType> xyz, T a = Implementation::fullChannel<T>()) {
            return {Implementation::fromXyz<T>(xyz), a};
        }

        constexpr /*implicit*/ Color4() noexcept: Vector4<T>{ZeroInit} {}

        constexpr explicit Color4(ZeroInitT) noexcept: Vector4<T>{ZeroInit} {}

        explicit Color4(NoInitT) noexcept: Vector4<T>{NoInit} {}

        constexpr explicit Color4(T rgb, T alpha = Implementation::fullChannel<T>()) noexcept: Vector4<T>(rgb, rgb, rgb, alpha) {}

        constexpr /*implicit*/ Color4(T r, T g, T b, T a = Implementation::fullChannel<T>()) noexcept: Vector4<T>(r, g, b, a) {}

        constexpr /*implicit*/ Color4(const Vector3<T>& rgb, T a = Implementation::fullChannel<T>()) noexcept: Vector4<T>(rgb[0], rgb[1], rgb[2], a) {}

        template<class U> constexpr explicit Color4(const Vector<4, U>& other) noexcept: Vector4<T>(other) {}

        template<class U, class V =
            #ifndef CORRADE_MSVC2015_COMPATIBILITY /* Causes ICE */
            decltype(Implementation::VectorConverter<4, T, U>::from(std::declval<U>()))
            #else
            decltype(Implementation::VectorConverter<4, T, U>())
            #endif
            >
        constexpr explicit Color4(const U& other): Vector4<T>(Implementation::VectorConverter<4, T, U>::from(other)) {}

        constexpr /*implicit*/ Color4(const Vector<4, T>& other) noexcept: Vector4<T>(other) {}

        ColorHsv<FloatingPointType> toHsv() const {
            return Implementation::toHsv<T>(Vector4<T>::rgb());
        }

        Deg<FloatingPointType> hue() const {
            return Implementation::hue<T>(Vector4<T>::rgb());
        }

        FloatingPointType saturation() const {
            return Implementation::saturation<T>(Vector4<T>::rgb());
        }

        FloatingPointType value() const {
            return Implementation::value<T>(Vector4<T>::rgb());
        }

        Vector4<FloatingPointType> toSrgbAlpha() const {
            return Implementation::toSrgbAlpha<T>(*this);
        }

        template<class Integral> Vector4<Integral> toSrgbAlpha() const {
            return Implementation::toSrgbAlphaIntegral<T, Integral>(*this);
        }

        UnsignedInt toSrgbAlphaInt() const {
            const auto srgbAlpha = toSrgbAlpha<UnsignedByte>();
            return (srgbAlpha[0] << 24) | (srgbAlpha[1] << 16) | (srgbAlpha[2] << 8) | srgbAlpha[3];
        }

        Vector3<FloatingPointType> toXyz() const {
            return Implementation::toXyz<T>(rgb());
        }

        Color3<T>& xyz() { return Color3<T>::from(Vector4<T>::data()); }
        constexpr const Color3<T> xyz() const { return Vector4<T>::xyz(); }

        Color3<T>& rgb() { return xyz(); }
        constexpr const Color3<T> rgb() const { return xyz(); }

        MAGNUM_VECTOR_SUBCLASS_IMPLEMENTATION(4, Color4)
};

template<class T> inline Vector3<T> xyYToXyz(const Vector3<T>& xyY) {
    return {xyY[0]*xyY[2]/xyY[1], xyY[2], (T(1) - xyY[0] - xyY[1])*xyY[2]/xyY[1]};
}

template<class T> inline Vector3<T> xyzToXyY(const Vector3<T>& xyz) {
    return {xyz.xy()/xyz.sum(), xyz.y()};
}

template<class T> struct ColorHsv {
    constexpr /*implicit*/ ColorHsv() noexcept: hue{}, saturation{}, value{} {}

    constexpr explicit ColorHsv(ZeroInitT) noexcept: hue{}, saturation{}, value{} {}

    explicit ColorHsv(NoInitT) noexcept: hue{NoInit} /* and the others not */ {}

    constexpr /*implicit*/ ColorHsv(Deg<T> hue, T saturation, T value) noexcept: hue{hue}, saturation{saturation}, value{value} {}

    template<class U> constexpr explicit ColorHsv(const ColorHsv<U>& other) noexcept: hue{other.hue}, saturation{T(other.saturation)}, value{T(other.value)} {}

    bool operator==(const ColorHsv<T>& other) const {
        return hue == other.hue &&
            TypeTraits<T>::equals(saturation, other.saturation) &&
            TypeTraits<T>::equals(value, other.value);
    }

    bool operator!=(const ColorHsv<T>& other) const {
        return !operator==(other);
    }

    Deg<T> hue;

    T saturation;

    T value;
};

MAGNUM_VECTORn_OPERATOR_IMPLEMENTATION(4, Color4)

namespace Literals {

constexpr Color3<UnsignedByte> operator "" _rgb(unsigned long long value) {
    return {UnsignedByte(value >> 16), UnsignedByte(value >> 8), UnsignedByte(value)};
}

constexpr Vector3<UnsignedByte> operator "" _srgb(unsigned long long value) {
    return {UnsignedByte(value >> 16), UnsignedByte(value >> 8), UnsignedByte(value)};
}

constexpr Color4<UnsignedByte> operator "" _rgba(unsigned long long value) {
    return {UnsignedByte(value >> 24), UnsignedByte(value >> 16), UnsignedByte(value >> 8), UnsignedByte(value)};
}

constexpr Vector4<UnsignedByte> operator "" _srgba(unsigned long long value) {
    return {UnsignedByte(value >> 24), UnsignedByte(value >> 16), UnsignedByte(value >> 8), UnsignedByte(value)};
}

inline Color3<Float> operator "" _rgbf(unsigned long long value) {
    return Math::unpack<Color3<Float>>(Color3<UnsignedByte>{UnsignedByte(value >> 16), UnsignedByte(value >> 8), UnsignedByte(value)});
}

inline Color3<Float> operator "" _srgbf(unsigned long long value) {
    return Color3<Float>::fromSrgb(UnsignedInt(value));
}

inline Color4<Float> operator "" _rgbaf(unsigned long long value) {
    return Math::unpack<Color4<Float>>(Color4<UnsignedByte>{UnsignedByte(value >> 24), UnsignedByte(value >> 16), UnsignedByte(value >> 8), UnsignedByte(value)});
}

inline Color4<Float> operator "" _srgbaf(unsigned long long value) {
    return Color4<Float>::fromSrgbAlpha(UnsignedInt(value));
}

}

namespace Implementation {
    template<class T> struct TypeForSize<3, Color3<T>> { typedef Color3<T> Type; };
    template<class T> struct TypeForSize<3, Color4<T>> { typedef Color3<T> Type; };
    template<class T> struct TypeForSize<4, Color3<T>> { typedef Color4<T> Type; };
    template<class T> struct TypeForSize<4, Color4<T>> { typedef Color4<T> Type; };

    template<class T> struct StrictWeakOrdering<Color3<T>>: StrictWeakOrdering<Vector<3, T>> {};
    template<class T> struct StrictWeakOrdering<Color4<T>>: StrictWeakOrdering<Vector<4, T>> {};
}

}}

namespace Corrade { namespace Utility {

}}

#endif
#ifndef Magnum_Math_Complex_h
#define Magnum_Math_Complex_h

namespace Magnum { namespace Math {

namespace Implementation {
    template<class T> constexpr static Complex<T> complexFromMatrix(const Matrix2x2<T>& matrix) {
        return {matrix[0][0], matrix[0][1]};
    }

    template<class, class> struct ComplexConverter;
}

template<class T> inline T dot(const Complex<T>& a, const Complex<T>& b) {
    return a.real()*b.real() + a.imaginary()*b.imaginary();
}

template<class T> inline Rad<T> angle(const Complex<T>& normalizedA, const Complex<T>& normalizedB) {
    CORRADE_ASSERT(normalizedA.isNormalized() && normalizedB.isNormalized(),
        "Math::angle(): complex numbers" << normalizedA << "and" << normalizedB << "are not normalized", {});
    return Rad<T>(std::acos(dot(normalizedA, normalizedB)));
}

template<class T> class Complex {
    public:
        typedef T Type;

        static Complex<T> rotation(Rad<T> angle) {
            return {std::cos(T(angle)), std::sin(T(angle))};
        }

        static Complex<T> fromMatrix(const Matrix2x2<T>& matrix) {
            CORRADE_ASSERT(matrix.isOrthogonal(),
                "Math::Complex::fromMatrix(): the matrix is not orthogonal:" << Corrade::Utility::Debug::newline << matrix, {});
            return Implementation::complexFromMatrix(matrix);
        }

        constexpr /*implicit*/ Complex() noexcept: _real(T(1)), _imaginary(T(0)) {}

        constexpr explicit Complex(IdentityInitT) noexcept: _real(T(1)), _imaginary(T(0)) {}

        constexpr explicit Complex(ZeroInitT) noexcept: _real{}, _imaginary{} {}

        explicit Complex(NoInitT) noexcept {}

        constexpr /*implicit*/ Complex(T real, T imaginary) noexcept: _real(real), _imaginary(imaginary) {}

        constexpr explicit Complex(const Vector2<T>& vector) noexcept: _real(vector.x()), _imaginary(vector.y()) {}

        template<class U> constexpr explicit Complex(const Complex<U>& other) noexcept: _real{T(other._real)}, _imaginary{T(other._imaginary)} {}

        template<class U, class V = decltype(Implementation::ComplexConverter<T, U>::from(std::declval<U>()))> constexpr explicit Complex(const U& other): Complex{Implementation::ComplexConverter<T, U>::from(other)} {}

        constexpr /*implicit*/ Complex(const Complex<T>&) noexcept = default;

        template<class U, class V = decltype(Implementation::ComplexConverter<T, U>::to(std::declval<Complex<T>>()))> constexpr explicit operator U() const {
            return Implementation::ComplexConverter<T, U>::to(*this);
        }

        T* data() { return &_real; }
        constexpr const T* data() const { return &_real; }

        bool operator==(const Complex<T>& other) const {
            return TypeTraits<T>::equals(_real, other._real) &&
                   TypeTraits<T>::equals(_imaginary, other._imaginary);
        }

        bool operator!=(const Complex<T>& other) const {
            return !operator==(other);
        }

        bool isNormalized() const {
            return Implementation::isNormalizedSquared(dot());
        }

        T& real() { return _real; }
        constexpr T real() const { return _real; }

        T& imaginary() { return _imaginary; }
        constexpr T imaginary() const { return _imaginary; }

        constexpr explicit operator Vector2<T>() const {
            return {_real, _imaginary};
        }

        Rad<T> angle() const {
            return Rad<T>(std::atan2(_imaginary, _real));
        }

        Matrix2x2<T> toMatrix() const {
            return {Vector<2, T>(_real, _imaginary),
                    Vector<2, T>(-_imaginary, _real)};
        }

        Complex<T>& operator+=(const Complex<T>& other) {
            _real += other._real;
            _imaginary += other._imaginary;
            return *this;
        }

        Complex<T> operator+(const Complex<T>& other) const {
            return Complex<T>(*this) += other;
        }

        Complex<T> operator-() const {
            return {-_real, -_imaginary};
        }

        Complex<T>& operator-=(const Complex<T>& other) {
            _real -= other._real;
            _imaginary -= other._imaginary;
            return *this;
        }

        Complex<T> operator-(const Complex<T>& other) const {
            return Complex<T>(*this) -= other;
        }

        Complex<T>& operator*=(T scalar) {
            _real *= scalar;
            _imaginary *= scalar;
            return *this;
        }

        Complex<T>& operator*=(const Vector2<T>& vector) {
             _real *= vector.x();
             _imaginary *= vector.y();
             return *this;
        }

        Complex<T> operator*(T scalar) const {
            return Complex<T>(*this) *= scalar;
        }

        Complex<T> operator*(const Vector2<T>& vector) const {
            return Complex<T>(*this) *= vector;
        }

        Complex<T>& operator/=(T scalar) {
            _real /= scalar;
            _imaginary /= scalar;
            return *this;
        }

        Complex<T>& operator/=(const Vector2<T>& vector) {
             _real /= vector.x();
             _imaginary /= vector.y();
             return *this;
        }

        Complex<T> operator/(T scalar) const {
            return Complex<T>(*this) /= scalar;
        }

        Complex<T> operator/(const Vector2<T>& vector) const {
            return Complex<T>(*this) /= vector;
        }

        Complex<T> operator*(const Complex<T>& other) const {
            return {_real*other._real - _imaginary*other._imaginary,
                    _imaginary*other._real + _real*other._imaginary};
        }

        T dot() const { return Math::dot(*this, *this); }

        T length() const { return std::hypot(_real, _imaginary); }

        Complex<T> normalized() const {
            return (*this)/length();
        }

        Complex<T> conjugated() const {
            return {_real, -_imaginary};
        }

        Complex<T> inverted() const {
            return conjugated()/dot();
        }

        Complex<T> invertedNormalized() const {
            CORRADE_ASSERT(isNormalized(),
                "Math::Complex::invertedNormalized():" << *this << "is not normalized", {});
            return conjugated();
        }

        Vector2<T> transformVector(const Vector2<T>& vector) const {
            return Vector2<T>((*this)*Complex<T>(vector));
        }

    private:
        template<class> friend class Complex;

        T _real, _imaginary;
};

template<class T> inline Complex<T> operator*(T scalar, const Complex<T>& complex) {
    return complex*scalar;
}

template<class T> inline Complex<T> operator*(const Vector2<T>& vector, const Complex<T>& complex) {
    return complex*vector;
}

template<class T> inline Complex<T> operator/(T scalar, const Complex<T>& complex) {
    return {scalar/complex.real(), scalar/complex.imaginary()};
}

template<class T> inline Complex<T> operator/(const Vector2<T>& vector, const Complex<T>& complex) {
    return {vector.x()/complex.real(), vector.y()/complex.imaginary()};
}

template<class T> inline Complex<T> lerp(const Complex<T>& normalizedA, const Complex<T>& normalizedB, T t) {
    CORRADE_ASSERT(normalizedA.isNormalized() && normalizedB.isNormalized(),
        "Math::lerp(): complex numbers" << normalizedA << "and" << normalizedB << "are not normalized", {});
    return ((T(1) - t)*normalizedA + t*normalizedB).normalized();
}

template<class T> inline Complex<T> slerp(const Complex<T>& normalizedA, const Complex<T>& normalizedB, T t) {
    CORRADE_ASSERT(normalizedA.isNormalized() && normalizedB.isNormalized(),
        "Math::slerp(): complex numbers" << normalizedA << "and" << normalizedB << "are not normalized", {});
    const T cosAngle = dot(normalizedA, normalizedB);

    if(std::abs(cosAngle) >= T(1)) return Complex<T>{normalizedA};

    const T a = std::acos(cosAngle);
    return (std::sin((T(1) - t)*a)*normalizedA + std::sin(t*a)*normalizedB)/std::sin(a);
}

namespace Implementation {

template<class T> struct StrictWeakOrdering<Complex<T>> {
    bool operator()(const Complex<T>& a, const Complex<T>& b) const {
        if(a.real() < b.real())
            return true;
        if(a.real() > b.real())
            return false;

        return a.imaginary() < b.imaginary();
    }
};

}

}}

#endif
#ifndef Magnum_Math_Quaternion_h
#define Magnum_Math_Quaternion_h

namespace Magnum { namespace Math {

namespace Implementation {
    template<class, class> struct QuaternionConverter;
}

template<class T> inline T dot(const Quaternion<T>& a, const Quaternion<T>& b) {
    return dot(a.vector(), b.vector()) + a.scalar()*b.scalar();
}

namespace Implementation {
    template<class T> inline T angle(const Quaternion<T>& normalizedA, const Quaternion<T>& normalizedB) {
        return std::acos(dot(normalizedA, normalizedB));
    }
}

template<class T> inline Rad<T> angle(const Quaternion<T>& normalizedA, const Quaternion<T>& normalizedB) {
    CORRADE_ASSERT(normalizedA.isNormalized() && normalizedB.isNormalized(),
        "Math::angle(): quaternions" << normalizedA << "and" << normalizedB << "are not normalized", {});
    return Rad<T>{Implementation::angle(normalizedA, normalizedB)};
}

template<class T> inline Quaternion<T> lerp(const Quaternion<T>& normalizedA, const Quaternion<T>& normalizedB, T t) {
    CORRADE_ASSERT(normalizedA.isNormalized() && normalizedB.isNormalized(),
        "Math::lerp(): quaternions" << normalizedA << "and" << normalizedB << "are not normalized", {});
    return ((T(1) - t)*normalizedA + t*normalizedB).normalized();
}

template<class T> inline Quaternion<T> lerpShortestPath(const Quaternion<T>& normalizedA, const Quaternion<T>& normalizedB, T t) {
    return lerp(dot(normalizedA, normalizedB) < T(0) ? -normalizedA : normalizedA, normalizedB, t);
}

template<class T> inline Quaternion<T> slerp(const Quaternion<T>& normalizedA, const Quaternion<T>& normalizedB, T t) {
    CORRADE_ASSERT(normalizedA.isNormalized() && normalizedB.isNormalized(),
        "Math::slerp(): quaternions" << normalizedA << "and" << normalizedB << "are not normalized", {});
    const T cosHalfAngle = dot(normalizedA, normalizedB);

    if(std::abs(cosHalfAngle) >= T(1) - TypeTraits<T>::epsilon())
        return normalizedA;

    const T a = std::acos(cosHalfAngle);
    return (std::sin((T(1) - t)*a)*normalizedA + std::sin(t*a)*normalizedB)/std::sin(a);
}

template<class T> inline Quaternion<T> slerpShortestPath(const Quaternion<T>& normalizedA, const Quaternion<T>& normalizedB, T t) {
    CORRADE_ASSERT(normalizedA.isNormalized() && normalizedB.isNormalized(),
        "Math::slerpShortestPath(): quaternions" << normalizedA << "and" << normalizedB << "are not normalized", {});
    const T cosHalfAngle = dot(normalizedA, normalizedB);

    if(std::abs(cosHalfAngle) >= T(1) - TypeTraits<T>::epsilon())
        return normalizedA;

    const Quaternion<T> shortestNormalizedA = cosHalfAngle < 0 ? -normalizedA : normalizedA;

    const T a = std::acos(std::abs(cosHalfAngle));
    return (std::sin((T(1) - t)*a)*shortestNormalizedA + std::sin(t*a)*normalizedB)/std::sin(a);
}

template<class T> class Quaternion {
    public:
        typedef T Type;

        static Quaternion<T> rotation(Rad<T> angle, const Vector3<T>& normalizedAxis);

        static Quaternion<T> fromMatrix(const Matrix3x3<T>& matrix);

        constexpr /*implicit*/ Quaternion() noexcept: _scalar{T(1)} {}

        constexpr explicit Quaternion(IdentityInitT) noexcept: _scalar{T(1)} {}

        constexpr explicit Quaternion(ZeroInitT) noexcept: _vector{ZeroInit}, _scalar{T{0}} {}

        explicit Quaternion(NoInitT) noexcept: _vector{NoInit} {}

        constexpr /*implicit*/ Quaternion(const Vector3<T>& vector, T scalar) noexcept: _vector(vector), _scalar(scalar) {}

        constexpr explicit Quaternion(const Vector3<T>& vector) noexcept: _vector(vector), _scalar(T(0)) {}

        template<class U> constexpr explicit Quaternion(const Quaternion<U>& other) noexcept: _vector{other._vector}, _scalar{T(other._scalar)} {}

        template<class U, class V = decltype(Implementation::QuaternionConverter<T, U>::from(std::declval<U>()))> constexpr explicit Quaternion(const U& other): Quaternion{Implementation::QuaternionConverter<T, U>::from(other)} {}

        constexpr /*implicit*/ Quaternion(const Quaternion<T>&) noexcept = default;

        template<class U, class V = decltype(Implementation::QuaternionConverter<T, U>::to(std::declval<Quaternion<T>>()))> constexpr explicit operator U() const {
            return Implementation::QuaternionConverter<T, U>::to(*this);
        }

        T* data() { return _vector.data(); }
        constexpr const T* data() const { return _vector.data(); }

        bool operator==(const Quaternion<T>& other) const {
            return _vector == other._vector && TypeTraits<T>::equals(_scalar, other._scalar);
        }

        bool operator!=(const Quaternion<T>& other) const {
            return !operator==(other);
        }

        bool isNormalized() const {
            return Implementation::isNormalizedSquared(dot());
        }

        Vector3<T>& vector() { return _vector; }
        constexpr const Vector3<T> vector() const { return _vector; }

        T& scalar() { return _scalar; }
        constexpr T scalar() const { return _scalar; }

        Rad<T> angle() const;

        Vector3<T> axis() const;

        Matrix3x3<T> toMatrix() const;

        Quaternion<T> operator-() const { return {-_vector, -_scalar}; }

        Quaternion<T>& operator+=(const Quaternion<T>& other) {
            _vector += other._vector;
            _scalar += other._scalar;
            return *this;
        }

        Quaternion<T> operator+(const Quaternion<T>& other) const {
            return Quaternion<T>(*this) += other;
        }

        Quaternion<T>& operator-=(const Quaternion<T>& other) {
            _vector -= other._vector;
            _scalar -= other._scalar;
            return *this;
        }

        Quaternion<T> operator-(const Quaternion<T>& other) const {
            return Quaternion<T>(*this) -= other;
        }

        Quaternion<T>& operator*=(T scalar) {
            _vector *= scalar;
            _scalar *= scalar;
            return *this;
        }

        Quaternion<T> operator*(T scalar) const {
            return Quaternion<T>(*this) *= scalar;
        }

        Quaternion<T>& operator/=(T scalar) {
            _vector /= scalar;
            _scalar /= scalar;
            return *this;
        }

        Quaternion<T> operator/(T scalar) const {
            return Quaternion<T>(*this) /= scalar;
        }

        Quaternion<T> operator*(const Quaternion<T>& other) const;

        T dot() const { return Math::dot(*this, *this); }

        T length() const { return std::sqrt(dot()); }

        Quaternion<T> normalized() const { return (*this)/length(); }

        Quaternion<T> conjugated() const { return {-_vector, _scalar}; }

        Quaternion<T> inverted() const { return conjugated()/dot(); }

        Quaternion<T> invertedNormalized() const;

        Vector3<T> transformVector(const Vector3<T>& vector) const {
            return ((*this)*Quaternion<T>(vector)*inverted()).vector();
        }

        Vector3<T> transformVectorNormalized(const Vector3<T>& vector) const;

    private:
        template<class> friend class Quaternion;

        constexpr static T pow2(T value) {
            return value*value;
        }

        Vector3<T> _vector;
        T _scalar;
};

template<class T> inline Quaternion<T> operator*(T scalar, const Quaternion<T>& quaternion) {
    return quaternion*scalar;
}

template<class T> inline Quaternion<T> operator/(T scalar, const Quaternion<T>& quaternion) {
    return {scalar/quaternion.vector(), scalar/quaternion.scalar()};
}

namespace Implementation {

template<class T> Quaternion<T> quaternionFromMatrix(const Matrix3x3<T>& m) {
    const Vector<3, T> diagonal = m.diagonal();
    const T trace = diagonal.sum();

    if(trace > T(0)) {
        const T s = std::sqrt(trace + T(1));
        const T t = T(0.5)/s;
        return {Vector3<T>(m[1][2] - m[2][1],
                           m[2][0] - m[0][2],
                           m[0][1] - m[1][0])*t, s*T(0.5)};
    }

    std::size_t i = 0;
    if(diagonal[1] > diagonal[0]) i = 1;
    if(diagonal[2] > diagonal[i]) i = 2;

    const std::size_t j = (i + 1) % 3;
    const std::size_t k = (i + 2) % 3;

    const T s = std::sqrt(diagonal[i] - diagonal[j] - diagonal[k] + T(1));
    const T t = (s == T(0) ? T(0) : T(0.5)/s);

    Vector3<T> vec;
    vec[i] = s*T(0.5);
    vec[j] = (m[i][j] + m[j][i])*t;
    vec[k] = (m[i][k] + m[k][i])*t;

    return {vec, (m[j][k] - m[k][j])*t};
}

}

template<class T> inline Quaternion<T> Quaternion<T>::rotation(const Rad<T> angle, const Vector3<T>& normalizedAxis) {
    CORRADE_ASSERT(normalizedAxis.isNormalized(),
        "Math::Quaternion::rotation(): axis" << normalizedAxis << "is not normalized", {});
    return {normalizedAxis*std::sin(T(angle)/2), std::cos(T(angle)/2)};
}

template<class T> inline Quaternion<T> Quaternion<T>::fromMatrix(const Matrix3x3<T>& matrix) {
    CORRADE_ASSERT(matrix.isOrthogonal(),
        "Math::Quaternion::fromMatrix(): the matrix is not orthogonal:" << Corrade::Utility::Debug::newline << matrix, {});
    return Implementation::quaternionFromMatrix(matrix);
}

template<class T> inline Rad<T> Quaternion<T>::angle() const {
    CORRADE_ASSERT(isNormalized(),
        "Math::Quaternion::angle():" << *this << "is not normalized", {});
    return Rad<T>(T(2)*std::acos(_scalar));
}

template<class T> inline Vector3<T> Quaternion<T>::axis() const {
    CORRADE_ASSERT(isNormalized(),
        "Math::Quaternion::axis():" << *this << "is not normalized", {});
    return _vector/std::sqrt(1-pow2(_scalar));
}

template<class T> Matrix3x3<T> Quaternion<T>::toMatrix() const {
    return {
        Vector<3, T>(T(1) - 2*pow2(_vector.y()) - 2*pow2(_vector.z()),
            2*_vector.x()*_vector.y() + 2*_vector.z()*_scalar,
                2*_vector.x()*_vector.z() - 2*_vector.y()*_scalar),
        Vector<3, T>(2*_vector.x()*_vector.y() - 2*_vector.z()*_scalar,
            T(1) - 2*pow2(_vector.x()) - 2*pow2(_vector.z()),
                2*_vector.y()*_vector.z() + 2*_vector.x()*_scalar),
        Vector<3, T>(2*_vector.x()*_vector.z() + 2*_vector.y()*_scalar,
            2*_vector.y()*_vector.z() - 2*_vector.x()*_scalar,
                T(1) - 2*pow2(_vector.x()) - 2*pow2(_vector.y()))
    };
}

template<class T> inline Quaternion<T> Quaternion<T>::operator*(const Quaternion<T>& other) const {
    return {_scalar*other._vector + other._scalar*_vector + Math::cross(_vector, other._vector),
            _scalar*other._scalar - Math::dot(_vector, other._vector)};
}

template<class T> inline Quaternion<T> Quaternion<T>::invertedNormalized() const {
    CORRADE_ASSERT(isNormalized(),
        "Math::Quaternion::invertedNormalized():" << *this << "is not normalized", {});
    return conjugated();
}

template<class T> inline Vector3<T> Quaternion<T>::transformVectorNormalized(const Vector3<T>& vector) const {
    CORRADE_ASSERT(isNormalized(),
        "Math::Quaternion::transformVectorNormalized():" << *this << "is not normalized", {});
    const Vector3<T> t = T(2)*Math::cross(_vector, vector);
    return vector + _scalar*t + Math::cross(_vector, t);
}

namespace Implementation {

template<class T> struct StrictWeakOrdering<Quaternion<T>> {
    bool operator()(const Quaternion<T>& a, const Quaternion<T>& b) const {
        StrictWeakOrdering<Vector3<T>> o;
        if(o(a.vector(), b.vector()))
            return true;
        if(o(b.vector(), a.vector()))
            return false;

        return a.scalar() < b.scalar();
    }
};

}

}}

#endif
#ifndef Magnum_Math_CubicHermiteSpline_h
#define Magnum_Math_CubicHermiteSpline_h

namespace Magnum { namespace Math {

template<class T> class CubicHermite {
    public:
        typedef T Type;

        template<UnsignedInt dimensions, class U> static
        typename std::enable_if<std::is_base_of<Vector<dimensions, U>, T>::value, CubicHermite<T>>::type
        fromBezier(const CubicBezier<dimensions, U>& a, const CubicBezier<dimensions, U>& b) {
            return CORRADE_CONSTEXPR_ASSERT(a[3] == b[0],
                "Math::CubicHermite::fromBezier(): segments are not adjacent"),
                CubicHermite<T>{3*(a[3] - a[2]), a[3], 3*(b[1] - a[3])};
        }

        constexpr /*implicit*/ CubicHermite() noexcept: CubicHermite{typename std::conditional<std::is_constructible<T, IdentityInitT>::value, IdentityInitT, ZeroInitT>::type{typename std::conditional<std::is_constructible<T, IdentityInitT>::value, IdentityInitT, ZeroInitT>::type::Init{}}} {}

        constexpr explicit CubicHermite(ZeroInitT) noexcept: CubicHermite{ZeroInit, typename std::conditional<std::is_constructible<T, ZeroInitT>::value, ZeroInitT*, void*>::type{}} {}

        template<class U = T, class = typename std::enable_if<std::is_constructible<U, IdentityInitT>::value>::type> constexpr explicit CubicHermite(IdentityInitT) noexcept: _inTangent{ZeroInit}, _point{IdentityInit}, _outTangent{ZeroInit} {}

        explicit CubicHermite(NoInitT) noexcept: CubicHermite{NoInit, typename std::conditional<std::is_constructible<T, NoInitT>::value, NoInitT*, void*>::type{}} {}

        constexpr /*implicit*/ CubicHermite(const T& inTangent, const T& point, const T& outTangent) noexcept: _inTangent{inTangent}, _point{point}, _outTangent{outTangent} {}

        template<class U> constexpr explicit CubicHermite(const CubicHermite<U>& other) noexcept: _inTangent{T(other._inTangent)}, _point{T(other._point)}, _outTangent{T(other._outTangent)} {}

        T* data() { return &_inTangent; }
        constexpr const T* data() const { return &_inTangent; }

        bool operator==(const CubicHermite<T>& other) const;

        bool operator!=(const CubicHermite<T>& other) const {
            return !operator==(other);
        }

        T& inTangent() { return _inTangent; }
        constexpr const T& inTangent() const { return _inTangent; }

        T& point() { return _point; }
        constexpr const T& point() const { return _point; }

        T& outTangent() { return _outTangent; }
        constexpr const T& outTangent() const { return _outTangent; }

    private:
        template<class> friend class CubicHermite;

        constexpr explicit CubicHermite(ZeroInitT, ZeroInitT*) noexcept: _inTangent{ZeroInit}, _point{ZeroInit}, _outTangent{ZeroInit} {}
        constexpr explicit CubicHermite(ZeroInitT, void*) noexcept: _inTangent{T(0)}, _point{T(0)}, _outTangent{T(0)} {}

        explicit CubicHermite(NoInitT, NoInitT*) noexcept: _inTangent{NoInit}, _point{NoInit}, _outTangent{NoInit} {}
        explicit CubicHermite(NoInitT, void*) noexcept {}

        T _inTangent;
        T _point;
        T _outTangent;
};

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<class T> using CubicHermite1D = CubicHermite<T>;
#endif

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<class T> using CubicHermite2D = CubicHermite<Vector2<T>>;
#endif

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<class T> using CubicHermite3D = CubicHermite<Vector3<T>>;
#endif

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<class T> using CubicHermiteComplex = CubicHermite<Complex<T>>;
#endif

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<class T> using CubicHermiteQuaternion = CubicHermite<Quaternion<T>>;
#endif

template<class T, class U> T select(const CubicHermite<T>& a, const CubicHermite<T>& b, U t) {
    return t < U(1) ? a.point() : b.point();
}

template<class T, class U> T lerp(const CubicHermite<T>& a, const CubicHermite<T>& b, U t) {
    return Implementation::lerp(a.point(), b.point(), t);
}

template<class T> Complex<T> lerp(const CubicHermiteComplex<T>& a, const CubicHermiteComplex<T>& b, T t) {
    return lerp(a.point(), b.point(), t);
}

template<class T> Quaternion<T> lerp(const CubicHermiteQuaternion<T>& a, const CubicHermiteQuaternion<T>& b, T t) {
    return lerp(a.point(), b.point(), t);
}

template<class T> Quaternion<T> lerpShortestPath(const CubicHermiteQuaternion<T>& a, const CubicHermiteQuaternion<T>& b, T t) {
    return lerpShortestPath(a.point(), b.point(), t);
}

template<class T> inline Complex<T> slerp(const CubicHermiteComplex<T>& a, const CubicHermiteComplex<T>& b, T t) {
    return slerp(a.point(), b.point(), t);
}

template<class T> inline Quaternion<T> slerp(const CubicHermiteQuaternion<T>& a, const CubicHermiteQuaternion<T>& b, T t) {
    return slerp(a.point(), b.point(), t);
}

template<class T> Quaternion<T> slerpShortestPath(const CubicHermiteQuaternion<T>& a, const CubicHermiteQuaternion<T>& b, T t) {
    return slerpShortestPath(a.point(), b.point(), t);
}

template<class T, class U> T splerp(const CubicHermite<T>& a, const CubicHermite<T>& b, U t) {
    return (U(2)*t*t*t - U(3)*t*t + U(1))*a.point() +
        (t*t*t - U(2)*t*t + t)*a.outTangent() +
        (U(-2)*t*t*t + U(3)*t*t)*b.point() +
        (t*t*t - t*t)*b.inTangent();
}

template<class T> Complex<T> splerp(const CubicHermiteComplex<T>& a, const CubicHermiteComplex<T>& b, T t) {
    CORRADE_ASSERT(a.point().isNormalized() && b.point().isNormalized(),
        "Math::splerp(): complex spline points" << a.point() << "and" << b.point() << "are not normalized", {});
    return ((T(2)*t*t*t - T(3)*t*t + T(1))*a.point() +
        (t*t*t - T(2)*t*t + t)*a.outTangent() +
        (T(-2)*t*t*t + T(3)*t*t)*b.point() +
        (t*t*t - t*t)*b.inTangent()).normalized();
}

template<class T> Quaternion<T> splerp(const CubicHermiteQuaternion<T>& a, const CubicHermiteQuaternion<T>& b, T t) {
    CORRADE_ASSERT(a.point().isNormalized() && b.point().isNormalized(),
        "Math::splerp(): quaternion spline points" << a.point() << "and" << b.point() << "are not normalized", {});
    return ((T(2)*t*t*t - T(3)*t*t + T(1))*a.point() +
        (t*t*t - T(2)*t*t + t)*a.outTangent() +
        (T(-2)*t*t*t + T(3)*t*t)*b.point() +
        (t*t*t - t*t)*b.inTangent()).normalized();
}

template<class T> inline bool CubicHermite<T>::operator==(const CubicHermite<T>& other) const {
    return TypeTraits<T>::equals(_inTangent, other._inTangent) &&
        TypeTraits<T>::equals(_point, other._point) &&
        TypeTraits<T>::equals(_outTangent, other._outTangent);
}

namespace Implementation {

template<class T> struct StrictWeakOrdering<CubicHermite<T>> {
    bool operator()(const CubicHermite<T>& a, const CubicHermite<T>& b) const {
        StrictWeakOrdering<T> o;
        if(o(a.inTangent(), b.inTangent()))
            return true;
        if(o(b.inTangent(), a.inTangent()))
            return false;
        if(o(a.point(), b.point()))
            return true;
        if(o(b.point(), a.point()))
            return false;
        return o(a.outTangent(), b.outTangent());
    }
};

}

}}

#endif
#ifndef Magnum_Math_Distance_h
#define Magnum_Math_Distance_h

namespace Magnum { namespace Math { namespace Distance {

template<class T> inline T linePointSquared(const Vector2<T>& a, const Vector2<T>& b, const Vector2<T>& point) {
    const Vector2<T> bMinusA = b - a;
    return Math::pow<2>(cross(bMinusA, a - point))/bMinusA.dot();
}

template<class T> inline T linePoint(const Vector2<T>& a, const Vector2<T>& b, const Vector2<T>& point) {
    const Vector2<T> bMinusA = b - a;
    return std::abs(cross(bMinusA, a - point))/bMinusA.length();
}

template<class T> inline T linePointSquared(const Vector3<T>& a, const Vector3<T>& b, const Vector3<T>& point) {
    return cross(point - a, point - b).dot()/(b - a).dot();
}

template<class T> inline T linePoint(const Vector3<T>& a, const Vector3<T>& b, const Vector3<T>& point) {
    return std::sqrt(linePointSquared(a, b, point));
}

template<class T> T lineSegmentPointSquared(const Vector2<T>& a, const Vector2<T>& b, const Vector2<T>& point);

template<class T> T lineSegmentPoint(const Vector2<T>& a, const Vector2<T>& b, const Vector2<T>& point);

template<class T> T lineSegmentPointSquared(const Vector3<T>& a, const Vector3<T>& b, const Vector3<T>& point);

template<class T> inline T lineSegmentPoint(const Vector3<T>& a, const Vector3<T>& b, const Vector3<T>& point) {
    return std::sqrt(lineSegmentPointSquared(a, b, point));
}

template<class T> inline T pointPlaneScaled(const Vector3<T>& point, const Vector4<T>& plane) {
    return dot(plane.xyz(), point) + plane.w();
}

template<class T> inline T pointPlane(const Vector3<T>& point, const Vector4<T>& plane) {
    return pointPlaneScaled<T>(point, plane)/plane.xyz().length();
}

template<class T> inline T pointPlaneNormalized(const Vector3<T>& point, const Vector4<T>& plane) {
    CORRADE_ASSERT(plane.xyz().isNormalized(),
        "Math::Distance::pointPlaneNormalized(): plane normal" << plane.xyz() << "is not normalized", {});
    return pointPlaneScaled<T>(point, plane);
}

template<class T> T lineSegmentPoint(const Vector2<T>& a, const Vector2<T>& b, const Vector2<T>& point) {
    const Vector2<T> pointMinusA = point - a;
    const Vector2<T> pointMinusB = point - b;
    const Vector2<T> bMinusA = b - a;
    const T pointDistanceA = pointMinusA.dot();
    const T pointDistanceB = pointMinusB.dot();
    const T bDistanceA = bMinusA.dot();

    if(pointDistanceB > bDistanceA + pointDistanceA)
        return std::sqrt(pointDistanceA);

    if(pointDistanceA > bDistanceA + pointDistanceB)
        return std::sqrt(pointDistanceB);

    return std::abs(cross(bMinusA, -pointMinusA))/std::sqrt(bDistanceA);
}

template<class T> T lineSegmentPointSquared(const Vector2<T>& a, const Vector2<T>& b, const Vector2<T>& point) {
    const Vector2<T> pointMinusA = point - a;
    const Vector2<T> pointMinusB = point - b;
    const Vector2<T> bMinusA = b - a;
    const T pointDistanceA = pointMinusA.dot();
    const T pointDistanceB = pointMinusB.dot();
    const T bDistanceA = bMinusA.dot();

    if(pointDistanceB > bDistanceA + pointDistanceA)
        return pointDistanceA;

    if(pointDistanceA > bDistanceA + pointDistanceB)
        return pointDistanceB;

    return Math::pow<2>(cross(bMinusA, -pointMinusA))/bDistanceA;
}

template<class T> T lineSegmentPointSquared(const Vector3<T>& a, const Vector3<T>& b, const Vector3<T>& point) {
    const Vector3<T> pointMinusA = point - a;
    const Vector3<T> pointMinusB = point - b;
    const T pointDistanceA = pointMinusA.dot();
    const T pointDistanceB = pointMinusB.dot();
    const T bDistanceA = (b - a).dot();

    if(pointDistanceB > bDistanceA + pointDistanceA)
        return pointDistanceA;

    if(pointDistanceA > bDistanceA + pointDistanceB)
        return pointDistanceB;

    return cross(pointMinusA, pointMinusB).dot()/bDistanceA;
}

}}}

#endif
#ifndef Corrade_Utility_TypeTraits_h
#define Corrade_Utility_TypeTraits_h

namespace Corrade { namespace Utility {

#define CORRADE_HAS_TYPE(className, typeExpression)                         \
template<class U> class className {                                         \
    template<class T> static char get(T&&, typeExpression* = nullptr);      \
    static short get(...);                                                  \
    public:                                                                 \
        enum: bool { value = sizeof(get(std::declval<U>())) == sizeof(char) }; \
}

namespace Implementation {
    CORRADE_HAS_TYPE(HasMemberBegin, decltype(std::declval<T>().begin()));
    CORRADE_HAS_TYPE(HasMemberEnd, decltype(std::declval<T>().end()));
    CORRADE_HAS_TYPE(HasBegin, decltype(begin(std::declval<T>())));
    CORRADE_HAS_TYPE(HasEnd, decltype(end(std::declval<T>())));
    CORRADE_HAS_TYPE(HasMemberCStr, decltype(std::declval<T>().c_str()));
}

template<class T> using IsIterable = std::integral_constant<bool,
    (Implementation::HasMemberBegin<T>::value || Implementation::HasBegin<T>::value) &&
    (Implementation::HasMemberEnd<T>::value || Implementation::HasEnd<T>::value)
    >;

template<class T> using IsStringLike = std::integral_constant<bool,
    Implementation::HasMemberCStr<T>::value
    >;

}}

#endif
#ifndef Magnum_Math_Dual_h
#define Magnum_Math_Dual_h

namespace Magnum { namespace Math {

namespace Implementation {
    CORRADE_HAS_TYPE(IsDual, decltype(std::declval<const T>().dual()));
}

template<class T> class Dual {
    template<class> friend class Dual;

    public:
        typedef T Type;

        constexpr /*implicit*/ Dual() noexcept: _real{}, _dual{} {}

        template<class U = T, class = typename std::enable_if<std::is_pod<U>::value>::type> constexpr explicit Dual(ZeroInitT) noexcept: _real{}, _dual{} {}
        template<class U = T, class V = T, class = typename std::enable_if<std::is_constructible<U, ZeroInitT>::value>::type> constexpr explicit Dual(ZeroInitT) noexcept: _real{ZeroInit}, _dual{ZeroInit} {}

        template<class U = T, class = typename std::enable_if<std::is_pod<U>::value>::type> explicit Dual(NoInitT) noexcept {}
        template<class U = T, class V = T, class = typename std::enable_if<std::is_constructible<U, NoInitT>::value>::type> explicit Dual(NoInitT) noexcept: _real{NoInit}, _dual{NoInit} {}

        #if !defined(CORRADE_MSVC2017_COMPATIBILITY) || defined(CORRADE_MSVC2015_COMPATIBILITY)
        constexpr /*implicit*/ Dual(const T& real, const T& dual = T()) noexcept: _real(real), _dual(dual) {}
        #else
        constexpr /*implicit*/ Dual(const T& real, const T& dual) noexcept: _real(real), _dual(dual) {}
        constexpr /*implicit*/ Dual(const T& real) noexcept: _real(real), _dual() {}
        #endif

        template<class U> constexpr explicit Dual(const Dual<U>& other) noexcept: _real{T(other._real)}, _dual{T(other._dual)} {}

        constexpr /*implicit*/ Dual(const Dual<T>&) noexcept = default;

        T* data() { return &_real; }
        constexpr const T* data() const { return &_real; }

        bool operator==(const Dual<T>& other) const {
            return TypeTraits<T>::equals(_real, other._real) &&
                   TypeTraits<T>::equals(_dual, other._dual);
        }

        bool operator!=(const Dual<T>& other) const {
            return !operator==(other);
        }

        T& real() { return _real; }
        constexpr const T real() const { return _real; }

        T& dual() { return _dual; }
        constexpr const T dual() const { return _dual; }

        Dual<T>& operator+=(const Dual<T>& other) {
            _real += other._real;
            _dual += other._dual;
            return *this;
        }

        Dual<T> operator+(const Dual<T>& other) const {
            return Dual<T>(*this)+=other;
        }

        Dual<T> operator-() const {
            return {-_real, -_dual};
        }

        Dual<T>& operator-=(const Dual<T>& other) {
            _real -= other._real;
            _dual -= other._dual;
            return *this;
        }

        Dual<T> operator-(const Dual<T>& other) const {
            return Dual<T>(*this)-=other;
        }

        template<class U> auto operator*(const Dual<U>& other) const -> Dual<decltype(std::declval<T>()*std::declval<U>())> {
            return {_real*other._real, _real*other._dual + _dual*other._real};
        }

        template<class U, class V = typename std::enable_if<!Implementation::IsDual<U>::value, void>::type> Dual<decltype(std::declval<T>()*std::declval<U>())> operator*(const U& other) const {
            return {_real*other, _dual*other};
        }

        template<class U> auto operator/(const Dual<U>& other) const -> Dual<decltype(std::declval<T>()/std::declval<U>())> {
            return {_real/other._real, (_dual*other._real - _real*other._dual)/(other._real*other._real)};
        }

        template<class U, class V = typename std::enable_if<!Implementation::IsDual<U>::value, Dual<decltype(std::declval<T>()/std::declval<U>())>>::type> V operator/(const U& other) const {
            return {_real/other, _dual/other};
        }

        Dual<T> conjugated() const {
            return {_real, -_dual};
        }

    private:
        T _real, _dual;
};

template<class T, class U, class V = typename std::enable_if<!Implementation::IsDual<T>::value, Dual<decltype(std::declval<T>()*std::declval<U>())>>::type> inline V operator*(const T& a, const Dual<U>& b) {
    return {a*b.real(), a*b.dual()};
}

#define MAGNUM_DUAL_SUBCLASS_IMPLEMENTATION(Type, Underlying, Multiplicable) \
    Type<T> operator-() const {                                             \
        return Math::Dual<Underlying<T>>::operator-();                      \
    }                                                                       \
    Type<T>& operator+=(const Math::Dual<Underlying<T>>& other) {           \
        Math::Dual<Underlying<T>>::operator+=(other);                       \
        return *this;                                                       \
    }                                                                       \
    Type<T> operator+(const Math::Dual<Underlying<T>>& other) const {       \
        return Math::Dual<Underlying<T>>::operator+(other);                 \
    }                                                                       \
    Type<T>& operator-=(const Math::Dual<Underlying<T>>& other) {           \
        Math::Dual<Underlying<T>>::operator-=(other);                       \
        return *this;                                                       \
    }                                                                       \
    Type<T> operator-(const Math::Dual<Underlying<T>>& other) const {       \
        return Math::Dual<Underlying<T>>::operator-(other);                 \
    }                                                                       \
    Type<T> operator*(const Math::Dual<Multiplicable>& other) const {       \
        return Math::Dual<Underlying<T>>::operator*(other);                 \
    }                                                                       \
    Type<T> operator*(const Multiplicable& other) const {                   \
        return Math::Dual<Underlying<T>>::operator*(other);                 \
    }                                                                       \
    Type<T> operator/(const Math::Dual<Multiplicable>& other) const {       \
        return Math::Dual<Underlying<T>>::operator/(other);                 \
    }                                                                       \
    Type<T> operator/(const Multiplicable& other) const {                   \
        return Math::Dual<Underlying<T>>::operator/(other);                 \
    }

#define MAGNUM_DUAL_SUBCLASS_MULTIPLICATION_IMPLEMENTATION(Type, Underlying) \
    template<class U> Type<T> operator*(const Math::Dual<U>& other) const { \
        return Math::Dual<Underlying<T>>::operator*(other);                 \
    }                                                                       \
    template<class U> Type<T> operator/(const Math::Dual<U>& other) const { \
        return Math::Dual<Underlying<T>>::operator/(other);                 \
    }                                                                       \
    Type<T> operator*(const Math::Dual<Underlying<T>>& other) const {       \
        return Math::Dual<Underlying<T>>::operator*(other);                 \
    }                                                                       \
    Type<T> operator/(const Math::Dual<Underlying<T>>& other) const {       \
        return Math::Dual<Underlying<T>>::operator/(other);                 \
    }

#define MAGNUM_DUAL_OPERATOR_IMPLEMENTATION(Type, Underlying, Multiplicable) \
    template<class T> inline Type<T> operator*(const Math::Dual<Multiplicable>& a, const Type<T>& b) { \
        return a*static_cast<const Math::Dual<Underlying<T>>&>(b);          \
    }                                                                       \
    template<class T> inline Type<T> operator*(const Multiplicable& a, const Type<T>& b) { \
        return a*static_cast<const Math::Dual<Underlying<T>>&>(b);          \
    }                                                                       \
    template<class T> inline Type<T> operator/(const Math::Dual<Multiplicable>& a, const Type<T>& b) { \
        return a/static_cast<const Math::Dual<Underlying<T>>&>(b);          \
    }

template<class T> Dual<T> sqrt(const Dual<T>& dual) {
    T sqrt0 = std::sqrt(dual.real());
    return {sqrt0, dual.dual()/(2*sqrt0)};
}

template<class T> std::pair<Dual<T>, Dual<T>> sincos(const Dual<Rad<T>>& angle)
{
    const T sin = std::sin(T(angle.real()));
    const T cos = std::cos(T(angle.real()));
    return {{sin, T(angle.dual())*cos}, {cos, -T(angle.dual())*sin}};
}
template<class T> std::pair<Dual<T>, Dual<T>> sincos(const Dual<Deg<T>>& angle) { return sincos(Dual<Rad<T>>(angle)); }
template<class T> std::pair<Dual<T>, Dual<T>> sincos(const Dual<Unit<Rad, T>>& angle) { return sincos(Dual<Rad<T>>(angle)); }
template<class T> std::pair<Dual<T>, Dual<T>> sincos(const Dual<Unit<Deg, T>>& angle) { return sincos(Dual<Rad<T>>(angle)); }

namespace Implementation {

template<class T> struct StrictWeakOrdering<Dual<T>> {
    bool operator()(const Dual<T>& a, const Dual<T>& b) const {
        StrictWeakOrdering<T> o;
        if(o(a.real(), b.real()))
            return true;
        if(o(b.real(), a.real()))
            return false;

        return o(a.dual(), b.dual());
    }
};

}

}}

#endif
#ifndef Magnum_Math_Matrix3_h
#define Magnum_Math_Matrix3_h

namespace Magnum { namespace Math {

template<class T> class Matrix3: public Matrix3x3<T> {
    public:
        constexpr static Matrix3<T> translation(const Vector2<T>& vector) {
            return {{      T(1),       T(0), T(0)},
                    {      T(0),       T(1), T(0)},
                    {vector.x(), vector.y(), T(1)}};
        }

        constexpr static Matrix3<T> scaling(const Vector2<T>& vector) {
            return {{vector.x(),       T(0), T(0)},
                    {      T(0), vector.y(), T(0)},
                    {      T(0),       T(0), T(1)}};
        }

        static Matrix3<T> rotation(Rad<T> angle);

        static Matrix3<T> reflection(const Vector2<T>& normal) {
            CORRADE_ASSERT(normal.isNormalized(),
                "Math::Matrix3::reflection(): normal" << normal << "is not normalized", {});
            return from(Matrix2x2<T>() - T(2)*normal*RectangularMatrix<1, 2, T>(normal).transposed(), {});
        }

        constexpr static Matrix3<T> shearingX(T amount) {
            return {{  T(1), T(0), T(0)},
                    {amount, T(1), T(0)},
                    {  T(0), T(0), T(1)}};
        }

        constexpr static Matrix3<T> shearingY(T amount) {
            return {{T(1), amount, T(0)},
                    {T(0),   T(1), T(0)},
                    {T(0),   T(0), T(1)}};
        }

        static Matrix3<T> projection(const Vector2<T>& size) {
            return scaling(2.0f/size);
        }

        constexpr static Matrix3<T> from(const Matrix2x2<T>& rotationScaling, const Vector2<T>& translation) {
            return {{rotationScaling[0], T(0)},
                    {rotationScaling[1], T(0)},
                    {       translation, T(1)}};
        }

        constexpr /*implicit*/ Matrix3() noexcept: Matrix3x3<T>{IdentityInit, T(1)} {}

        constexpr explicit Matrix3(IdentityInitT, T value = T{1}) noexcept: Matrix3x3<T>{IdentityInit, value} {}

        constexpr explicit Matrix3(ZeroInitT) noexcept: Matrix3x3<T>{ZeroInit} {}

        constexpr explicit Matrix3(NoInitT) noexcept: Matrix3x3<T>{NoInit} {}

        constexpr /*implicit*/ Matrix3(const Vector3<T>& first, const Vector3<T>& second, const Vector3<T>& third) noexcept: Matrix3x3<T>(first, second, third) {}

        constexpr explicit Matrix3(T value) noexcept: Matrix3x3<T>{value} {}

        template<class U> constexpr explicit Matrix3(const RectangularMatrix<3, 3, U>& other) noexcept: Matrix3x3<T>(other) {}

        template<class U, class V = decltype(Implementation::RectangularMatrixConverter<3, 3, T, U>::from(std::declval<U>()))> constexpr explicit Matrix3(const U& other) noexcept: Matrix3x3<T>(Implementation::RectangularMatrixConverter<3, 3, T, U>::from(other)) {}

        template<std::size_t otherSize> constexpr explicit Matrix3(const RectangularMatrix<otherSize, otherSize, T>& other) noexcept: Matrix3x3<T>{other} {}

        constexpr /*implicit*/ Matrix3(const RectangularMatrix<3, 3, T>& other) noexcept: Matrix3x3<T>(other) {}

        bool isRigidTransformation() const {
            return rotationScaling().isOrthogonal() && row(2) == Vector3<T>(T(0), T(0), T(1));
        }

        constexpr Matrix2x2<T> rotationScaling() const {
            return {(*this)[0].xy(),
                    (*this)[1].xy()};
        }

        Matrix2x2<T> rotationShear() const {
            return {(*this)[0].xy().normalized(),
                    (*this)[1].xy().normalized()};
        }

        Matrix2x2<T> rotation() const;

        Matrix2x2<T> rotationNormalized() const;

        Vector2<T> scalingSquared() const {
            return {(*this)[0].xy().dot(),
                    (*this)[1].xy().dot()};
        }

        Vector2<T> scaling() const {
            return {(*this)[0].xy().length(),
                    (*this)[1].xy().length()};
        }

        T uniformScalingSquared() const;

        T uniformScaling() const { return std::sqrt(uniformScalingSquared()); }

        Vector2<T>& right() { return (*this)[0].xy(); }
        constexpr Vector2<T> right() const { return (*this)[0].xy(); }

        Vector2<T>& up() { return (*this)[1].xy(); }
        constexpr Vector2<T> up() const { return (*this)[1].xy(); }

        Vector2<T>& translation() { return (*this)[2].xy(); }
        constexpr Vector2<T> translation() const { return (*this)[2].xy(); }

        Matrix3<T> invertedRigid() const;

        Vector2<T> transformVector(const Vector2<T>& vector) const {
            return ((*this)*Vector3<T>(vector, T(0))).xy();
        }

        Vector2<T> transformPoint(const Vector2<T>& vector) const {
            return ((*this)*Vector3<T>(vector, T(1))).xy();
        }

        MAGNUM_RECTANGULARMATRIX_SUBCLASS_IMPLEMENTATION(3, 3, Matrix3<T>)
        MAGNUM_MATRIX_SUBCLASS_IMPLEMENTATION(3, Matrix3, Vector3)
};

MAGNUM_MATRIXn_OPERATOR_IMPLEMENTATION(3, Matrix3)

template<class T> Matrix3<T> Matrix3<T>::rotation(const Rad<T> angle) {
    const T sine = std::sin(T(angle));
    const T cosine = std::cos(T(angle));

    return {{ cosine,   sine, T(0)},
            {  -sine, cosine, T(0)},
            {   T(0),   T(0), T(1)}};
}

template<class T> Matrix2x2<T> Matrix3<T>::rotation() const {
    Matrix2x2<T> rotation{(*this)[0].xy().normalized(),
                          (*this)[1].xy().normalized()};
    CORRADE_ASSERT(rotation.isOrthogonal(),
        "Math::Matrix3::rotation(): the normalized rotation part is not orthogonal:" << Corrade::Utility::Debug::newline << rotation, {});
    return rotation;
}

template<class T> Matrix2x2<T> Matrix3<T>::rotationNormalized() const {
    Matrix2x2<T> rotation{(*this)[0].xy(),
                          (*this)[1].xy()};
    CORRADE_ASSERT(rotation.isOrthogonal(),
        "Math::Matrix3::rotationNormalized(): the rotation part is not orthogonal:" << Corrade::Utility::Debug::newline << rotation, {});
    return rotation;
}

template<class T> T Matrix3<T>::uniformScalingSquared() const {
    const T scalingSquared = (*this)[0].xy().dot();
    CORRADE_ASSERT(TypeTraits<T>::equals((*this)[1].xy().dot(), scalingSquared),
        "Math::Matrix3::uniformScaling(): the matrix doesn't have uniform scaling:" << Corrade::Utility::Debug::newline << rotationScaling(), {});
    return scalingSquared;
}

template<class T> inline Matrix3<T> Matrix3<T>::invertedRigid() const {
    CORRADE_ASSERT(isRigidTransformation(),
        "Math::Matrix3::invertedRigid(): the matrix doesn't represent a rigid transformation:" << Corrade::Utility::Debug::newline << *this, {});

    Matrix2x2<T> inverseRotation = rotationScaling().transposed();
    return from(inverseRotation, inverseRotation*-translation());
}

namespace Implementation {
    template<class T> struct StrictWeakOrdering<Matrix3<T>>: StrictWeakOrdering<RectangularMatrix<3, 3, T>> {};
}

}}

#endif
#ifndef Magnum_Math_DualComplex_h
#define Magnum_Math_DualComplex_h

namespace Magnum { namespace Math {

namespace Implementation {
    template<class, class> struct DualComplexConverter;
}

template<class T> class DualComplex: public Dual<Complex<T>> {
    public:
        typedef T Type;

        static DualComplex<T> rotation(Rad<T> angle) {
            return {Complex<T>::rotation(angle), {{}, {}}};
        }

        static DualComplex<T> translation(const Vector2<T>& vector) {
            return {{}, {vector.x(), vector.y()}};
        }

        static DualComplex<T> fromMatrix(const Matrix3<T>& matrix) {
            CORRADE_ASSERT(matrix.isRigidTransformation(),
                "Math::DualComplex::fromMatrix(): the matrix doesn't represent rigid transformation:" << Corrade::Utility::Debug::newline << matrix, {});
            return {Implementation::complexFromMatrix(matrix.rotationScaling()), Complex<T>(matrix.translation())};
        }

        constexpr /*implicit*/ DualComplex() noexcept: Dual<Complex<T>>({}, {T(0), T(0)}) {}

        constexpr explicit DualComplex(IdentityInitT) noexcept: Dual<Complex<T>>({}, {T(0), T(0)}) {}

        constexpr explicit DualComplex(ZeroInitT) noexcept: Dual<Complex<T>>{Complex<T>{ZeroInit}, Complex<T>{ZeroInit}} {}

        explicit DualComplex(NoInitT) noexcept: Dual<Complex<T>>{NoInit} {}

        constexpr /*implicit*/ DualComplex(const Complex<T>& real, const Complex<T>& dual = Complex<T>(T(0), T(0))) noexcept: Dual<Complex<T>>(real, dual) {}

        constexpr explicit DualComplex(const Vector2<T>& vector) noexcept: Dual<Complex<T>>({}, Complex<T>(vector)) {}

        template<class U> constexpr explicit DualComplex(const DualComplex<U>& other) noexcept: Dual<Complex<T>>{other} {}

        template<class U, class V = decltype(Implementation::DualComplexConverter<T, U>::from(std::declval<U>()))> constexpr explicit DualComplex(const U& other): DualComplex{Implementation::DualComplexConverter<T, U>::from(other)} {}

        constexpr /*implicit*/ DualComplex(const Dual<Complex<T>>& other) noexcept: Dual<Complex<T>>(other) {}

        template<class U, class V = decltype(Implementation::DualComplexConverter<T, U>::to(std::declval<DualComplex<T>>()))> constexpr explicit operator U() const {
            return Implementation::DualComplexConverter<T, U>::to(*this);
        }

        T* data() { return Dual<Complex<T>>::data()->data(); }
        constexpr const T* data() const { return Dual<Complex<T>>::data()->data(); }

        bool isNormalized() const {
            return Implementation::isNormalizedSquared(lengthSquared());
        }

        constexpr Complex<T> rotation() const {
            return Dual<Complex<T>>::real();
        }

        Vector2<T> translation() const {
            return Vector2<T>(Dual<Complex<T>>::dual());
        }

        Matrix3<T> toMatrix() const {
            return Matrix3<T>::from(Dual<Complex<T>>::real().toMatrix(), translation());
        }

        DualComplex<T> operator*(const DualComplex<T>& other) const {
            return {Dual<Complex<T>>::real()*other.real(), Dual<Complex<T>>::real()*other.dual() + Dual<Complex<T>>::dual()};
        }

        DualComplex<T> complexConjugated() const {
            return {Dual<Complex<T>>::real().conjugated(), Dual<Complex<T>>::dual().conjugated()};
        }

        DualComplex<T> dualConjugated() const {
            return Dual<Complex<T>>::conjugated();
        }

        DualComplex<T> conjugated() const {
            return {Dual<Complex<T>>::real().conjugated(), {-Dual<Complex<T>>::dual().real(), Dual<Complex<T>>::dual().imaginary()}};
        }

        T lengthSquared() const {
            return Dual<Complex<T>>::real().dot();
        }

        T length() const {
            return Dual<Complex<T>>::real().length();
        }

        DualComplex<T> normalized() const {
            return {Dual<Complex<T>>::real()/length(), Dual<Complex<T>>::dual()};
        }

        DualComplex<T> inverted() const {
            return DualComplex<T>(Dual<Complex<T>>::real().inverted(), {{}, {}})*DualComplex<T>({}, -Dual<Complex<T>>::dual());
        }

        DualComplex<T> invertedNormalized() const {
            return DualComplex<T>(Dual<Complex<T>>::real().invertedNormalized(), {{}, {}})*DualComplex<T>({}, -Dual<Complex<T>>::dual());
        }

        Vector2<T> transformPoint(const Vector2<T>& vector) const {
            return Vector2<T>(((*this)*DualComplex<T>(vector)).dual());
        }

        MAGNUM_DUAL_SUBCLASS_IMPLEMENTATION(DualComplex, Vector2, T)
};

MAGNUM_DUAL_OPERATOR_IMPLEMENTATION(DualComplex, Vector2, T)

namespace Implementation {
    template<class T> struct StrictWeakOrdering<DualComplex<T>>: StrictWeakOrdering<Dual<Complex<T>>> {};
}

}}

#endif
#ifndef Magnum_Math_Matrix4_h
#define Magnum_Math_Matrix4_h

#ifdef CORRADE_TARGET_WINDOWS /* I so HATE windef.h */
#undef near
#undef far
#endif

namespace Magnum { namespace Math {

template<class T> class Matrix4: public Matrix4x4<T> {
    public:
        constexpr static Matrix4<T> translation(const Vector3<T>& vector) {
            return {{      T(1),       T(0),       T(0), T(0)},
                    {      T(0),       T(1),       T(0), T(0)},
                    {      T(0),       T(0),       T(1), T(0)},
                    {vector.x(), vector.y(), vector.z(), T(1)}};
        }

        constexpr static Matrix4<T> scaling(const Vector3<T>& vector) {
            return {{vector.x(),       T(0),       T(0), T(0)},
                    {      T(0), vector.y(),       T(0), T(0)},
                    {      T(0),       T(0), vector.z(), T(0)},
                    {      T(0),       T(0),       T(0), T(1)}};
        }

        static Matrix4<T> rotation(Rad<T> angle, const Vector3<T>& normalizedAxis);

        static Matrix4<T> rotationX(Rad<T> angle);

        static Matrix4<T> rotationY(Rad<T> angle);

        static Matrix4<T> rotationZ(Rad<T> angle);

        static Matrix4<T> reflection(const Vector3<T>& normal);

        constexpr static Matrix4<T> shearingXY(T amountX, T amountY) {
            return {{    (1),    T(0), T(0), T(0)},
                    {    (0),    T(1), T(0), T(0)},
                    {amountX, amountY, T(1), T(0)},
                    {    (0),    T(0), T(0), T(1)}};
        }

        constexpr static Matrix4<T> shearingXZ(T amountX, T amountZ) {
            return {{   T(1), T(0),    T(0), T(0)},
                    {amountX, T(1), amountZ, T(0)},
                    {   T(0), T(0),    T(1), T(0)},
                    {   T(0), T(0),    T(0), T(1)}};
        }

        constexpr static Matrix4<T> shearingYZ(T amountY, T amountZ) {
            return {{T(1), amountY, amountZ, T(0)},
                    {T(0),    T(1),    T(0), T(0)},
                    {T(0),    T(0),    T(1), T(0)},
                    {T(0),    T(0),    T(0), T(1)}};
        }

        static Matrix4<T> orthographicProjection(const Vector2<T>& size, T near, T far);

        static Matrix4<T> perspectiveProjection(const Vector2<T>& size, T near, T far);

        static Matrix4<T> perspectiveProjection(Rad<T> fov, T aspectRatio, T near, T far) {
            return perspectiveProjection(T(2)*near*std::tan(T(fov)*T(0.5))*Vector2<T>::yScale(T(1)/aspectRatio), near, far);
        }

        static Matrix4<T> perspectiveProjection(const Vector2<T>& bottomLeft, const Vector2<T>& topRight, T near, T far);

        static Matrix4<T> lookAt(const Vector3<T>& eye, const Vector3<T>& target, const Vector3<T>& up);

        constexpr static Matrix4<T> from(const Matrix3x3<T>& rotationScaling, const Vector3<T>& translation) {
            return {{rotationScaling[0], T(0)},
                    {rotationScaling[1], T(0)},
                    {rotationScaling[2], T(0)},
                    {       translation, T(1)}};
        }

        constexpr /*implicit*/ Matrix4() noexcept: Matrix4x4<T>{IdentityInit, T(1)} {}

        constexpr explicit Matrix4(IdentityInitT, T value = T{1}) noexcept: Matrix4x4<T>{IdentityInit, value} {}

        constexpr explicit Matrix4(ZeroInitT) noexcept: Matrix4x4<T>{ZeroInit} {}

        constexpr explicit Matrix4(NoInitT) noexcept: Matrix4x4<T>{NoInit} {}

        constexpr /*implicit*/ Matrix4(const Vector4<T>& first, const Vector4<T>& second, const Vector4<T>& third, const Vector4<T>& fourth) noexcept: Matrix4x4<T>(first, second, third, fourth) {}

        constexpr explicit Matrix4(T value) noexcept: Matrix4x4<T>{value} {}

        template<class U> constexpr explicit Matrix4(const RectangularMatrix<4, 4, U>& other) noexcept: Matrix4x4<T>(other) {}

        template<class U, class V = decltype(Implementation::RectangularMatrixConverter<4, 4, T, U>::from(std::declval<U>()))> constexpr explicit Matrix4(const U& other): Matrix4x4<T>(Implementation::RectangularMatrixConverter<4, 4, T, U>::from(other)) {}

        template<std::size_t otherSize> constexpr explicit Matrix4(const RectangularMatrix<otherSize, otherSize, T>& other) noexcept: Matrix4x4<T>{other} {}

        constexpr /*implicit*/ Matrix4(const RectangularMatrix<4, 4, T>& other) noexcept: Matrix4x4<T>(other) {}

        bool isRigidTransformation() const {
            return rotationScaling().isOrthogonal() && row(3) == Vector4<T>(T(0), T(0), T(0), T(1));
        }

        constexpr Matrix3x3<T> rotationScaling() const {
            return {(*this)[0].xyz(),
                    (*this)[1].xyz(),
                    (*this)[2].xyz()};
        }

        Matrix3x3<T> rotationShear() const {
            return {(*this)[0].xyz().normalized(),
                    (*this)[1].xyz().normalized(),
                    (*this)[2].xyz().normalized()};
        }

        Matrix3x3<T> rotation() const;

        Matrix3x3<T> rotationNormalized() const;

        Vector3<T> scalingSquared() const {
            return {(*this)[0].xyz().dot(),
                    (*this)[1].xyz().dot(),
                    (*this)[2].xyz().dot()};
        }

        Vector3<T> scaling() const {
            return {(*this)[0].xyz().length(),
                    (*this)[1].xyz().length(),
                    (*this)[2].xyz().length()};
        }

        T uniformScalingSquared() const;

        T uniformScaling() const { return std::sqrt(uniformScalingSquared()); }

        Matrix3x3<T> normalMatrix() const {
            return Matrix3x3<T>{(*this)[0].xyz(),
                                (*this)[1].xyz(),
                                (*this)[2].xyz()}.comatrix();
        }

        Vector3<T>& right() { return (*this)[0].xyz(); }
        constexpr Vector3<T> right() const { return (*this)[0].xyz(); }

        Vector3<T>& up() { return (*this)[1].xyz(); }
        constexpr Vector3<T> up() const { return (*this)[1].xyz(); }

        Vector3<T>& backward() { return (*this)[2].xyz(); }
        constexpr Vector3<T> backward() const { return (*this)[2].xyz(); }

        Vector3<T>& translation() { return (*this)[3].xyz(); }
        constexpr Vector3<T> translation() const { return (*this)[3].xyz(); }

        Matrix4<T> invertedRigid() const;

        Vector3<T> transformVector(const Vector3<T>& vector) const {
            return ((*this)*Vector4<T>(vector, T(0))).xyz();
        }

        Vector3<T> transformPoint(const Vector3<T>& vector) const {
            const Vector4<T> transformed{(*this)*Vector4<T>(vector, T(1))};
            return transformed.xyz()/transformed.w();
        }

        MAGNUM_RECTANGULARMATRIX_SUBCLASS_IMPLEMENTATION(4, 4, Matrix4<T>)
        MAGNUM_MATRIX_SUBCLASS_IMPLEMENTATION(4, Matrix4, Vector4)
};

MAGNUM_MATRIXn_OPERATOR_IMPLEMENTATION(4, Matrix4)

template<class T> Matrix4<T> Matrix4<T>::rotation(const Rad<T> angle, const Vector3<T>& normalizedAxis) {
    CORRADE_ASSERT(normalizedAxis.isNormalized(),
        "Math::Matrix4::rotation(): axis" << normalizedAxis << "is not normalized", {});

    const T sine = std::sin(T(angle));
    const T cosine = std::cos(T(angle));
    const T oneMinusCosine = T(1) - cosine;

    const T xx = normalizedAxis.x()*normalizedAxis.x();
    const T xy = normalizedAxis.x()*normalizedAxis.y();
    const T xz = normalizedAxis.x()*normalizedAxis.z();
    const T yy = normalizedAxis.y()*normalizedAxis.y();
    const T yz = normalizedAxis.y()*normalizedAxis.z();
    const T zz = normalizedAxis.z()*normalizedAxis.z();

    return {
        {cosine + xx*oneMinusCosine,
            xy*oneMinusCosine + normalizedAxis.z()*sine,
                xz*oneMinusCosine - normalizedAxis.y()*sine,
                   T(0)},
        {xy*oneMinusCosine - normalizedAxis.z()*sine,
            cosine + yy*oneMinusCosine,
                yz*oneMinusCosine + normalizedAxis.x()*sine,
                   T(0)},
        {xz*oneMinusCosine + normalizedAxis.y()*sine,
            yz*oneMinusCosine - normalizedAxis.x()*sine,
                cosine + zz*oneMinusCosine,
                   T(0)},
        {T(0), T(0), T(0), T(1)}
    };
}

template<class T> Matrix4<T> Matrix4<T>::rotationX(const Rad<T> angle) {
    const T sine = std::sin(T(angle));
    const T cosine = std::cos(T(angle));

    return {{T(1),   T(0),   T(0), T(0)},
            {T(0), cosine,   sine, T(0)},
            {T(0),  -sine, cosine, T(0)},
            {T(0),   T(0),   T(0), T(1)}};
}

template<class T> Matrix4<T> Matrix4<T>::rotationY(const Rad<T> angle) {
    const T sine = std::sin(T(angle));
    const T cosine = std::cos(T(angle));

    return {{cosine, T(0),  -sine, T(0)},
            {  T(0), T(1),   T(0), T(0)},
            {  sine, T(0), cosine, T(0)},
            {  T(0), T(0),   T(0), T(1)}};
}

template<class T> Matrix4<T> Matrix4<T>::rotationZ(const Rad<T> angle) {
    const T sine = std::sin(T(angle));
    const T cosine = std::cos(T(angle));

    return {{cosine,   sine, T(0), T(0)},
            { -sine, cosine, T(0), T(0)},
            {  T(0),   T(0), T(1), T(0)},
            {  T(0),   T(0), T(0), T(1)}};
}

template<class T> Matrix4<T> Matrix4<T>::reflection(const Vector3<T>& normal) {
    CORRADE_ASSERT(normal.isNormalized(),
        "Math::Matrix4::reflection(): normal" << normal << "is not normalized", {});
    return from(Matrix3x3<T>() - T(2)*normal*RectangularMatrix<1, 3, T>(normal).transposed(), {});
}

template<class T> Matrix4<T> Matrix4<T>::orthographicProjection(const Vector2<T>& size, const T near, const T far) {
    const Vector2<T> xyScale = T(2.0)/size;
    const T zScale = T(2.0)/(near-far);

    return {{xyScale.x(),        T(0),             T(0), T(0)},
            {       T(0), xyScale.y(),             T(0), T(0)},
            {       T(0),        T(0),           zScale, T(0)},
            {       T(0),        T(0), near*zScale-T(1), T(1)}};
}

template<class T> Matrix4<T> Matrix4<T>::perspectiveProjection(const Vector2<T>& size, const T near, const T far) {
    const Vector2<T> xyScale = 2*near/size;

    T m22, m32;
    if(far == Constants<T>::inf()) {
        m22 = T(-1);
        m32 = T(-2)*near;
    } else {
        const T zScale = T(1.0)/(near-far);
        m22 = (far+near)*zScale;
        m32 = T(2)*far*near*zScale;
    }

    return {{xyScale.x(),        T(0), T(0),  T(0)},
            {       T(0), xyScale.y(), T(0),  T(0)},
            {       T(0),        T(0), m22,  T(-1)},
            {       T(0),        T(0), m32,  T(0)}};
}

template<class T> Matrix4<T> Matrix4<T>::perspectiveProjection(const Vector2<T>& bottomLeft, const Vector2<T>& topRight, const T near, const T far) {
    const Vector2<T> xyDifference = topRight - bottomLeft;
    const Vector2<T> xyScale = 2*near/xyDifference;
    const Vector2<T> xyOffset = (topRight + bottomLeft)/xyDifference;

    T m22, m32;
    if(far == Constants<T>::inf()) {
        m22 = T(-1);
        m32 = T(-2)*near;
    } else {
        const T zScale = T(1.0)/(near-far);
        m22 = (far+near)*zScale;
        m32 = T(2)*far*near*zScale;
    }

    return {{ xyScale.x(),         T(0), T(0),  T(0)},
            {        T(0),  xyScale.y(), T(0),  T(0)},
            {xyOffset.x(), xyOffset.y(), m22,  T(-1)},
            {        T(0),         T(0), m32,  T(0)}};
}

template<class T> Matrix4<T> Matrix4<T>::lookAt(const Vector3<T>& eye, const Vector3<T>& target, const Vector3<T>& up) {
    const Vector3<T> backward = (eye - target).normalized();
    const Vector3<T> right = cross(up, backward).normalized();
    const Vector3<T> realUp = cross(backward, right);
    return from({right, realUp, backward}, eye);
}

template<class T> Matrix3x3<T> Matrix4<T>::rotation() const {
    Matrix3x3<T> rotation{(*this)[0].xyz().normalized(),
                          (*this)[1].xyz().normalized(),
                          (*this)[2].xyz().normalized()};
    CORRADE_ASSERT(rotation.isOrthogonal(),
        "Math::Matrix4::rotation(): the normalized rotation part is not orthogonal:" << Corrade::Utility::Debug::newline << rotation, {});
    return rotation;
}

template<class T> Matrix3x3<T> Matrix4<T>::rotationNormalized() const {
    Matrix3x3<T> rotation{(*this)[0].xyz(),
                          (*this)[1].xyz(),
                          (*this)[2].xyz()};
    CORRADE_ASSERT(rotation.isOrthogonal(),
        "Math::Matrix4::rotationNormalized(): the rotation part is not orthogonal:" << Corrade::Utility::Debug::newline << rotation, {});
    return rotation;
}

template<class T> T Matrix4<T>::uniformScalingSquared() const {
    const T scalingSquared = (*this)[0].xyz().dot();
    CORRADE_ASSERT(TypeTraits<T>::equals((*this)[1].xyz().dot(), scalingSquared) &&
                   TypeTraits<T>::equals((*this)[2].xyz().dot(), scalingSquared),
        "Math::Matrix4::uniformScaling(): the matrix doesn't have uniform scaling:" << Corrade::Utility::Debug::newline << rotationScaling(), {});
    return scalingSquared;
}

template<class T> Matrix4<T> Matrix4<T>::invertedRigid() const {
    CORRADE_ASSERT(isRigidTransformation(),
        "Math::Matrix4::invertedRigid(): the matrix doesn't represent a rigid transformation:" << Corrade::Utility::Debug::newline << *this, {});

    Matrix3x3<T> inverseRotation = rotationScaling().transposed();
    return from(inverseRotation, inverseRotation*-translation());
}

namespace Implementation {
    template<class T> struct StrictWeakOrdering<Matrix4<T>>: StrictWeakOrdering<RectangularMatrix<4, 4, T>> {};
}

}}

#endif
#ifndef Magnum_Math_DualQuaternion_h
#define Magnum_Math_DualQuaternion_h

namespace Magnum { namespace Math {

namespace Implementation {
    template<class, class> struct DualQuaternionConverter;
}

template<class T> inline DualQuaternion<T> sclerp(const DualQuaternion<T>& normalizedA, const DualQuaternion<T>& normalizedB, const T t) {
    CORRADE_ASSERT(normalizedA.isNormalized() && normalizedB.isNormalized(),
        "Math::sclerp(): dual quaternions" << normalizedA << "and" << normalizedB << "are not normalized", {});
    const T cosHalfAngle = dot(normalizedA.real(), normalizedB.real());

    if(std::abs(cosHalfAngle) >= T(1) - TypeTraits<T>::epsilon())
        return DualQuaternion<T>::translation(Implementation::lerp(normalizedA.translation(), normalizedB.translation(), t))*DualQuaternion<T>{normalizedA.real()};

    const DualQuaternion<T> diff = normalizedA.quaternionConjugated()*normalizedB;
    const Quaternion<T>& l = diff.real();
    const Quaternion<T>& m = diff.dual();

    const T invr = l.vector().lengthInverted();
    const Dual<T> aHalf{std::acos(l.scalar()), -m.scalar()*invr};

    const Vector3<T> direction = l.vector()*invr;
    const Vector3<T> moment = (m.vector() - direction*(aHalf.dual()*l.scalar()))*invr;
    const Dual<Vector3<T>> n{direction, moment};

    const std::pair<Dual<T>, Dual<T>> sincos = Math::sincos(t*Dual<Rad<T>>(aHalf));
    return normalizedA*DualQuaternion<T>{n*sincos.first, sincos.second};
}

template<class T> inline DualQuaternion<T> sclerpShortestPath(const DualQuaternion<T>& normalizedA, const DualQuaternion<T>& normalizedB, const T t) {
    CORRADE_ASSERT(normalizedA.isNormalized() && normalizedB.isNormalized(),
        "Math::sclerp(): dual quaternions" << normalizedA << "and" << normalizedB << "are not normalized", {});
    const T cosHalfAngle = dot(normalizedA.real(), normalizedB.real());

    if(std::abs(cosHalfAngle) >= T(1) - TypeTraits<T>::epsilon())
        return DualQuaternion<T>::translation(Implementation::lerp(normalizedA.translation(), normalizedB.translation(), t))*DualQuaternion<T>{normalizedA.real()};

    const DualQuaternion<T> diff = normalizedA.quaternionConjugated()*(cosHalfAngle < T(0) ? -normalizedB : normalizedB);
    const Quaternion<T>& l = diff.real();
    const Quaternion<T>& m = diff.dual();

    const T invr = l.vector().lengthInverted();
    const Dual<T> aHalf{std::acos(l.scalar()), -m.scalar()*invr};

    const Vector3<T> direction = l.vector()*invr;
    const Vector3<T> moment = (m.vector() - direction*(aHalf.dual()*l.scalar()))*invr;
    const Dual<Vector3<T>> n{direction, moment};

    const std::pair<Dual<T>, Dual<T>> sincos = Math::sincos(t*Dual<Rad<T>>(aHalf));
    return normalizedA*DualQuaternion<T>{n*sincos.first, sincos.second};
}

template<class T> class DualQuaternion: public Dual<Quaternion<T>> {
    public:
        typedef T Type;

        static DualQuaternion<T> rotation(Rad<T> angle, const Vector3<T>& normalizedAxis) {
            return {Quaternion<T>::rotation(angle, normalizedAxis), {{}, T(0)}};
        }

        static DualQuaternion<T> translation(const Vector3<T>& vector) {
            return {{}, {vector/T(2), T(0)}};
        }

        static DualQuaternion<T> fromMatrix(const Matrix4<T>& matrix) {
            CORRADE_ASSERT(matrix.isRigidTransformation(),
                "Math::DualQuaternion::fromMatrix(): the matrix doesn't represent a rigid transformation:" << Corrade::Utility::Debug::newline << matrix, {});

            Quaternion<T> q = Implementation::quaternionFromMatrix(matrix.rotationScaling());
            return {q, Quaternion<T>(matrix.translation()/2)*q};
        }

        constexpr /*implicit*/ DualQuaternion() noexcept: Dual<Quaternion<T>>{{}, {{}, T(0)}} {}

        constexpr explicit DualQuaternion(IdentityInitT) noexcept: Dual<Quaternion<T>>{{}, {{}, T(0)}} {}

        constexpr explicit DualQuaternion(ZeroInitT) noexcept: Dual<Quaternion<T>>{Quaternion<T>{ZeroInit}, Quaternion<T>{ZeroInit}} {}

        explicit DualQuaternion(NoInitT) noexcept: Dual<Quaternion<T>>{NoInit} {}

        constexpr /*implicit*/ DualQuaternion(const Quaternion<T>& real, const Quaternion<T>& dual = Quaternion<T>({}, T(0))) noexcept: Dual<Quaternion<T>>(real, dual) {}

        constexpr /*implicit*/ DualQuaternion(const Dual<Vector3<T>>& vector, const Dual<T>& scalar) noexcept: Dual<Quaternion<T>>{{vector.real(), scalar.real()}, {vector.dual(), scalar.dual()}} {}

        constexpr explicit DualQuaternion(const Vector3<T>& vector) noexcept: Dual<Quaternion<T>>({}, {vector, T(0)}) {}

        template<class U> constexpr explicit DualQuaternion(const DualQuaternion<U>& other) noexcept: Dual<Quaternion<T>>(other) {}

        template<class U, class V = decltype(Implementation::DualQuaternionConverter<T, U>::from(std::declval<U>()))> constexpr explicit DualQuaternion(const U& other): DualQuaternion{Implementation::DualQuaternionConverter<T, U>::from(other)} {}

        constexpr /*implicit*/ DualQuaternion(const Dual<Quaternion<T>>& other) noexcept: Dual<Quaternion<T>>(other) {}

        template<class U, class V = decltype(Implementation::DualQuaternionConverter<T, U>::to(std::declval<DualQuaternion<T>>()))> constexpr explicit operator U() const {
            return Implementation::DualQuaternionConverter<T, U>::to(*this);
        }

        T* data() { return Dual<Quaternion<T>>::data()->data(); }
        constexpr const T* data() const { return Dual<Quaternion<T>>::data()->data(); }

        bool isNormalized() const {
            Dual<T> a = lengthSquared();
            return Implementation::isNormalizedSquared(a.real()) &&
                   TypeTraits<T>::equalsZero(a.dual(), Math::max(Math::abs(Math::Dual<Quaternion<T>>::dual().vector()).max(), Math::abs(Math::Dual<Quaternion<T>>::dual().scalar())));
        }

        constexpr Quaternion<T> rotation() const {
            return Dual<Quaternion<T>>::real();
        }

        Vector3<T> translation() const {
            return (Dual<Quaternion<T>>::dual()*Dual<Quaternion<T>>::real().conjugated()).vector()*T(2);
        }

        Matrix4<T> toMatrix() const {
            return Matrix4<T>::from(Dual<Quaternion<T>>::real().toMatrix(), translation());
        }

        DualQuaternion<T> quaternionConjugated() const {
            return {Dual<Quaternion<T>>::real().conjugated(), Dual<Quaternion<T>>::dual().conjugated()};
        }

        DualQuaternion<T> dualConjugated() const {
            return Dual<Quaternion<T>>::conjugated();
        }

        DualQuaternion<T> conjugated() const {
            return {Dual<Quaternion<T>>::real().conjugated(), {Dual<Quaternion<T>>::dual().vector(), -Dual<Quaternion<T>>::dual().scalar()}};
        }

        Dual<T> lengthSquared() const {
            return {Dual<Quaternion<T>>::real().dot(), T(2)*dot(Dual<Quaternion<T>>::real(), Dual<Quaternion<T>>::dual())};
        }

        Dual<T> length() const {
            return Math::sqrt(lengthSquared());
        }

        DualQuaternion<T> normalized() const {
            return (*this)/length();
        }

        DualQuaternion<T> inverted() const {
            return quaternionConjugated()/lengthSquared();
        }

        DualQuaternion<T> invertedNormalized() const {
            CORRADE_ASSERT(isNormalized(),
                "Math::DualQuaternion::invertedNormalized():" << *this << "is not normalized", {});
            return quaternionConjugated();
        }

        Vector3<T> transformPoint(const Vector3<T>& vector) const {
            return ((*this)*DualQuaternion<T>(vector)*inverted().dualConjugated()).dual().vector();
        }

        Vector3<T> transformPointNormalized(const Vector3<T>& vector) const {
            CORRADE_ASSERT(isNormalized(),
                "Math::DualQuaternion::transformPointNormalized():" << *this << "is not normalized", {});
            return ((*this)*DualQuaternion<T>(vector)*conjugated()).dual().vector();
        }

        MAGNUM_DUAL_SUBCLASS_IMPLEMENTATION(DualQuaternion, Quaternion, T)
        MAGNUM_DUAL_SUBCLASS_MULTIPLICATION_IMPLEMENTATION(DualQuaternion, Quaternion)
};

MAGNUM_DUAL_OPERATOR_IMPLEMENTATION(DualQuaternion, Quaternion, T)

namespace Implementation {
    template<class T> struct StrictWeakOrdering<DualQuaternion<T>>: StrictWeakOrdering<Dual<Quaternion<T>>> {};
}

}}

#endif
#ifndef Magnum_Math_Frustum_h
#define Magnum_Math_Frustum_h

namespace Magnum { namespace Math {

namespace Implementation {
    template<class, class> struct FrustumConverter;
}

template<class T> class Frustum {
    public:
        static Frustum<T> fromMatrix(const Matrix4<T>& m) {
            return {m.row(3) + m.row(0),
                    m.row(3) - m.row(0),
                    m.row(3) + m.row(1),
                    m.row(3) - m.row(1),
                    m.row(3) + m.row(2),
                    m.row(3) - m.row(2)};
        }

        constexpr /*implicit*/ Frustum() noexcept: Frustum<T>{IdentityInit} {}

        constexpr explicit Frustum(IdentityInitT) noexcept;

        explicit Frustum(NoInitT) noexcept: _data{Vector4<T>{NoInit}, Vector4<T>{NoInit}, Vector4<T>{NoInit}, Vector4<T>{NoInit}, Vector4<T>{NoInit}, Vector4<T>{NoInit}} {}

        constexpr /*implicit*/ Frustum(const Vector4<T>& left, const Vector4<T>& right, const Vector4<T>& bottom, const Vector4<T>& top, const Vector4<T>& near, const Vector4<T>& far) noexcept: _data{left, right, bottom, top, near, far} {}

        template<class U> constexpr explicit Frustum(const Frustum<U>& other) noexcept;

        template<class U, class V = decltype(Implementation::FrustumConverter<T, U>::from(std::declval<U>()))> constexpr explicit Frustum(const U& other) noexcept: Frustum<T>{Implementation::FrustumConverter<T, U>::from(other)} {}

        template<class U, class V = decltype(Implementation::FrustumConverter<T, U>::to(std::declval<Frustum<T>>()))> constexpr explicit operator U() const {
            return Implementation::FrustumConverter<T, U>::to(*this);
        }

        bool operator==(const Frustum<T>& other) const {
            for(std::size_t i = 0; i != 6; ++i)
                if(_data[i] != other._data[i]) return false;

            return true;
        }

        bool operator!=(const Frustum<T>& other) const {
            return !operator==(other);
        }

        T* data() { return _data[0].data(); }
        constexpr const T* data() const { return _data[0].data(); }

        constexpr const Vector4<T>& operator[](std::size_t i) const {
            return CORRADE_CONSTEXPR_ASSERT(i < 6, "Math::Frustum::operator[](): index" << i << "out of range"), _data[i];
        }

        Vector4<T>* begin() { return _data; }
        constexpr const Vector4<T>* begin() const { return _data; }
        constexpr const Vector4<T>* cbegin() const { return _data; }

        Vector4<T>* end() { return _data + 6; }
        constexpr const Vector4<T>* end() const { return _data + 6; }
        constexpr const Vector4<T>* cend() const { return _data + 6; }

        constexpr Vector4<T> left() const { return _data[0]; }

        constexpr Vector4<T> right() const { return _data[1]; }

        constexpr Vector4<T> bottom() const { return _data[2]; }

        constexpr Vector4<T> top() const { return _data[3]; }

        constexpr Vector4<T> near() const { return _data[4]; }

        constexpr Vector4<T> far() const { return _data[5]; }

    private:
        Vector4<T> _data[6];
};

template<class T> constexpr Frustum<T>::Frustum(IdentityInitT) noexcept: _data{
    { 1.0f,  0.0f,  0.0f, 1.0f},
    {-1.0f,  0.0f,  0.0f, 1.0f},
    { 0.0f,  1.0f,  0.0f, 1.0f},
    { 0.0f, -1.0f,  0.0f, 1.0f},
    { 0.0f,  0.0f,  1.0f, 1.0f},
    { 0.0f,  0.0f, -1.0f, 1.0f}} {}

template<class T> template<class U> constexpr Frustum<T>::Frustum(const Frustum<U>& other) noexcept: _data{
    Vector4<T>{other[0]},
    Vector4<T>{other[1]},
    Vector4<T>{other[2]},
    Vector4<T>{other[3]},
    Vector4<T>{other[4]},
    Vector4<T>{other[5]}} {}

namespace Implementation {

template<class T> struct StrictWeakOrdering<Frustum<T>> {
    bool operator()(const Frustum<T>& a, const Frustum<T>& b) const {
        StrictWeakOrdering<Vector<4, T>> o;
        for(std::size_t i = 0; i < 6; ++i) {
            if(o(a[i], b[i]))
                return true;
            if(o(b[i], a[i]))
                return false;
        }

        return false;
    }
};

}

}}

#endif
#ifndef Magnum_Math_Half_h
#define Magnum_Math_Half_h

namespace Magnum { namespace Math {

class Half {
    public:
        constexpr /*implicit*/ Half() noexcept: _data{} {}

        constexpr explicit Half(ZeroInitT) noexcept: _data{} {}

        constexpr explicit Half(UnsignedShort data) noexcept: _data{data} {}

        explicit Half(Float value) noexcept: _data{packHalf(value)} {}

        explicit Half(NoInitT) noexcept {}

        constexpr bool operator==(Half other) const {
            return (((      _data & 0x7c00) == 0x7c00 && (      _data & 0x03ff)) ||
                    ((other._data & 0x7c00) == 0x7c00 && (other._data & 0x03ff))) ?
                false : _data == other._data;
        }

        constexpr bool operator!=(Half other) const {
            return !operator==(other);
        }

        constexpr Half operator+() const { return *this; }

        constexpr Half operator-() const {
            return Half{UnsignedShort(_data ^ (1 << 15))};
        }

        constexpr explicit operator UnsignedShort() const { return _data; }

        explicit operator Float() const { return unpackHalf(_data); }

        constexpr UnsignedShort data() const { return _data; }

    private:
        UnsignedShort _data;
};

namespace Literals {

inline Half operator "" _h(long double value) { return Half(Float(value)); }

}

namespace Implementation {

template<> struct StrictWeakOrdering<Half> {
    bool operator()(Half a, Half b) const {
        return a.data() < b.data();
    }
};

}

}}

#endif
#ifndef Magnum_Math_Range_h
#define Magnum_Math_Range_h

namespace Magnum { namespace Math {

namespace Implementation {
    template<UnsignedInt, class> struct RangeTraits;

    template<class T> struct RangeTraits<1, T> { typedef T Type; };
    template<class T> struct RangeTraits<2, T> { typedef Vector2<T> Type; };
    template<class T> struct RangeTraits<3, T> { typedef Vector3<T> Type; };

    template<UnsignedInt, class, class> struct RangeConverter;
}

template<UnsignedInt dimensions, class T> class Range {
    template<UnsignedInt, class> friend class Range;

    public:
        typedef typename Implementation::RangeTraits<dimensions, T>::Type VectorType;

        static Range<dimensions, T> fromSize(const VectorType& min, const VectorType& size) {
            return {min, min+size};
        }

        static Range<dimensions, T> fromCenter(const VectorType& center, const VectorType& halfSize) {
            return {center - halfSize, center + halfSize};
        }

        constexpr /*implicit*/ Range() noexcept: Range<dimensions, T>{ZeroInit, typename std::conditional<dimensions == 1, void*, ZeroInitT*>::type{}} {}

        constexpr explicit Range(ZeroInitT) noexcept: Range<dimensions, T>{ZeroInit, typename std::conditional<dimensions == 1, void*, ZeroInitT*>::type{}} {}

        explicit Range(NoInitT) noexcept: Range<dimensions, T>{NoInit, typename std::conditional<dimensions == 1, void*, NoInitT*>::type{}} {}

        constexpr /*implicit*/ Range(const VectorType& min, const VectorType& max) noexcept: _min{min}, _max{max} {}

        /*implicit*/ Range(const std::pair<VectorType, VectorType>& minmax) noexcept:
            _min{minmax.first}, _max{minmax.second} {}

        template<UnsignedInt d = dimensions, class = typename std::enable_if<d != 1>::type>
        /*implicit*/ Range(const std::pair<Vector<dimensions, T>, Vector<dimensions, T>>& minmax) noexcept: _min{minmax.first}, _max{minmax.second} {}

        template<class U> constexpr explicit Range(const Range<dimensions, U>& other) noexcept: _min(other._min), _max(other._max) {}

        template<class U, class V = decltype(Implementation::RangeConverter<dimensions, T, U>::from(std::declval<U>()))> constexpr explicit Range(const U& other): Range{Implementation::RangeConverter<dimensions, T, U>::from(other)} {}

        constexpr /*implicit*/ Range(const Range<dimensions, T>&) noexcept = default;

        template<class U, class V = decltype(Implementation::RangeConverter<dimensions, T, U>::to(std::declval<Range<dimensions, T>>()))> constexpr explicit operator U() const {
            return Implementation::RangeConverter<dimensions, T, U>::to(*this);
        }

        bool operator==(const Range<dimensions, T>& other) const;

        bool operator!=(const Range<dimensions, T>& other) const {
            return !operator==(other);
        }

        T* data() {
            return dataInternal(typename std::conditional<dimensions == 1, void*, T*>::type{});
        }
        constexpr const T* data() const {
            return dataInternal(typename std::conditional<dimensions == 1, void*, T*>::type{});
        }

        VectorType& min() { return _min; }
        constexpr const VectorType min() const { return _min; }

        VectorType& max() { return _max; }
        constexpr const VectorType max() const { return _max; }

        VectorType size() const { return _max - _min; }

        VectorType center() const { return (_min + _max)/T(2); }

        Range<dimensions, T> translated(const VectorType& vector) const {
            return {_min + vector, _max + vector};
        }

        Range<dimensions, T> padded(const VectorType& padding) const {
            return {_min - padding, _max + padding};
        }

        Range<dimensions, T> scaled(const VectorType& scaling) const {
            return {_min*scaling, _max*scaling};
        }

        Range<dimensions, T> scaledFromCenter(const VectorType& scaling) const {
            return fromCenter(center(), size()*scaling/T(2));
        }

        bool contains(const VectorType& b) const {
            return (Vector<dimensions, T>{b} >= _min).all() &&
                   (Vector<dimensions, T>{b} < _max).all();
        }

        bool contains(const Range<dimensions, T>& b) const {
            return (Vector<dimensions, T>{b._min} >= _min).all() &&
                   (Vector<dimensions, T>{b._max} <= _max).all();
        }

    private:
        constexpr explicit Range(ZeroInitT, ZeroInitT*) noexcept: _min{ZeroInit}, _max{ZeroInit} {}
        constexpr explicit Range(ZeroInitT, void*) noexcept: _min{T(0)}, _max{T(0)} {}

        explicit Range(NoInitT, NoInitT*) noexcept: _min{NoInit}, _max{NoInit} {}
        explicit Range(NoInitT, void*) noexcept {}

        constexpr const VectorType* dataInternal(void*) const { return &_min; }
        VectorType* dataInternal(void*) { return &_min; }
        constexpr const T* dataInternal(T*) const { return _min.data(); }
        T* dataInternal(T*) { return _min.data(); }

        VectorType _min, _max;
};

#define MAGNUM_RANGE_SUBCLASS_IMPLEMENTATION(dimensions, Type, VectorType)  \
    static Type<T> fromSize(const VectorType<T>& min, const VectorType<T>& size) { \
        return Range<dimensions, T>::fromSize(min, size);                   \
    }                                                                       \
    static Type<T> fromCenter(const VectorType<T>& center, const VectorType<T>& halfSize) { \
        return Range<dimensions, T>::fromCenter(center, halfSize);          \
    }                                                                       \
                                                                            \
    Type<T> translated(const VectorType<T>& vector) const {                 \
        return Range<dimensions, T>::translated(vector);                    \
    }                                                                       \
    Type<T> padded(const VectorType<T>& padding) const {                    \
        return Range<dimensions, T>::padded(padding);                       \
    }                                                                       \
    Type<T> scaled(const VectorType<T>& scaling) const {                    \
        return Range<dimensions, T>::scaled(scaling);                       \
    }                                                                       \
    Type<T> scaledFromCenter(const VectorType<T>& scaling) const {          \
        return Range<dimensions, T>::scaledFromCenter(scaling);             \
    }

#ifndef CORRADE_MSVC2015_COMPATIBILITY /* Multiple definitions still broken */
template<class T> using Range1D = Range<1, T>;
#endif

template<class T> class Range2D: public Range<2, T> {
    public:
        constexpr /*implicit*/ Range2D() noexcept: Range<2, T>{ZeroInit} {}

        constexpr explicit Range2D(ZeroInitT) noexcept: Range<2, T>{ZeroInit} {}

        explicit Range2D(NoInitT) noexcept: Range<2, T>{NoInit} {}

        constexpr /*implicit*/ Range2D(const Vector2<T>& min, const Vector2<T>& max) noexcept: Range<2, T>(min, max) {}

        template<class U> constexpr explicit Range2D(const Range2D<U>& other) noexcept: Range<2, T>(other) {}

        template<class U, class V =
            #ifndef CORRADE_MSVC2015_COMPATIBILITY /* Causes ICE */
            decltype(Implementation::RangeConverter<2, T, U>::from(std::declval<U>()))
            #else
            decltype(Implementation::RangeConverter<2, T, U>())
            #endif
            >
        constexpr explicit Range2D(const U& other): Range<2, T>{Implementation::RangeConverter<2, T, U>::from(other)} {}

        constexpr /*implicit*/ Range2D(const Range<2, T>& other) noexcept: Range<2, T>(other) {}

        Vector2<T>& bottomLeft() { return Range<2, T>::min(); }
        constexpr Vector2<T> bottomLeft() const { return Range<2, T>::min(); }

        constexpr Vector2<T> bottomRight() const {
            return {Range<2, T>::max().x(), Range<2, T>::min().y()};
        }

        constexpr Vector2<T> topLeft() const {
            return {Range<2, T>::min().x(), Range<2, T>::max().y()};
        }

        Vector2<T>& topRight() { return Range<2, T>::max(); }
        constexpr Vector2<T> topRight() const { return Range<2, T>::max(); }

        T& left() { return Range<2, T>::min().x(); }
        constexpr T left() const { return Range<2, T>::min().x(); }

        T& right() { return Range<2, T>::max().x(); }
        constexpr T right() const { return Range<2, T>::max().x(); }

        T& bottom() { return Range<2, T>::min().y(); }
        constexpr T bottom() const { return Range<2, T>::min().y(); }

        T& top() { return Range<2, T>::max().y(); }
        constexpr T top() const { return Range<2, T>::max().y(); }

        constexpr Range<1, T> x() const {
            return {Range<2, T>::min().x(), Range<2, T>::max().x()};
        }

        constexpr Range<1, T> y() const {
            return {Range<2, T>::min().y(), Range<2, T>::max().y()};
        }

        T sizeX() const {
            return Range<2, T>::max().x() - Range<2, T>::min().x();
        }

        T sizeY() const {
            return Range<2, T>::max().y() - Range<2, T>::min().y();
        }

        T centerX() const {
            return (Range<2, T>::min().x() + Range<2, T>::max().x())/T(2);
        }

        T centerY() const {
            return (Range<2, T>::min().y() + Range<2, T>::max().y())/T(2);
        }

        MAGNUM_RANGE_SUBCLASS_IMPLEMENTATION(2, Range2D, Vector2)
};

template<class T> class Range3D: public Range<3, T> {
    public:
        constexpr /*implicit*/ Range3D() noexcept: Range<3, T>{ZeroInit} {}

        constexpr explicit Range3D(ZeroInitT) noexcept: Range<3, T>{ZeroInit} {}

        explicit Range3D(NoInitT) noexcept: Range<3, T>{NoInit} {}

        constexpr /*implicit*/ Range3D(const Vector3<T>& min, const Vector3<T>& max) noexcept: Range<3, T>(min, max) {}

        template<class U> constexpr explicit Range3D(const Range3D<U>& other) noexcept: Range<3, T>(other) {}

        template<class U, class V = decltype(Implementation::RangeConverter<3, T, U>::from(std::declval<U>()))> constexpr explicit Range3D(const U& other) noexcept: Range<3, T>{Implementation::RangeConverter<3, T, U>::from(other)} {}

        constexpr /*implicit*/ Range3D(const Range<3, T>& other) noexcept: Range<3, T>(other) {}

        Vector3<T>& backBottomLeft() { return Range<3, T>::min(); }
        constexpr Vector3<T> backBottomLeft() const { return Range<3, T>::min(); }

        constexpr Vector3<T> backBottomRight() const {
            return {Range<3, T>::max().x(), Range<3, T>::min().y(), Range<3, T>::min().z()};
        }

        constexpr Vector3<T> backTopLeft() const {
            return {Range<3, T>::min().x(), Range<3, T>::max().y(), Range<3, T>::min().z()};
        }

        constexpr Vector3<T> backTopRight() const {
            return {Range<3, T>::max().x(), Range<3, T>::max().y(), Range<3, T>::min().z()};
        }

        Vector3<T>& frontTopRight() { return Range<3, T>::max(); }
        constexpr Vector3<T> frontTopRight() const { return Range<3, T>::max(); }

        constexpr Vector3<T> frontTopLeft() const {
            return {Range<3, T>::min().x(), Range<3, T>::max().y(), Range<3, T>::max().z()};
        }

        constexpr Vector3<T> frontBottomRight() const {
            return {Range<3, T>::max().x(), Range<3, T>::min().y(), Range<3, T>::max().z()};
        }

        constexpr Vector3<T> frontBottomLeft() const {
            return {Range<3, T>::min().x(), Range<3, T>::min().y(), Range<3, T>::max().z()};
        }

        T& left() { return Range<3, T>::min().x(); }
        constexpr T left() const { return Range<3, T>::min().x(); }

        T& right() { return Range<3, T>::max().x(); }
        constexpr T right() const { return Range<3, T>::max().x(); }

        T& bottom() { return Range<3, T>::min().y(); }
        constexpr T bottom() const { return Range<3, T>::min().y(); }

        T& top() { return Range<3, T>::max().y(); }
        constexpr T top() const { return Range<3, T>::max().y(); }

        T& back() { return Range<3, T>::min().z(); }
        constexpr T back() const { return Range<3, T>::min().z(); }

        T& front() { return Range<3, T>::max().z(); }
        constexpr T front() const { return Range<3, T>::max().z(); }

        constexpr Range<1, T> x() const {
            return {Range<3, T>::min().x(), Range<3, T>::max().x()};
        }

        constexpr Range<1, T> y() const {
            return {Range<3, T>::min().y(), Range<3, T>::max().y()};
        }

        constexpr Range<1, T> z() const {
            return {Range<3, T>::min().z(), Range<3, T>::max().z()};
        }

        constexpr Range2D<T> xy() const {
            return {Range<3, T>::min().xy(), Range<3, T>::max().xy()};
        }

        T sizeX() const {
            return Range<3, T>::max().x() - Range<3, T>::min().x();
        }

        T sizeY() const {
            return Range<3, T>::max().y() - Range<3, T>::min().y();
        }

        T sizeZ() const {
            return Range<3, T>::max().z() - Range<3, T>::min().z();
        }

        T centerX() const {
            return (Range<3, T>::min().x() + Range<3, T>::max().x())/T(2);
        }

        T centerY() const {
            return (Range<3, T>::min().y() + Range<3, T>::max().y())/T(2);
        }

        T centerZ() const {
            return (Range<3, T>::min().z() + Range<3, T>::max().z())/T(2);
        }

        MAGNUM_RANGE_SUBCLASS_IMPLEMENTATION(3, Range3D, Vector3)
};

template<UnsignedInt dimensions, class T> inline Range<dimensions, T> join(const Range<dimensions, T>& a, const Range<dimensions, T>& b) {
    if(a.min() == a.max()) return b;
    if(b.min() == b.max()) return a;
    return {min(a.min(), b.min()), max(a.max(), b.max())};
}

template<UnsignedInt dimensions, class T> inline Range<dimensions, T> intersect(const Range<dimensions, T>& a, const Range<dimensions, T>& b) {
    if(!intersects(a, b)) return {};
    return {max(a.min(), b.min()), min(a.max(), b.max())};
}

template<UnsignedInt dimensions, class T> inline bool intersects(const Range<dimensions, T>& a, const Range<dimensions, T>& b) {
    return (Vector<dimensions, T>{a.max()} > b.min()).all() &&
           (Vector<dimensions, T>{a.min()} < b.max()).all();
}

template<UnsignedInt dimensions, class T> inline bool Range<dimensions, T>::operator==(const Range<dimensions, T>& other) const {
    return TypeTraits<VectorType>::equals(_min, other._min) &&
        TypeTraits<VectorType>::equals(_max, other._max);
}

namespace Implementation {

template<UnsignedInt dimensions, class T> struct StrictWeakOrdering<Range<dimensions, T>> {
    bool operator()(const Range<dimensions, T>& a, const Range<dimensions, T>& b) const {
        StrictWeakOrdering<typename Range<dimensions, T>::VectorType> o;
        if(o(a.min(), b.min()))
            return true;
        if(o(b.min(), a.min()))
            return false;
        return o(a.max(), b.max());
    }
};

}

}}

#endif
#ifndef Magnum_Math_Intersection_h
#define Magnum_Math_Intersection_h

namespace Magnum { namespace Math { namespace Intersection {

template<class T> inline std::pair<T, T> lineSegmentLineSegment(const Vector2<T>& p, const Vector2<T>& r, const Vector2<T>& q, const Vector2<T>& s) {
    const Vector2<T> qp = q - p;
    const T rs = cross(r, s);
    return {cross(qp, s)/rs, cross(qp, r)/rs};
}

template<class T> inline T lineSegmentLine(const Vector2<T>& p, const Vector2<T>& r, const Vector2<T>& q, const Vector2<T>& s) {
    return cross(q - p, s)/cross(r, s);
}

template<class T> inline T planeLine(const Vector4<T>& plane, const Vector3<T>& p, const Vector3<T>& r) {
    return (-plane.w() - dot(plane.xyz(), p))/dot(plane.xyz(), r);
}

template<class T> bool pointFrustum(const Vector3<T>& point, const Frustum<T>& frustum);

template<class T> bool rangeFrustum(const Range3D<T>& range, const Frustum<T>& frustum);

template<class T> bool aabbFrustum(const Vector3<T>& aabbCenter, const Vector3<T>& aabbExtents, const Frustum<T>& frustum);

template<class T> bool sphereFrustum(const Vector3<T>& sphereCenter, T sphereRadius, const Frustum<T>& frustum);

template<class T> bool pointCone(const Vector3<T>& point, const Vector3<T>& coneOrigin, const Vector3<T>& coneNormal, Rad<T> coneAngle);

template<class T> bool pointCone(const Vector3<T>& point, const Vector3<T>& coneOrigin, const Vector3<T>& coneNormal, T tanAngleSqPlusOne);

template<class T> bool pointDoubleCone(const Vector3<T>& point, const Vector3<T>& coneOrigin, const Vector3<T>& coneNormal, Rad<T> coneAngle);

template<class T> bool pointDoubleCone(const Vector3<T>& point, const Vector3<T>& coneOrigin, const Vector3<T>& coneNormal, T tanAngleSqPlusOne);

template<class T> bool sphereConeView(const Vector3<T>& sphereCenter, T sphereRadius, const Matrix4<T>& coneView, Rad<T> coneAngle);

template<class T> bool sphereConeView(const Vector3<T>& sphereCenter, T sphereRadius, const Matrix4<T>& coneView, T sinAngle, T tanAngle);

template<class T> bool sphereCone(const Vector3<T>& sphereCenter, T sphereRadius, const Vector3<T>& coneOrigin, const Vector3<T>& coneNormal, Rad<T> coneAngle);

template<class T> bool sphereCone(const Vector3<T>& sphereCenter, T sphereRadius, const Vector3<T>& coneOrigin, const Vector3<T>& coneNormal, T sinAngle, T tanAngleSqPlusOne);

template<class T> bool aabbCone(const Vector3<T>& aabbCenter, const Vector3<T>& aabbExtents, const Vector3<T>& coneOrigin, const Vector3<T>& coneNormal, Rad<T> coneAngle);

template<class T> bool aabbCone(const Vector3<T>& aabbCenter, const Vector3<T>& aabbExtents, const Vector3<T>& coneOrigin, const Vector3<T>& coneNormal, T tanAngleSqPlusOne);

template<class T> bool rangeCone(const Range3D<T>& range, const Vector3<T>& coneOrigin, const Vector3<T>& coneNormal, const Rad<T> coneAngle);

template<class T> bool rangeCone(const Range3D<T>& range, const Vector3<T>& coneOrigin, const Vector3<T>& coneNormal, const T tanAngleSqPlusOne);

template<class T> bool pointFrustum(const Vector3<T>& point, const Frustum<T>& frustum) {
    for(const Vector4<T>& plane: frustum) {
        if(Distance::pointPlaneScaled<T>(point, plane) < T(0))
            return false;
    }

    return true;
}

template<class T> bool rangeFrustum(const Range3D<T>& range, const Frustum<T>& frustum) {
    const Vector3<T> center = range.min() + range.max();
    const Vector3<T> extent = range.max() - range.min();

    for(const Vector4<T>& plane: frustum) {
        const Vector3<T> absPlaneNormal = Math::abs(plane.xyz());

        const Float d = Math::dot(center, plane.xyz());
        const Float r = Math::dot(extent, absPlaneNormal);
        if(d + r < -T(2)*plane.w()) return false;
    }

    return true;
}

template<class T> bool aabbFrustum(const Vector3<T>& aabbCenter, const Vector3<T>& aabbExtents, const Frustum<T>& frustum) {
    for(const Vector4<T>& plane: frustum) {
        const Vector3<T> absPlaneNormal = Math::abs(plane.xyz());

        const Float d = Math::dot(aabbCenter, plane.xyz());
        const Float r = Math::dot(aabbExtents, absPlaneNormal);
        if(d + r < -plane.w()) return false;
    }

    return true;
}

template<class T> bool sphereFrustum(const Vector3<T>& sphereCenter, const T sphereRadius, const Frustum<T>& frustum) {
    const T radiusSq = sphereRadius*sphereRadius;

    for(const Vector4<T>& plane: frustum) {
        if(Distance::pointPlaneScaled<T>(sphereCenter, plane) < -radiusSq)
            return false;
    }

    return true;
}

template<class T> bool pointCone(const Vector3<T>& point, const Vector3<T>& coneOrigin, const Vector3<T>& coneNormal, const Rad<T> coneAngle) {
    const T tanAngleSqPlusOne = Math::pow<2>(Math::tan(coneAngle*T(0.5))) + T(1);
    return pointCone(point, coneOrigin, coneNormal, tanAngleSqPlusOne);
}

template<class T> bool pointCone(const Vector3<T>& point, const Vector3<T>& coneOrigin, const Vector3<T>& coneNormal, const T tanAngleSqPlusOne) {
    const Vector3<T> c = point - coneOrigin;
    const T lenA = dot(c, coneNormal);

    return lenA >= 0 && c.dot() <= lenA*lenA*tanAngleSqPlusOne;
}

template<class T> bool pointDoubleCone(const Vector3<T>& point, const Vector3<T>& coneOrigin, const Vector3<T>& coneNormal, const Rad<T> coneAngle) {
    const T tanAngleSqPlusOne = Math::pow<2>(Math::tan(coneAngle*T(0.5))) + T(1);
    return pointDoubleCone(point, coneOrigin, coneNormal, tanAngleSqPlusOne);
}

template<class T> bool pointDoubleCone(const Vector3<T>& point, const Vector3<T>& coneOrigin, const Vector3<T>& coneNormal, const T tanAngleSqPlusOne) {
    const Vector3<T> c = point - coneOrigin;
    const T lenA = dot(c, coneNormal);

    return c.dot() <= lenA*lenA*tanAngleSqPlusOne;
}

template<class T> bool sphereConeView(const Vector3<T>& sphereCenter, const T sphereRadius, const Matrix4<T>& coneView, const Rad<T> coneAngle) {
    const Rad<T> halfAngle = coneAngle*T(0.5);
    const T sinAngle = Math::sin(halfAngle);
    const T tanAngle = Math::tan(halfAngle);

    return sphereConeView(sphereCenter, sphereRadius, coneView, sinAngle, tanAngle);
}

template<class T> bool sphereConeView(const Vector3<T>& sphereCenter, const T sphereRadius, const Matrix4<T>& coneView, const T sinAngle, const T tanAngle) {
    CORRADE_ASSERT(coneView.isRigidTransformation(),
        "Math::Intersection::sphereConeView(): coneView does not represent a rigid transformation:" << Corrade::Utility::Debug::newline << coneView, false);

    const Vector3<T> center = coneView.transformPoint(sphereCenter);

    if (-center.z() > -sphereRadius*sinAngle) {
        const T coneRadius = tanAngle*(center.z() - sphereRadius/sinAngle);
        return center.xy().dot() <= coneRadius*coneRadius;
    } else {
        return center.dot() <= sphereRadius*sphereRadius;
    }

    return false;
}

template<class T> bool sphereCone(
    const Vector3<T>& sphereCenter, const T sphereRadius,
    const Vector3<T>& coneOrigin, const Vector3<T>& coneNormal, const Rad<T> coneAngle)
{
    const Rad<T> halfAngle = coneAngle*T(0.5);
    const T sinAngle = Math::sin(halfAngle);
    const T tanAngleSqPlusOne = T(1) + Math::pow<T>(Math::tan<T>(halfAngle), T(2));

    return sphereCone(sphereCenter, sphereRadius, coneOrigin, coneNormal, sinAngle, tanAngleSqPlusOne);
}

template<class T> bool sphereCone(
    const Vector3<T>& sphereCenter, const T sphereRadius,
    const Vector3<T>& coneOrigin, const Vector3<T>& coneNormal,
    const T sinAngle, const T tanAngleSqPlusOne)
{
    const Vector3<T> diff = sphereCenter - coneOrigin;

    if(Math::dot(diff - sphereRadius*sinAngle*coneNormal, coneNormal) > T(0)) {
        const Vector3<T> c = sinAngle*diff + coneNormal*sphereRadius;
        const T lenA = Math::dot(c, coneNormal);

        return c.dot() <= lenA*lenA*tanAngleSqPlusOne;

    } else return diff.dot() <= sphereRadius*sphereRadius;
}

template<class T> bool aabbCone(
    const Vector3<T>& aabbCenter, const Vector3<T>& aabbExtents,
    const Vector3<T>& coneOrigin, const Vector3<T>& coneNormal, const Rad<T> coneAngle)
{
    const T tanAngleSqPlusOne = Math::pow<T>(Math::tan<T>(coneAngle*T(0.5)), T(2)) + T(1);
    return aabbCone(aabbCenter, aabbExtents, coneOrigin, coneNormal, tanAngleSqPlusOne);
}

template<class T> bool aabbCone(
    const Vector3<T>& aabbCenter, const Vector3<T>& aabbExtents,
    const Vector3<T>& coneOrigin, const Vector3<T>& coneNormal, const T tanAngleSqPlusOne)
{
    const Vector3<T> c = aabbCenter - coneOrigin;

    for(const Int axis: {0, 1, 2}) {
        const Int z = axis;
        const Int x = (axis + 1) % 3;
        const Int y = (axis + 2) % 3;
        if(coneNormal[z] != T(0)) {
            const Float t0 = ((c[z] - aabbExtents[z])/coneNormal[z]);
            const Float t1 = ((c[z] + aabbExtents[z])/coneNormal[z]);

            const Vector3<T> i0 = coneNormal*t0;
            const Vector3<T> i1 = coneNormal*t1;

            for(const auto& i : {i0, i1}) {
                Vector3<T> closestPoint = i;

                if(i[x] - c[x] > aabbExtents[x]) {
                    closestPoint[x] = c[x] + aabbExtents[x];
                } else if(i[x] - c[x] < -aabbExtents[x]) {
                    closestPoint[x] = c[x] - aabbExtents[x];
                }

                if(i[y] - c[y] > aabbExtents[y]) {
                    closestPoint[y] = c[y] + aabbExtents[y];
                } else if(i[y] - c[y] < -aabbExtents[y]) {
                    closestPoint[y] = c[y] - aabbExtents[y];
                }

                if(pointCone<T>(closestPoint, {}, coneNormal, tanAngleSqPlusOne))
                    return true;
            }
        }
    }

    return false;
}

template<class T> bool rangeCone(const Range3D<T>& range, const Vector3<T>& coneOrigin, const Vector3<T>& coneNormal, const Rad<T> coneAngle) {
    const T tanAngleSqPlusOne = Math::pow<2>(Math::tan(coneAngle*T(0.5))) + T(1);
    return rangeCone(range, coneOrigin, coneNormal, tanAngleSqPlusOne);
}

template<class T> bool rangeCone(const Range3D<T>& range, const Vector3<T>& coneOrigin, const Vector3<T>& coneNormal, const T tanAngleSqPlusOne) {
    const Vector3<T> center = (range.min() + range.max())*T(0.5);
    const Vector3<T> extents = (range.max() - range.min())*T(0.5);
    return aabbCone(center, extents, coneOrigin, coneNormal, tanAngleSqPlusOne);
}

}}}

#endif
#ifndef Magnum_Math_StrictWeakOrdering_h
#define Magnum_Math_StrictWeakOrdering_h

namespace Magnum { namespace Math {

namespace Implementation {

template<class T> struct StrictWeakOrdering {
    bool operator()(const T& a, const T& b) const {
        return a < b;
    }
};

}

struct StrictWeakOrdering {
    template<class T> bool operator()(const T& a, const T& b) const {
        Implementation::StrictWeakOrdering<T> o;
        return o(a, b);
    }
};

}}

#endif
#ifndef Magnum_Math_Algorithms_GaussJordan_h
#define Magnum_Math_Algorithms_GaussJordan_h

namespace Magnum { namespace Math { namespace Algorithms {

template<std::size_t size, std::size_t rows, class T> bool gaussJordanInPlaceTransposed(RectangularMatrix<size, size, T>& a, RectangularMatrix<size, rows, T>& t) {
    for(std::size_t row = 0; row != size; ++row) {
        std::size_t rowMax = row;
        for(std::size_t row2 = row+1; row2 != size; ++row2)
            if(std::abs(a[row2][row]) > std::abs(a[rowMax][row]))
                rowMax = row2;

        using std::swap;
        swap(a[row], a[rowMax]);
        swap(t[row], t[rowMax]);

        if(TypeTraits<T>::equals(a[row][row], T(0)))
            return false;

        for(std::size_t row2 = row+1; row2 != size; ++row2) {
            T c = a[row2][row]/a[row][row];

            a[row2] -= a[row]*c;
            t[row2] -= t[row]*c;
        }
    }

    for(std::size_t row = size; row != 0; --row) {
        T c = T(1)/a[row-1][row-1];

        for(std::size_t row2 = 0; row2 != row-1; ++row2)
            t[row2] -= t[row-1]*a[row2][row-1]*c;

        t[row-1] *= c;
    }

    return true;
}

template<std::size_t size, std::size_t cols, class T> bool gaussJordanInPlace(RectangularMatrix<size, size, T>& a, RectangularMatrix<cols, size, T>& t) {
    a = a.transposed();
    RectangularMatrix<size, cols, T> tTransposed = t.transposed();

    bool ret = gaussJordanInPlaceTransposed(a, tTransposed);

    a = a.transposed();
    t = tTransposed.transposed();

    return ret;
}

template<std::size_t size, class T> Matrix<size, T> gaussJordanInverted(Matrix<size, T> matrix) {
    Matrix<size, T> inverted{Math::IdentityInit};
    CORRADE_INTERNAL_ASSERT_OUTPUT(gaussJordanInPlaceTransposed(matrix, inverted));
    return inverted;
}

}}}

#endif
#ifndef Magnum_Math_Algorithms_GramSchmidt_h
#define Magnum_Math_Algorithms_GramSchmidt_h

namespace Magnum { namespace Math { namespace Algorithms {

template<std::size_t cols, std::size_t rows, class T> void gramSchmidtOrthogonalizeInPlace(RectangularMatrix<cols, rows, T>& matrix) {
    static_assert(cols <= rows, "Unsupported matrix aspect ratio");
    for(std::size_t i = 0; i != cols; ++i) {
        for(std::size_t j = i+1; j != cols; ++j)
            matrix[j] -= matrix[j].projected(matrix[i]);
    }
}

template<std::size_t cols, std::size_t rows, class T> RectangularMatrix<cols, rows, T> gramSchmidtOrthogonalize(RectangularMatrix<cols, rows, T> matrix) {
    gramSchmidtOrthogonalizeInPlace(matrix);
    return matrix;
}

template<std::size_t cols, std::size_t rows, class T> void gramSchmidtOrthonormalizeInPlace(RectangularMatrix<cols, rows, T>& matrix) {
    static_assert(cols <= rows, "Unsupported matrix aspect ratio");
    for(std::size_t i = 0; i != cols; ++i) {
        matrix[i] = matrix[i].normalized();
        for(std::size_t j = i+1; j != cols; ++j)
            matrix[j] -= matrix[j].projectedOntoNormalized(matrix[i]);
    }
}

template<std::size_t cols, std::size_t rows, class T> RectangularMatrix<cols, rows, T> gramSchmidtOrthonormalize(RectangularMatrix<cols, rows, T> matrix) {
    gramSchmidtOrthonormalizeInPlace(matrix);
    return matrix;
}

}}}

#endif
#ifndef Magnum_Math_Algorithms_KahanSum_h
#define Magnum_Math_Algorithms_KahanSum_h

namespace Magnum { namespace Math { namespace Algorithms {

template<class Iterator, class T = typename std::decay<decltype(*std::declval<Iterator>())>::type> T kahanSum(Iterator begin, Iterator end, T sum = T(0), T* compensation = nullptr) {
    T c = compensation ? *compensation : T(0);
    for(Iterator it = begin; it != end; ++it) {
        const T y = *it - c;
        const T t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }

    if(compensation) *compensation = c;
    return sum;
}

}}}

#endif
#ifndef Magnum_Math_Algorithms_Qr_h
#define Magnum_Math_Algorithms_Qr_h

namespace Magnum { namespace Math { namespace Algorithms {

template<std::size_t size, class T> std::pair<Matrix<size, T>, Matrix<size, T>> qr(const Matrix<size, T>& matrix) {
    const Matrix<size, T> q = gramSchmidtOrthonormalize(matrix);
    Matrix<size, T> r{ZeroInit};
    for(std::size_t k = 0; k != size; ++k) {
        for(std::size_t j = 0; j <= k; ++j) {
            r[k][j] = Math::dot(q[j], matrix[k]);
        }
    }

    return {q, r};
}

}}}

#endif
#ifdef MAGNUM_MATH_GLM_INTEGRATION
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/dual_quaternion.hpp>
#include <glm/matrix.hpp>
#ifndef Magnum_GlmIntegration_Integration_h
#define Magnum_GlmIntegration_Integration_h

#if GLM_VERSION < 96 /* Was just two decimals in the old days, now it's 3 */
namespace glm {
    template<class T, glm::precision q> using tvec2 = detail::tvec2<T, q>;
    template<class T, glm::precision q> using tvec3 = detail::tvec3<T, q>;
    template<class T, glm::precision q> using tvec4 = detail::tvec4<T, q>;

    template<class T, glm::precision q> using tmat2x2 = detail::tmat2x2<T, q>;
    template<class T, glm::precision q> using tmat2x3 = detail::tmat2x3<T, q>;
    template<class T, glm::precision q> using tmat2x4 = detail::tmat2x4<T, q>;

    template<class T, glm::precision q> using tmat3x2 = detail::tmat3x2<T, q>;
    template<class T, glm::precision q> using tmat3x3 = detail::tmat3x3<T, q>;
    template<class T, glm::precision q> using tmat3x4 = detail::tmat3x4<T, q>;

    template<class T, glm::precision q> using tmat4x2 = detail::tmat4x2<T, q>;
    template<class T, glm::precision q> using tmat4x3 = detail::tmat4x3<T, q>;
    template<class T, glm::precision q> using tmat4x4 = detail::tmat4x4<T, q>;
}
#endif

namespace Magnum { namespace Math { namespace Implementation {

template<
    #if GLM_VERSION < 990
    glm::precision
    #else
    glm::qualifier
    #endif
q> struct BoolVectorConverter<2, glm::tvec2<bool, q>> {
    static BoolVector<2> from(const glm::tvec2<bool, q>& other) {
        return (other.x << 0)|(other.y << 1);
    }

    static glm::tvec2<bool, q> to(const BoolVector<2>& other) {
        return {other[0], other[1]};
    }
};

template<
    #if GLM_VERSION < 990
    glm::precision
    #else
    glm::qualifier
    #endif
q> struct BoolVectorConverter<3, glm::tvec3<bool, q>> {
    static BoolVector<3> from(const glm::tvec3<bool, q>& other) {
        return (other.x << 0)|(other.y << 1)|(other.z << 2);
    }

    static glm::tvec3<bool, q> to(const BoolVector<3>& other) {
        return {other[0], other[1], other[2]};
    }
};

template<
    #if GLM_VERSION < 990
    glm::precision
    #else
    glm::qualifier
    #endif
q> struct BoolVectorConverter<4, glm::tvec4<bool, q>> {
    static BoolVector<4> from(const glm::tvec4<bool, q>& other) {
        return (other.x << 0)|(other.y << 1)|(other.z << 2)|(other.w << 3);
    }

    static glm::tvec4<bool, q> to(const BoolVector<4>& other) {
        return {other[0], other[1], other[2], other[3]};
    }
};

template<class T,
    #if GLM_VERSION < 990
    glm::precision
    #else
    glm::qualifier
    #endif
q> struct VectorConverter<2, T, glm::tvec2<T, q>> {
    static Vector<2, T> from(const glm::tvec2<T, q>& other) {
        return {other.x, other.y};
    }

    static glm::tvec2<T, q> to(const Vector<2, T>& other) {
        return {other[0], other[1]};
    }
};

template<class T,
    #if GLM_VERSION < 990
    glm::precision
    #else
    glm::qualifier
    #endif
q> struct VectorConverter<3, T, glm::tvec3<T, q>> {
    static Vector<3, T> from(const glm::tvec3<T, q>& other) {
        return {other.x, other.y, other.z};
    }

    static glm::tvec3<T, q> to(const Vector<3, T>& other) {
        return {other[0], other[1],  other[2]};
    }
};

template<class T,
    #if GLM_VERSION < 990
    glm::precision
    #else
    glm::qualifier
    #endif
q> struct VectorConverter<4, T, glm::tvec4<T, q>> {
    static Vector<4, T> from(const glm::tvec4<T, q>& other) {
        return {other.x, other.y, other.z, other.w};
    }

    static glm::tvec4<T, q> to(const Vector<4, T>& other) {
        return {other[0], other[1],  other[2], other[3]};
    }
};

template<class T,
    #if GLM_VERSION < 990
    glm::precision
    #else
    glm::qualifier
    #endif
q> struct RectangularMatrixConverter<2, 2, T, glm::tmat2x2<T, q>> {
    static RectangularMatrix<2, 2, T> from(const glm::tmat2x2<T, q>& other) {
        return {Vector<2, T>(other[0]),
                Vector<2, T>(other[1])};
    }

    static glm::tmat2x2<T, q> to(const RectangularMatrix<2, 2, T>& other) {
        return {glm::tvec2<T, q>(other[0]),
                glm::tvec2<T, q>(other[1])};
    }
};

template<class T,
    #if GLM_VERSION < 990
    glm::precision
    #else
    glm::qualifier
    #endif
q> struct RectangularMatrixConverter<2, 3, T, glm::tmat2x3<T, q>> {
    static RectangularMatrix<2, 3, T> from(const glm::tmat2x3<T, q>& other) {
        return {Vector<3, T>(other[0]),
                Vector<3, T>(other[1])};
    }

    static glm::tmat2x3<T, q> to(const RectangularMatrix<2, 3, T>& other) {
        return {glm::tvec3<T, q>(other[0]),
                glm::tvec3<T, q>(other[1])};
    }
};

template<class T,
    #if GLM_VERSION < 990
    glm::precision
    #else
    glm::qualifier
    #endif
q> struct RectangularMatrixConverter<2, 4, T, glm::tmat2x4<T, q>> {
    static RectangularMatrix<2, 4, T> from(const glm::tmat2x4<T, q>& other) {
        return {Vector<4, T>(other[0]),
                Vector<4, T>(other[1])};
    }

    static glm::tmat2x4<T, q> to(const RectangularMatrix<2, 4, T>& other) {
        return {glm::tvec4<T, q>(other[0]),
                glm::tvec4<T, q>(other[1])};
    }
};

template<class T,
    #if GLM_VERSION < 990
    glm::precision
    #else
    glm::qualifier
    #endif
q> struct RectangularMatrixConverter<3, 2, T, glm::tmat3x2<T, q>> {
    static RectangularMatrix<3, 2, T> from(const glm::tmat3x2<T, q>& other) {
        return {Vector<2, T>(other[0]),
                Vector<2, T>(other[1]),
                Vector<2, T>(other[2])};
    }

    static glm::tmat3x2<T, q> to(const RectangularMatrix<3, 2, T>& other) {
        return {glm::tvec2<T, q>(other[0]),
                glm::tvec2<T, q>(other[1]),
                glm::tvec2<T, q>(other[2])};
    }
};

template<class T,
    #if GLM_VERSION < 990
    glm::precision
    #else
    glm::qualifier
    #endif
q> struct RectangularMatrixConverter<3, 3, T, glm::tmat3x3<T, q>> {
    static RectangularMatrix<3, 3, T> from(const glm::tmat3x3<T, q>& other) {
        return {Vector<3, T>(other[0]),
                Vector<3, T>(other[1]),
                Vector<3, T>(other[2])};
    }

    static glm::tmat3x3<T, q> to(const RectangularMatrix<3, 3, T>& other) {
        return {glm::tvec3<T, q>(other[0]),
                glm::tvec3<T, q>(other[1]),
                glm::tvec3<T, q>(other[2])};
    }
};

template<class T,
    #if GLM_VERSION < 990
    glm::precision
    #else
    glm::qualifier
    #endif
q> struct RectangularMatrixConverter<3, 4, T, glm::tmat3x4<T, q>> {
    static RectangularMatrix<3, 4, T> from(const glm::tmat3x4<T, q>& other) {
        return {Vector<4, T>(other[0]),
                Vector<4, T>(other[1]),
                Vector<4, T>(other[2])};
    }

    static glm::tmat3x4<T, q> to(const RectangularMatrix<3, 4, T>& other) {
        return {glm::tvec4<T, q>(other[0]),
                glm::tvec4<T, q>(other[1]),
                glm::tvec4<T, q>(other[2])};
    }
};

template<class T,
    #if GLM_VERSION < 990
    glm::precision
    #else
    glm::qualifier
    #endif
q> struct RectangularMatrixConverter<4, 2, T, glm::tmat4x2<T, q>> {
    static RectangularMatrix<4, 2, T> from(const glm::tmat4x2<T, q>& other) {
        return {Vector<2, T>(other[0]),
                Vector<2, T>(other[1]),
                Vector<2, T>(other[2]),
                Vector<2, T>(other[3])};
    }

    static glm::tmat4x2<T, q> to(const RectangularMatrix<4, 2, T>& other) {
        return {glm::tvec2<T, q>(other[0]),
                glm::tvec2<T, q>(other[1]),
                glm::tvec2<T, q>(other[2]),
                glm::tvec2<T, q>(other[3])};
    }
};

template<class T,
    #if GLM_VERSION < 990
    glm::precision
    #else
    glm::qualifier
    #endif
q> struct RectangularMatrixConverter<4, 3, T, glm::tmat4x3<T, q>> {
    static RectangularMatrix<4, 3, T> from(const glm::tmat4x3<T, q>& other) {
        return {Vector<3, T>(other[0]),
                Vector<3, T>(other[1]),
                Vector<3, T>(other[2]),
                Vector<3, T>(other[3])};
    }

    static glm::tmat4x3<T, q> to(const RectangularMatrix<4, 3, T>& other) {
        return {glm::tvec3<T, q>(other[0]),
                glm::tvec3<T, q>(other[1]),
                glm::tvec3<T, q>(other[2]),
                glm::tvec3<T, q>(other[3])};
    }
};

template<class T,
    #if GLM_VERSION < 990
    glm::precision
    #else
    glm::qualifier
    #endif
q> struct RectangularMatrixConverter<4, 4, T, glm::tmat4x4<T, q>> {
    static RectangularMatrix<4, 4, T> from(const glm::tmat4x4<T, q>& other) {
        return {Vector<4, T>(other[0]),
                Vector<4, T>(other[1]),
                Vector<4, T>(other[2]),
                Vector<4, T>(other[3])};
    }

    static glm::tmat4x4<T, q> to(const RectangularMatrix<4, 4, T>& other) {
        return {glm::tvec4<T, q>(other[0]),
                glm::tvec4<T, q>(other[1]),
                glm::tvec4<T, q>(other[2]),
                glm::tvec4<T, q>(other[3])};
    }
};

}}}

#endif
#ifndef Magnum_GlmIntegration_GtcIntegration_h
#define Magnum_GlmIntegration_GtcIntegration_h

#if GLM_VERSION < 96 /* Was just two decimals in the old days, now it's 3 */
namespace glm {
    template<class T, glm::precision q> using tquat = detail::tquat<T, q>;
}
#endif

namespace Magnum { namespace Math { namespace Implementation {

template<class T,
    #if GLM_VERSION < 990
    glm::precision
    #else
    glm::qualifier
    #endif
q> struct QuaternionConverter<T, glm::tquat<T, q>> {
    static Quaternion<T> from(const glm::tquat<T, q>& other) {
        return {{other.x, other.y, other.z}, other.w};
    }

    static glm::tquat<T, q> to(const Quaternion<T>& other) {
        #if GLM_VERSION*10 + GLM_VERSION_REVISION < 952
        return glm::tquat<T, q>(other.scalar(), other.vector().x(), other.vector().y(), other.vector().z());
        #else
        return {other.scalar(), other.vector().x(), other.vector().y(), other.vector().z()};
        #endif
    }
};

}}}

#endif
#ifndef Magnum_GlmIntegration_GtxIntegration_h
#define Magnum_GlmIntegration_GtxIntegration_h

#if GLM_VERSION < 96 /* Was just two decimals in the old days, now it's 3 */
namespace glm {
    template<class T, glm::precision q> using tdualquat = detail::tdualquat<T, q>;
}
#endif

namespace Magnum { namespace Math { namespace Implementation {

template<class T,
    #if GLM_VERSION < 990
    glm::precision
    #else
    glm::qualifier
    #endif
q> struct DualQuaternionConverter<T, glm::tdualquat<T, q>> {
    static DualQuaternion<T> from(const glm::tdualquat<T, q>& other) {
        return {Quaternion<T>(other.real), Quaternion<T>(other.dual)};
    }

    static glm::tdualquat<T, q> to(const DualQuaternion<T>& other) {
        return {glm::tquat<T, q>(other.real()), glm::tquat<T, q>(other.dual())};
    }
};

}}}

#endif
#endif
#ifdef MAGNUM_MATH_EIGEN_INTEGRATION
#include <Eigen/Core>
#include <Eigen/Geometry>
#ifndef Magnum_EigenIntegration_Integration_h
#define Magnum_EigenIntegration_Integration_h

namespace Magnum {

namespace Math { namespace Implementation {

template<std::size_t size> struct BoolVectorConverter<size, Eigen::Ref<const Eigen::Array<bool, int(size), 1
    #ifdef CORRADE_MSVC2019_COMPATIBILITY
    , 0, int(size), 1
    #endif
>>> {
    static BoolVector<size> from(const Eigen::Ref<const Eigen::Array<bool, size, 1>>& other) {
        BoolVector<size> out;
        for(std::size_t i = 0; i != size; ++i)
            out.set(i, other(i, 0));
        return out;
    }
};
template<std::size_t size> struct BoolVectorConverter<size, Eigen::Ref<Eigen::Array<bool, int(size), 1
    #ifdef CORRADE_MSVC2019_COMPATIBILITY
    , 0, int(size), 1
    #endif
>>>: BoolVectorConverter<size, Eigen::Ref<const Eigen::Array<bool, int(size), 1
    #ifdef CORRADE_MSVC2019_COMPATIBILITY
    , 0, int(size), 1
    #endif
>>> {};
template<std::size_t size> struct BoolVectorConverter<size, Eigen::Array<bool, int(size), 1
    #ifdef CORRADE_MSVC2019_COMPATIBILITY
    , 0, int(size), 1
    #endif
>> {
    static BoolVector<size> from(const Eigen::Array<bool, size, 1>& other) {
        #if defined(__GNUC__) && !defined(__clang__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
        #endif
        return BoolVectorConverter<size, Eigen::Ref<const Eigen::Array<bool, int(size), 1>>>::from(other);
        #if defined(__GNUC__) && !defined(__clang__)
        #pragma GCC diagnostic pop
        #endif
    }

    static Eigen::Array<bool, size, 1> to(const BoolVector<size>& other) {
        Eigen::Array<bool, size, 1> out;
        for(std::size_t i = 0; i != size; ++i)
            out(i, 0) = other[i];
        return out;
    }
};

template<std::size_t size, class T> struct VectorConverter<size, T, Eigen::Ref<const Eigen::Array<T, int(size), 1
    #ifdef CORRADE_MSVC2019_COMPATIBILITY
    , 0, int(size), 1
    #endif
>>> {
    static Vector<size, T> from(const Eigen::Ref<const Eigen::Array<T, size, 1>>& other) {
        Vector<size, T> out{NoInit};
        for(std::size_t i = 0; i != size; ++i)
            out[i] = other(i, 0);
        return out;
    }
};
template<std::size_t size, class T> struct VectorConverter<size, T, Eigen::Ref<Eigen::Array<T, int(size), 1
    #ifdef CORRADE_MSVC2019_COMPATIBILITY
    , 0, int(size), 1
    #endif
>>>: VectorConverter<size, T, Eigen::Ref<const Eigen::Array<T, int(size), 1
    #ifdef CORRADE_MSVC2019_COMPATIBILITY
    , 0, int(size), 1
    #endif
>>> {};
template<std::size_t size, class T> struct VectorConverter<size, T, Eigen::Array<T, int(size), 1
    #ifdef CORRADE_MSVC2019_COMPATIBILITY
    , 0, int(size), 1
    #endif
>> {
    static Vector<size, T> from(const Eigen::Array<T, size, 1>& other) {
        #if defined(__GNUC__) && !defined(__clang__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
        #endif
        return VectorConverter<size, T, Eigen::Ref<const Eigen::Array<T, int(size), 1>>>::from(other);
        #if defined(__GNUC__) && !defined(__clang__)
        #pragma GCC diagnostic pop
        #endif
    }

    static Eigen::Array<T, size, 1> to(const Vector<size, T>& other) {
        Eigen::Array<T, size, 1> out;
        for(std::size_t i = 0; i != size; ++i)
            out(i, 0) = other[i];
        return out;
    }
};

template<std::size_t size, class T> struct VectorConverter<size, T, Eigen::Ref<const Eigen::Matrix<T, int(size), 1
    #ifdef CORRADE_MSVC2019_COMPATIBILITY
    , 0, int(size), 1
    #endif
>>> {
    static Vector<size, T> from(const Eigen::Ref<const Eigen::Matrix<T, size, 1>>& other) {
        Vector<size, T> out{NoInit};
        for(std::size_t i = 0; i != size; ++i)
            out[i] = other(i, 0);
        return out;
    }
};
template<std::size_t size, class T> struct VectorConverter<size, T, Eigen::Ref<Eigen::Matrix<T, int(size), 1
    #ifdef CORRADE_MSVC2019_COMPATIBILITY
    , 0, int(size), 1
    #endif
>>>: VectorConverter<size, T, Eigen::Ref<const Eigen::Matrix<T, int(size), 1
    #ifdef CORRADE_MSVC2019_COMPATIBILITY
    , 0, int(size), 1
    #endif
>>> {};
template<std::size_t size, class T> struct VectorConverter<size, T, Eigen::Matrix<T, int(size), 1
    #ifdef CORRADE_MSVC2019_COMPATIBILITY
    , 0, int(size), 1
    #endif
>> {
    static Vector<size, T> from(const Eigen::Matrix<T, size, 1>& other) {
        #if defined(__GNUC__) && !defined(__clang__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
        #endif
        return VectorConverter<size, T, Eigen::Ref<const Eigen::Matrix<T, int(size), 1>>>::from(other);
        #if defined(__GNUC__) && !defined(__clang__)
        #pragma GCC diagnostic pop
        #endif
    }

    static Eigen::Matrix<T, size, 1> to(const Vector<size, T>& other) {
        Eigen::Matrix<T, size, 1> out;
        for(std::size_t i = 0; i != size; ++i)
            out(i, 0) = other[i];
        return out;
    }
};

template<std::size_t cols, std::size_t rows, class T> struct RectangularMatrixConverter<cols, rows, T, Eigen::Ref<const Eigen::Array<T, int(rows), int(cols)
    #ifdef CORRADE_MSVC2019_COMPATIBILITY
    , 0, int(rows), int(cols)
    #endif
>>> {
    static RectangularMatrix<cols, rows, T> from(const Eigen::Ref<const Eigen::Array<T, rows, cols>>& other) {
        RectangularMatrix<cols, rows, T> out{NoInit};
        for(std::size_t col = 0; col != cols; ++col)
            for(std::size_t row = 0; row != rows; ++row)
                out[col][row] = other(row, col);
        return out;
    }
};
template<std::size_t cols, std::size_t rows, class T> struct RectangularMatrixConverter<cols, rows, T, Eigen::Ref<Eigen::Array<T, int(rows), int(cols)
    #ifdef CORRADE_MSVC2019_COMPATIBILITY
    , 0, int(rows), int(cols)
    #endif
>>>: RectangularMatrixConverter<cols, rows, T, Eigen::Ref<const Eigen::Array<T, int(rows), int(cols)
    #ifdef CORRADE_MSVC2019_COMPATIBILITY
    , 0, int(rows), int(cols)
    #endif
>>> {};
template<std::size_t cols, std::size_t rows, class T> struct RectangularMatrixConverter<cols, rows, T, Eigen::Array<T, int(rows), int(cols)
    #ifdef CORRADE_MSVC2019_COMPATIBILITY
    , 0, int(rows), int(cols)
    #endif
>> {
    static RectangularMatrix<cols, rows, T> from(const Eigen::Array<T, rows, cols>& other) {
        #if defined(__GNUC__) && !defined(__clang__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
        #endif
        return RectangularMatrixConverter<cols, rows, T, Eigen::Ref<const Eigen::Array<T, int(rows), int(cols)>>>::from(other);
        #if defined(__GNUC__) && !defined(__clang__)
        #pragma GCC diagnostic pop
        #endif
    }

    static Eigen::Array<T, rows, cols> to(const RectangularMatrix<cols, rows, T>& other) {
        Eigen::Array<T, rows, cols> out;
        for(std::size_t col = 0; col != cols; ++col)
            for(std::size_t row = 0; row != rows; ++row)
                out(row, col) = other[col][row];
        return out;
    }
};

template<std::size_t cols, std::size_t rows, class T> struct RectangularMatrixConverter<cols, rows, T, Eigen::Ref<const Eigen::Matrix<T, int(rows), int(cols)
    #ifdef CORRADE_MSVC2019_COMPATIBILITY
    , 0, int(rows), int(cols)
    #endif
>>> {
    static RectangularMatrix<cols, rows, T> from(const Eigen::Ref<const Eigen::Matrix<T, rows, cols>>& other) {
        RectangularMatrix<cols, rows, T> out{NoInit};
        for(std::size_t col = 0; col != cols; ++col)
            for(std::size_t row = 0; row != rows; ++row)
                out[col][row] = other(row, col);
        return out;
    }
};
template<std::size_t cols, std::size_t rows, class T> struct RectangularMatrixConverter<cols, rows, T, Eigen::Ref<Eigen::Matrix<T, int(rows), int(cols)
    #ifdef CORRADE_MSVC2019_COMPATIBILITY
    , 0, int(rows), int(cols)
    #endif
>>>: RectangularMatrixConverter<cols, rows, T, Eigen::Ref<const Eigen::Matrix<T, int(rows), int(cols)
    #ifdef CORRADE_MSVC2019_COMPATIBILITY
    , 0, int(rows), int(cols)
    #endif
>>> {};
template<std::size_t cols, std::size_t rows, class T> struct RectangularMatrixConverter<cols, rows, T, Eigen::Matrix<T, int(rows), int(cols)
    #ifdef CORRADE_MSVC2019_COMPATIBILITY
    , 0, int(rows), int(cols)
    #endif
>> {
    static RectangularMatrix<cols, rows, T> from(const Eigen::Matrix<T, rows, cols>& other) {
        #if defined(__GNUC__) && !defined(__clang__)
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
        #endif
        return RectangularMatrixConverter<cols, rows, T, Eigen::Ref<const Eigen::Matrix<T, int(rows), int(cols)>>>::from(other);
        #if defined(__GNUC__) && !defined(__clang__)
        #pragma GCC diagnostic pop
        #endif
    }

    static Eigen::Matrix<T, rows, cols> to(const RectangularMatrix<cols, rows, T>& other) {
        Eigen::Matrix<T, rows, cols> out;
        for(std::size_t col = 0; col != cols; ++col)
            for(std::size_t row = 0; row != rows; ++row)
                out(row, col) = other[col][row];
        return out;
    }
};

}}

namespace EigenIntegration {

template<class To, std::size_t cols, std::size_t rows, class T> inline To cast(const Math::RectangularMatrix<cols, rows, T>& from) {
    return Math::Implementation::RectangularMatrixConverter<cols, rows, T, To>::to(from);
}

template<class To, std::size_t size> inline To cast(const Math::BoolVector<size>& from) {
    return Math::Implementation::BoolVectorConverter<size, To>::to(from);
}

template<class To, std::size_t size, class T> inline To cast(const Math::Vector<size, T>& from) {
    return Math::Implementation::VectorConverter<size, T, To>::to(from);
}

}

}

#endif
#ifndef Magnum_EigenIntegration_GeometryIntegration_h
#define Magnum_EigenIntegration_GeometryIntegration_h

namespace Magnum { namespace Math { namespace Implementation {

template<std::size_t size, class T> struct VectorConverter<size, T, Eigen::Translation<T, int(size)>> {
    static Vector<size, T> from(const Eigen::Translation<T, size>& other) {
        return Vector<size, T>(other.vector());
    }

    static Eigen::Translation<T, size> to(const Vector<size, T>& other) {
        return Eigen::Translation<T, size>(VectorConverter<size, T, Eigen::Matrix<T, size, 1>>::to(other));
    }
};

template<std::size_t size, class T, int mode> struct RectangularMatrixConverter<size, size, T, Eigen::Transform<T, int(size - 1), mode>> {
    static_assert(mode == Eigen::Affine || mode == Eigen::Projective || mode == Eigen::Isometry,
        "only Affine, Projective and Isometry transform supported");

    static RectangularMatrix<size, size, T> from(const Eigen::Transform<T, int(size - 1), mode>& other) {
        return RectangularMatrix<size, size, T>(other.matrix());
    }

    static Eigen::Transform<T, int(size - 1), mode> to(const RectangularMatrix<size, size, T>& other) {
        return Eigen::Transform<T, int(size - 1), mode>(RectangularMatrixConverter<size, size, T, Eigen::Matrix<T, size, size>>::to(other));
    }
};

template<class T> struct RectangularMatrixConverter<3, 2, T, Eigen::Transform<T, 2, Eigen::AffineCompact>> {
    static RectangularMatrix<3, 2, T> from(const Eigen::Transform<T, 2, Eigen::AffineCompact>& other) {
        return RectangularMatrix<3, 2, T>(other.matrix());
    }

    static Eigen::Transform<T, 2, Eigen::AffineCompact> to(const RectangularMatrix<3, 2, T>& other) {
        return Eigen::Transform<T, 2, Eigen::AffineCompact>(RectangularMatrixConverter<3, 2, T, Eigen::Matrix<T, 2, 3>>::to(other));
    }
};
template<class T> struct RectangularMatrixConverter<4, 3, T, Eigen::Transform<T, 3, Eigen::AffineCompact>> {
    static RectangularMatrix<4, 3, T> from(const Eigen::Transform<T, 3, Eigen::AffineCompact>& other) {
        return RectangularMatrix<4, 3, T>(other.matrix());
    }

    static Eigen::Transform<T, 3, Eigen::AffineCompact> to(const RectangularMatrix<4, 3, T>& other) {
        return Eigen::Transform<T, 3, Eigen::AffineCompact>(RectangularMatrixConverter<4, 3, T, Eigen::Matrix<T, 3, 4>>::to(other));
    }
};

template<class T> struct QuaternionConverter<T, Eigen::Quaternion<T>> {
    static Quaternion<T> from(const Eigen::Quaternion<T>& other) {
        return {{other.x(), other.y(), other.z()}, other.w()};
    }

    static Eigen::Quaternion<T> to(const Quaternion<T>& other) {
        return {other.scalar(), other.vector().x(), other.vector().y(), other.vector().z()};
    }
};

}}

namespace EigenIntegration {

template<class To, class T> inline To cast(const Math::Quaternion<T>& from) {
    return To(from);
}

}

}

#endif
#endif
#ifdef MAGNUM_MATH_IMPLEMENTATION
namespace Magnum { namespace Math {

UnsignedInt log(UnsignedInt base, UnsignedInt number) {
    UnsignedInt log = 0;
    while(number /= base)
        ++log;
    return log;
}

UnsignedInt log2(UnsignedInt number) {
    UnsignedInt log = 0;
    while(number >>= 1)
        ++log;
    return log;
}

}}
namespace Magnum { namespace Math {

namespace {

union FloatBits {
    UnsignedInt u;
    Float f;
};

}

Float unpackHalf(const UnsignedShort value) {
    constexpr const FloatBits Magic{113 << 23};
    constexpr const UnsignedInt ShiftedExp = 0x7c00 << 13;

    const UnsignedShort h{value};
    FloatBits o;

    o.u = (h & 0x7fff) << 13;
    const UnsignedInt exp = ShiftedExp & o.u;
    o.u += (127 - 15) << 23;

    if(exp == ShiftedExp) {
        o.u += (128 - 16) << 23;
    } else if(exp == 0) {
        o.u += 1 << 23;
        o.f -= Magic.f;
    }

    o.u |= (h & 0x8000) << 16;
    return o.f;
}

UnsignedShort packHalf(const Float value) {
    constexpr const FloatBits FloatInfinity{255 << 23};
    constexpr const FloatBits HalfInfinity{31 << 23};
    constexpr const FloatBits Magic{15 << 23};
    constexpr const UnsignedInt SignMask = 0x80000000u;
    constexpr const UnsignedInt RoundMask = ~0xfffu;

    FloatBits f;
    f.f = value;
    UnsignedShort h;

    const UnsignedInt sign = f.u & SignMask;
    f.u ^= sign;

    if(f.u >= FloatInfinity.u) {
        h = (f.u > FloatInfinity.u) ? 0x7e00 : 0x7c00;

    } else {
        f.u &= RoundMask;
        f.f *= Magic.f;
        f.u -= RoundMask;

        if (f.u > HalfInfinity.u) f.u = HalfInfinity.u;

        h = f.u >> 13;
    }

    h |= sign >> 16;
    return h;
}

}}
#endif
