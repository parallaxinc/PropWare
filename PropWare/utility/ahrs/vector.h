/**
 * @file    PropWare/utility/imu/vector.h
 *
 * @author  David Zemon (Only modified for use in PropWare)
 *
 * @copyright
 * Inertial Measurement Unit Maths Library
 * Copyright (C) 2013-2014  Samuel Cowen
 * www.camelsoftware.com
 *
 * Bug fixes and cleanups by Gé Vissers (gvissers@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <cstring>
#include <cstdint>
#include <cmath>

namespace PropWare {

namespace imu {

template<size_t N>
class Vector {
    public:
        Vector () {
            memset(m_vector, 0, sizeof(double) * N);
        }

        Vector (double a) {
            memset(m_vector, 0, sizeof(double) * N);

            static_assert(1 <= N, "Size must be greater than or equal to 1");
            this->m_vector[0] = a;
        }

        Vector (double a, double b) {
            static_assert(2 <= N, "Size must be greater than or equal to 2");

            memset(m_vector, 0, sizeof(double) * N);
            this->m_vector[0] = a;
            this->m_vector[1] = b;
        }

        Vector (double a, double b, double c) {
            static_assert(3 <= N, "Size must be greater than or equal to 3");

            memset(m_vector, 0, sizeof(double) * N);
            this->m_vector[0] = a;
            this->m_vector[1] = b;
            this->m_vector[2] = c;
        }

        Vector (double a, double b, double c, double d) {
            static_assert(4 <= N, "Size must be greater than or equal to 4");

            memset(m_vector, 0, sizeof(double) * N);
            this->m_vector[0] = a;
            this->m_vector[1] = b;
            this->m_vector[2] = c;
            this->m_vector[3] = d;
        }

        Vector (const Vector<N> &v) {
            for (size_t x = 0; x < N; x++)
                this->m_vector[x] = v.m_vector[x];
        }

        size_t n () const {
            return N;
        }

        double magnitude () const {
            double      res = 0;
            for (size_t i   = 0; i < N; ++i)
                res += this->m_vector[i] * this->m_vector[i];

            return sqrt(res);
        }

        void normalize () {
            double mag = this->magnitude();
            if (isnan(mag) || mag == 0.0)
                return;

            for (size_t i = 0; i < N; ++i)
                this->m_vector[i] /= mag;
        }

        double dot (const Vector &v) const {
            double      ret = 0;
            for (size_t i   = 0; i < N; ++i)
                ret += this->m_vector[i] * v.m_vector[i];
            return ret;
        }

        // The cross product is only valid for vectors with 3 dimensions,
        // with the exception of higher dimensional stuff that is beyond
        // the intended scope of this library.
        // Only a definition for N==3 is given below this class, using
        // cross() with another value for N will result in a link error.
        Vector cross (const Vector &v) const;

        Vector scale (double scalar) const {
            Vector      ret;
            for (size_t i = 0; i < N; ++i)
                ret.m_vector[i] = this->m_vector[i] * scalar;
            return ret;
        }

        Vector invert () const {
            Vector      ret;
            for (size_t i = 0; i < N; ++i)
                ret.m_vector[i] = -m_vector[i];
            return ret;
        }

        Vector &operator= (const Vector &v) {
            for (size_t x = 0; x < N; x++)
                this->m_vector[x] = v.m_vector[x];
            return *this;
        }

        double &operator[] (const size_t n) {
            return this->m_vector[n];
        }

        double operator[] (const size_t n) const {
            return this->m_vector[n];
        }

        double &operator() (const size_t n) {
            return this->m_vector[n];
        }

        double operator() (const size_t n) const {
            return this->m_vector[n];
        }

        Vector operator+ (const Vector &v) const {
            Vector      ret;
            for (size_t i = 0; i < N; ++i)
                ret.m_vector[i] = this->m_vector[i] + v.m_vector[i];
            return ret;
        }

        Vector operator- (const Vector &v) const {
            Vector      ret;
            for (size_t i = 0; i < N; ++i)
                ret.m_vector[i] = this->m_vector[i] - v.m_vector[i];
            return ret;
        }

        Vector operator* (double scalar) const {
            return scale(scalar);
        }

        Vector operator/ (double scalar) const {
            Vector      ret;
            for (size_t i = 0; i < N; ++i)
                ret.m_vector[i] = this->m_vector[i] / scalar;
            return ret;
        }

        void to_degrees () {
            for (size_t i = 0; i < N; ++i)
                this->m_vector[i] *= 180 / M_PI;
        }

        void to_radians () {
            for (size_t i = 0; i < N; ++i)
                this->m_vector[i] *= M_PI / 180;
        }

        double &x () {
            return this->m_vector[0];
        }
        double &y () {
            return this->m_vector[1];
        }
        double &z () {
            return this->m_vector[2];
        }
        double x () const {
            return this->m_vector[0];
        }
        double y () const {
            return this->m_vector[1];
        }
        double z () const {
            return this->m_vector[2];
        }


    private:
        double m_vector[N];
};


template<>
inline Vector<3> Vector<3>::cross (const Vector &v) const {
    return Vector(
        this->m_vector[1] * v.m_vector[2] - this->m_vector[2] * v.m_vector[1],
        this->m_vector[2] * v.m_vector[0] - this->m_vector[0] * v.m_vector[2],
        this->m_vector[0] * v.m_vector[1] - this->m_vector[1] * v.m_vector[0]
    );
}

}

}
