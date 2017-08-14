/*
    Inertial Measurement Unit Maths Library
    Copyright (C) 2013-2014  Samuel Cowen
	www.camelsoftware.com

    Bug fixes and cleanups by Gé Vissers (gvissers@gmail.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#pragma once

#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cmath>

#include <PropWare/utility/ahrs/matrix.h>

namespace PropWare {

namespace imu {

class Quaternion {
    public:
        Quaternion ()
            : m_w(1.0),
              m_x(0.0),
              m_y(0.0),
              m_z(0.0) {
        }

        Quaternion (const double w, const double x, const double y, const double z)
            : m_w(w),
              m_x(x),
              m_y(y),
              m_z(z) {
        }

        Quaternion (const double w, const Vector<3> &vec)
            : m_w(w),
              m_x(vec.x()),
              m_y(vec.y()),
              m_z(vec.z()) {
        }

        double w () const {
            return this->m_w;
        }
        double x () const {
            return this->m_x;
        }
        double y () const {
            return this->m_y;
        }
        double z () const {
            return this->m_z;
        }

        double magnitude () const {
            return sqrt(
                m_w * this->m_w
                    + this->m_x * this->m_x
                    + this->m_y * this->m_y
                    + this->m_z * this->m_z
            );
        }

        void normalize () {
            double mag = magnitude();
            *this = this->scale(1 / mag);
        }

        Quaternion conjugate () const {
            return Quaternion(m_w, -m_x, -m_y, -m_z);
        }

        void from_axis_angle (const Vector<3> &axes, const double theta) {
            this->m_w        = cos(theta / 2);
            //only need to calculate sine of half theta once
            const double sht = sin(theta / 2);
            this->m_x = axes.x() * sht;
            this->m_y = axes.y() * sht;
            this->m_z = axes.z() * sht;
        }

        void from_matrix (const Matrix<3> &m) {
            double tr = m.trace();

            double S;
            if (tr > 0) {
                S = sqrt(tr + 1.0) * 2;
                this->m_w = 0.25 * S;
                this->m_x = (m(2, 1) - m(1, 2)) / S;
                this->m_y = (m(0, 2) - m(2, 0)) / S;
                this->m_z = (m(1, 0) - m(0, 1)) / S;
            } else if (m(0, 0) > m(1, 1) && m(0, 0) > m(2, 2)) {
                S = sqrt(1.0 + m(0, 0) - m(1, 1) - m(2, 2)) * 2;
                this->m_w = (m(2, 1) - m(1, 2)) / S;
                this->m_x = 0.25 * S;
                this->m_y = (m(0, 1) + m(1, 0)) / S;
                this->m_z = (m(0, 2) + m(2, 0)) / S;
            } else if (m(1, 1) > m(2, 2)) {
                S = sqrt(1.0 + m(1, 1) - m(0, 0) - m(2, 2)) * 2;
                this->m_w = (m(0, 2) - m(2, 0)) / S;
                this->m_x = (m(0, 1) + m(1, 0)) / S;
                this->m_y = 0.25 * S;
                this->m_z = (m(1, 2) + m(2, 1)) / S;
            } else {
                S = sqrt(1.0 + m(2, 2) - m(0, 0) - m(1, 1)) * 2;
                this->m_w = (m(1, 0) - m(0, 1)) / S;
                this->m_x = (m(0, 2) + m(2, 0)) / S;
                this->m_y = (m(1, 2) + m(2, 1)) / S;
                this->m_z = 0.25 * S;
            }
        }

        void to_axis_angle (Vector<3> &axes, double &angle) const {
            double sqw = sqrt(1 - this->m_w * this->m_w);
            if (sqw == 0) //it's a singularity and divide by zero, avoid
                return;

            angle = 2 * acos(m_w);
            axes.x() = this->m_x / sqw;
            axes.y() = this->m_y / sqw;
            axes.z() = this->m_z / sqw;
        }

        Matrix<3> to_matrix () const {
            Matrix<3> ret;
            ret.cell(0, 0) = 1 - 2 * this->m_y * this->m_y - 2 * this->m_z * this->m_z;
            ret.cell(0, 1) = 2 * this->m_x * this->m_y - 2 * this->m_w * this->m_z;
            ret.cell(0, 2) = 2 * this->m_x * this->m_z + 2 * this->m_w * this->m_y;

            ret.cell(1, 0) = 2 * this->m_x * this->m_y + 2 * this->m_w * this->m_z;
            ret.cell(1, 1) = 1 - 2 * this->m_x * this->m_x - 2 * this->m_z * this->m_z;
            ret.cell(1, 2) = 2 * this->m_y * this->m_z - 2 * this->m_w * this->m_x;

            ret.cell(2, 0) = 2 * this->m_x * this->m_z - 2 * this->m_w * this->m_y;
            ret.cell(2, 1) = 2 * this->m_y * this->m_z + 2 * this->m_w * this->m_x;
            ret.cell(2, 2) = 1 - 2 * this->m_x * this->m_x - 2 * this->m_y * this->m_y;
            return ret;
        }

        /**
         * @brief   Returns euler angles that represent the quaternion
         *
         * Angles are returned in rotation order and right-handed about the specified axes:
         *
         *  - `v[0]` is applied 1st about z (ie, roll)
         *  - `v[1]` is applied 2nd about y (ie, pitch)
         *  - `v[2]` is applied 3rd about x (ie, yaw)
         *
         * Note that this means `result.x()` is not a rotation about x; similarly for `result.z()`.
         *
         */
        Vector<3> to_euler () const {
            const auto sqw = this->m_w * this->m_w;
            const auto sqx = this->m_x * this->m_x;
            const auto sqy = this->m_y * this->m_y;
            const auto sqz = this->m_z * this->m_z;

            return Vector<3>(
                atan2(2.0 * (m_x * this->m_y + this->m_z * this->m_w), (sqx - sqy - sqz + sqw)),
                asin(-2.0 * (m_x * this->m_z - this->m_y * this->m_w) / (sqx + sqy + sqz + sqw)),
                atan2(2.0 * (m_y * this->m_z + this->m_x * this->m_w), (-sqx - sqy + sqz + sqw))
            );
        }

        Vector<3> to_angular_velocity (double dt) const {
            Vector<3>  ret;
            const Quaternion one(1.0, 0.0, 0.0, 0.0);
            const Quaternion delta = one - *this;
            Quaternion r     = (delta / dt);
            r = r * 2;
            r = r * one;

            ret.x() = r.x();
            ret.y() = r.y();
            ret.z() = r.z();
            return ret;
        }

        Vector<3> rotate_vector (const Vector<2> &v) const {
            return this->rotate_vector(Vector<3>(v.x(), v.y()));
        }

        Vector<3> rotate_vector (const Vector<3> &v) const {
            Vector<3> qv(m_x, this->m_y, this->m_z);
            Vector<3> t = qv.cross(v) * 2.0;
            return v + t * this->m_w + qv.cross(t);
        }


        Quaternion operator* (const Quaternion &q) const {
            return Quaternion(
                this->m_w * q.m_w - this->m_x * q.m_x - this->m_y * q.m_y - this->m_z * q.m_z,
                this->m_w * q.m_x + this->m_x * q.m_w + this->m_y * q.m_z - this->m_z * q.m_y,
                this->m_w * q.m_y - this->m_x * q.m_z + this->m_y * q.m_w + this->m_z * q.m_x,
                this->m_w * q.m_z + this->m_x * q.m_y - this->m_y * q.m_x + this->m_z * q.m_w
            );
        }

        Quaternion operator+ (const Quaternion &q) const {
            return Quaternion(m_w + q.m_w, this->m_x + q.m_x, this->m_y + q.m_y, this->m_z + q.m_z);
        }

        Quaternion operator- (const Quaternion &q) const {
            return Quaternion(m_w - q.m_w, this->m_x - q.m_x, this->m_y - q.m_y, this->m_z - q.m_z);
        }

        Quaternion operator/ (const double scalar) const {
            return Quaternion(m_w / scalar, this->m_x / scalar, this->m_y / scalar, this->m_z / scalar);
        }

        Quaternion operator* (const double scalar) const {
            return scale(scalar);
        }

        Quaternion scale (const double scalar) const {
            return Quaternion(m_w * scalar, this->m_x * scalar, this->m_y * scalar, this->m_z * scalar);
        }

    private:
        double m_w;
        double m_x;
        double m_y;
        double m_z;
};

}

}
