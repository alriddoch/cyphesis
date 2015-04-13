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

#ifndef COMMON_ATLAS_BOOST_GEOMETRY_IMPL_H
#define COMMON_ATLAS_BOOST_GEOMETRY_IMPL_H

#include "atlas_boost_geometry.h"

#include <Atlas/Message/Element.h>

namespace {

template <class Shape>
int _CornersFromMessageList(Atlas::Message::ListType const & list,
                            Shape & shape)
{
  // TODO: Actually write this implementation
  return 0;
}

}

template <class Shape>
int boostGeometryFromMessage(Atlas::Message::Element const & message,
                             Shape & shape)
{
  // TODO: Finish writing and testing this function
  if (message.isMap())
  {
    Atlas::Message::MapType const & shapeElement(message.Map());
    Atlas::Message::MapType::const_iterator it = shapeElement.find("points");
    if ((it != shapeElement.end()) && it->second.isList()) {
      return _CornersFromMessageList(it->second.List(), shape);
    }
  }
  else if (message.isList())
  {
    return _CornersFromMessageList(message.List(), shape);
  }

  return 0;
}

#endif // COMMON_ATLAS_BOOST_GEOMETRY_IMPL_H