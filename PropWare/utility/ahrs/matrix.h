/**
 * @file    PropWare/utility/imu/matrix.h
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

#include <PropWare/utility/ahrs/vector.h>

namespace PropWare {

namespace imu {

template<size_t N>
class Matrix {
    public:
        Matrix () {
            memset(m_matrix, 0, N * N * sizeof(double));
        }

        Matrix (const Matrix &m) {
            for (size_t ij = 0; ij < N * N; ++ij)
                this->m_matrix[ij] = m.m_matrix[ij];
        }

        Matrix &operator= (const Matrix &m) {
            for (size_t ij = 0; ij < N * N; ++ij)
                this->m_matrix[ij] = m.m_matrix[ij];
            return *this;
        }

        Vector<N> row_to_vector (const size_t i) const {
            Vector<N>   ret;
            for (size_t j = 0; j < N; j++)
                ret[j] = this->cell(i, j);
            return ret;
        }

        Vector<N> col_to_vector (const size_t j) const {
            Vector<N>   ret;
            for (size_t i = 0; i < N; i++)
                ret[i] = this->cell(i, j);
            return ret;
        }

        void vector_to_row (const Vector<N> &v, const size_t i) {
            for (size_t j = 0; j < N; j++)
                this->cell(i, j) = v[j];
        }

        void vector_to_col (const Vector<N> &v, const size_t j) {
            for (size_t i = 0; i < N; i++)
                this->cell(i, j) = v[i];
        }

        double operator() (const size_t i, const size_t j) const {
            return this->cell(i, j);
        }
        double &operator() (const size_t i, const size_t j) {
            return this->cell(i, j);
        }

        double cell (const size_t i, const size_t j) const {
            return this->m_matrix[i * N + j];
        }
        double &cell (const size_t i, const size_t j) {
            return this->m_matrix[i * N + j];
        }


        Matrix operator+ (const Matrix &m) const {
            Matrix      ret;
            for (size_t ij = 0; ij < N * N; ++ij)
                ret.m_matrix[ij] = this->m_matrix[ij] + m.m_matrix[ij];
            return ret;
        }

        Matrix operator- (const Matrix &m) const {
            Matrix      ret;
            for (size_t ij = 0; ij < N * N; ++ij)
                ret.m_matrix[ij] = this->m_matrix[ij] - m.m_matrix[ij];
            return ret;
        }

        Matrix operator* (double scalar) const {
            Matrix      ret;
            for (size_t ij = 0; ij < N * N; ++ij)
                ret.m_matrix[ij] = this->m_matrix[ij] * scalar;
            return ret;
        }

        Matrix operator* (const Matrix &m) const {
            Matrix      ret;
            for (size_t i = 0; i < N; i++) {
                Vector<N>   row = this->row_to_vector(i);
                for (size_t j   = 0; j < N; j++)
                    ret(i, j) = row.dot(m.col_to_vector(j));
            }
            return ret;
        }

        Matrix transpose () const {
            Matrix      ret;
            for (size_t i = 0; i < N; i++)
                for (size_t j = 0; j < N; j++)
                    ret(j, i) = this->cell(i, j);
            return ret;
        }

        Matrix<N - 1> minor_matrix (const size_t row, const size_t col) const {
            Matrix<N - 1> ret;
            for (size_t   i = 0, im = 0; i < N; i++) {
                if (i == row)
                    continue;

                for (size_t j = 0, jm = 0; j < N; j++)
                    if (j != col)
                        ret(im, jm++) = this->cell(i, j);

                im++;
            }
            return ret;
        }

        double determinant () const {
            // specialization for N == 1 given below this class
            double      det = 0.0, sign = 1.0;
            for (size_t i   = 0; i < N; ++i, sign = -sign)
                det += sign * this->cell(0, i) * this->minor_matrix(0, i).determinant();
            return det;
        }

        Matrix invert () const {
            Matrix ret;
            double det = this->determinant();

            for (size_t i = 0; i < N; i++)
                for (size_t j = 0; j < N; j++) {
                    ret(i, j)     = this->minor_matrix(j, i).determinant() / det;
                    if ((i + j) % 2 == 1)
                        ret(i, j) = -ret(i, j);
                }
            return ret;
        }

        double trace () const {
            double      tr = 0.0;
            for (size_t i  = 0; i < N; ++i)
                tr += this->cell(i, i);
            return tr;
        }

    private:
        double m_matrix[N * N];
};


template<>
inline double Matrix<1>::determinant () const {
    return this->cell(0, 0);
}

};

}
