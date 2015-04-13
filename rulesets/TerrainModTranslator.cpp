// Cyphesis Online RPG Server and AI Engine
// Copyright (C) 2010 Alistair Riddoch
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

#include "TerrainModTranslator.h"

#include "common/atlas_boost_geometry.h"
#include "common/log.h"
#include "common/debug.h"

#include <dymaxion/TerrainMod.h>

#include <wfmath/atlasconv.h>

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/ring.hpp>

#include <cassert>

using Atlas::Message::Element;
using Atlas::Message::MapType;
using Atlas::Message::ListType;
using Atlas::Message::FloatType;

using point = boost::geometry::model::d2::point_xy<
        float, boost::geometry::cs::cartesian>;
using ring = boost::geometry::model::ring<point>;

/**
 * @brief Ctor.
 */
TerrainModTranslator::TerrainModTranslator() : m_mod(0)
{
}

/**
 * @brief Parse the shape data and create the terrain mod instance with it
 * @param pos Position of the mod entity
 * @param orientation Orientation of the mod entity
 * @param modElement Atlas data describing the mod
 * @param typeName Name of the type of mod from the Atlas data
 * @param shape Reference to the shape object to be populated
 * @param shapeMap Atlas data containing the shape parameters
 */
template <class Shape>
bool TerrainModTranslator::parseStuff(
      const WFMath::Point<3> & pos,
      const WFMath::Quaternion & orientation,
      const MapType& modElement,
      const std::string & typeName,
      Shape & shape,
      const Element & shapeMap)
{
    if (!parseShape(shapeMap, pos, orientation, shape)) {
        return false;
    }
    if (typeName == "slopemod") {
        return createInstance<dymaxion::SlopeTerrainMod>(shape, pos, modElement, 0, 0);
    } else if (typeName == "levelmod") {
        return createInstance<dymaxion::LevelTerrainMod>(shape, pos, modElement);
    } else if (typeName == "adjustmod") {
        return createInstance<dymaxion::AdjustTerrainMod>(shape, pos, modElement);
    } else if (typeName == "cratermod") {
        return createInstance<dymaxion::CraterTerrainMod>(shape, pos, modElement);
    }
    return false;
}

/** 
 * @brief Parse the Atlas data and create the terrain mod instance with it
 * @param pos Position of the mod entity
 * @param orientation Orientation of the mod entity
 * @param modElement Atlas data describing the mod
 * @return true if translation succeeds
 */
bool TerrainModTranslator::parseData(
      const WFMath::Point<3> & pos,
      const WFMath::Quaternion & orientation,
      const MapType& modElement)
{
    MapType::const_iterator I = modElement.find("type");
    if (I == modElement.end() || !I->second.isString()) {
        return false;
    }
    const std::string& modType = I->second.String();

    I = modElement.find("shape");
    if (I == modElement.end() || !I->second.isMap()) {
        return false;
    }
    const MapType& shapeMap = I->second.Map();

    // Get shape's type
    I = shapeMap.find("type");
    if (I == shapeMap.end() || !I->second.isString()) {
        return false;
    }
    const std::string& shapeType = I->second.String();
    if (shapeType == "polygon") {
        ring shape;
        return parseStuff(pos, orientation, modElement, modType, shape, shapeMap);
    }
    return false;
}


dymaxion::TerrainMod* TerrainModTranslator::getModifier()
{
    return m_mod;
}

/**
 * @brief Parses the changes to the position of the mod
 * If no height data is given the height of the entity the mod belongs to will
 * be used. If however a "height" value is set, that will be used instead.
 * If no "height" value is set, but a "heightoffset" is present, that value
 * will be added to the height set by the position of the entity the mod
 * belongs to.
 * @param pos Position of the mod entity
 * @param modElement Atlas data describing the mod
 * @return The adjusted height of the mod
 */
float TerrainModTranslator::parsePosition(
      const WFMath::Point<3> & pos,
      const MapType& modElement)
{
    ///If the height is specified use that, else check for a height offset. If none is found, use the default height of the entity position
    MapType::const_iterator I = modElement.find("height");
    if (I != modElement.end()) {
        const Element& modHeightElem = I->second;
        if (modHeightElem.isNum()) {
            return modHeightElem.asNum();
        }
    } else {
        I = modElement.find("heightoffset");
        if (I != modElement.end()) {
            const Element& modHeightElem = I->second;
            if (modHeightElem.isNum()) {
                float heightoffset = modHeightElem.asNum();
                return pos.z() + heightoffset;
            }
        }
    }
    return pos.z();
}

/**
 * @brief Common method for parsing shape data from Atlas.
 * Since each different shape expects different Atlas data this is a
 * templated method with specialized implemtations for each available shape.
 * If you call this and get error regarding missing implementations it
 * probably means that there's no implementation for the type of shape you're
 * calling it with. Note that a new shape instance will be put on the heap if
 * the parsing is successful, and it's up to the calling code to properly
 * delete it when done.
 * @param shapeElement The atlas map element which contains the shape data.
 * Often this is found with the key "shape" in the atlas data.
 * @param pos The original position of the entity to which this shape will
 * belong. The shape will be positioned according to this.
 * @param shape The resulting shape is meant to be put here, if successfully
 * created. That means that a new shape instance will be created, and it's
 * then up to the calling method to properly delete it, to avoid memory leaks.
 * @return True if the atlas data was successfully parsed and a shape was
 * created.
 */
template<class Shape>
bool TerrainModTranslator::parseShape(
      const Element& shapeElement,
      const WFMath::Point<3>& pos,
      const WFMath::Quaternion& orientation,
      Shape & shape)
{
    if (boostGeometryFromMessage(shapeElement, shape) != 0)
    {
        ///Just log an error and return false, this isn't fatal.
        log(WARNING, "Error when parsing shape from atlas.");
        return false;
    }

#if 0
    if (orientation.isValid()) {
        /// rotation about Z axis
        WFMath::Vector<3> xVec = WFMath::Vector<3>(1.0, 0.0, 0.0).rotate(orientation);
        WFMath::CoordType theta = std::atan2(xVec.y(), xVec.x());
        WFMath::RotMatrix<2> rm;
        shape.rotatePoint(rm.rotation(theta), WFMath::Point<2>(0, 0));
    }

    shape.shift(WFMath::Vector<2>(pos.x(), pos.y()));
#endif
    return true;
}

/**
 * @brief Create or update an instance from the passed in atlas data.
 * @param shape The modified shape of the mod
 * @param pos Position of the mod entity
 * @return True if the atlas data could be successfully parsed
 */
template <template <class Shape> class Mod,
          class Shape>
bool TerrainModTranslator::createInstance(
      Shape & shape,
      const WFMath::Point<3>& pos,
      const MapType& modElement,
      float ,
      float )
{
    float level = parsePosition(pos, modElement);
    MapType::const_iterator I = modElement.find("slopes");
    if (I == modElement.end()) {
        log(ERROR, "SlopeTerrainMod defined without slopes");
        return false;
    }
    const Element& modSlopeElem = I->second;
    if (!modSlopeElem.isList()) {
        log(ERROR, "SlopeTerrainMod defined with malformed slopes");
        return false;
    }
    const ListType & slopes = modSlopeElem.asList();
    if (slopes.size() < 2 || !slopes[0].isNum() || !slopes[1].isNum()) {
        log(ERROR, "SlopeTerrainMod defined without slopes");
        return false;
    }
    const float dx = slopes[0].asNum();
    const float dy = slopes[1].asNum();
    if (m_mod != 0) {
        Mod<Shape> * mod = dynamic_cast<Mod<Shape>*>(m_mod);
        if (mod != 0) {
            mod->setShape(level, dx, dy, shape);
            return true;
        }
    }
    m_mod = new Mod<Shape>(level, dx, dy, shape);
    return true;
}

/**
 * @brief Create or update an instance from the passed in atlas data.
 * @param shape The modified shape of the mod
 * @param pos Position of the mod entity
 * @return True if the atlas data could be successfully parsed
 */
template <template <class S> class Mod,
          class Shape>
bool TerrainModTranslator::createInstance(
      Shape & shape,
      const WFMath::Point<3>& pos,
      const MapType& modElement)
{
    float level = parsePosition(pos, modElement);
    if (m_mod != 0) {
        Mod<Shape> * mod = dynamic_cast<Mod<Shape>*>(m_mod);
        if (mod != 0) {
            mod->setShape(level, shape);
            return true;
        }
    }
    m_mod = new Mod<Shape>(level, shape);
    return true;
}
