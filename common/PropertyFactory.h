// Cyphesis Online RPG Server and AI Engine
// Copyright (C) 2006 Alistair Riddoch
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

#ifndef COMMON_PROPERTY_FACTORY_H
#define COMMON_PROPERTY_FACTORY_H

class PropertyBase;

template<class T>
class ScriptKit;

/// \brief Kit interface for factories to create Property objects.
class PropertyKit {
  protected:
    // FIXME Replace with initializer
    PropertyKit() : m_scriptFactory(0) { }
  public:
    ScriptKit<PropertyBase> * m_scriptFactory;

    virtual ~PropertyKit() = 0;

    virtual PropertyBase * newProperty() = 0;

    virtual PropertyKit * duplicateFactory() const = 0;

    virtual PropertyKit * scriptPropertyFactory() const = 0;
};

/// \brief Factory template to create custom Property objects.
template <class T>
class PropertyFactory : public PropertyKit {
  public:
    typedef T PropertyT;

    virtual PropertyBase * newProperty();
    virtual PropertyFactory<T> * duplicateFactory() const;
    virtual PropertyKit * scriptPropertyFactory() const;
};

template <typename T>
class Property;

/// \brief Factory template to create standard Property objects.
template <typename Q>
class PropertyFactory<Property<Q>> : public PropertyKit {
  public:
    typedef Property<Q> PropertyT;

    virtual PropertyBase * newProperty();
    virtual PropertyFactory<Property<Q>> * duplicateFactory() const;
    virtual PropertyKit * scriptPropertyFactory() const;
};

#endif // COMMON_PROPERTY_FACTORY_H
