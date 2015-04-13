// Cyphesis Online RPG Server and AI Engine
// Copyright (C) 2015 Alistair Riddoch
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

// $Id$

#include "atlas_boost_geometry_impl.h"

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/ring.hpp>

using Atlas::Message::Element;

using point = boost::geometry::model::d2::point_xy<
        float, boost::geometry::cs::cartesian>;
using ring = boost::geometry::model::ring<point>;

template int boostGeometryFromMessage<ring>(Element const & data,
                                            ring & shape);
