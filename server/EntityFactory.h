// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2000,2001 Alistair Riddoch

#ifndef SERVER_ENTITY_FACTORY_H
#define SERVER_ENTITY_FACTORY_H

#include "common/types.h"

#include <sigc++/object.h>

namespace Atlas { namespace Message {
  class Object;
} }

class Entity;
class FactoryBase;
class BaseWorld;

template <class T>
class PersistantThingFactory;

typedef std::map<std::string, FactoryBase *> FactoryDict;

class EntityFactory {
  private:
    explicit EntityFactory(BaseWorld & w);
    static EntityFactory * m_instance;

    FactoryDict m_factories;
    BaseWorld & m_world;
    PersistantThingFactory<Entity> * m_eft;

    void installBaseClasses();
  public:
    static void init(BaseWorld & w) {
        m_instance = new EntityFactory(w);
        m_instance->installBaseClasses();
    }
    static EntityFactory * instance() {
        return m_instance;
    }
    static void del() {
        if (m_instance != NULL) {
            delete m_instance;
        }
    }
    Entity * newEntity(const std::string &, const std::string &,
                       const Atlas::Message::Element::MapType &);
    void flushFactories();

    void installFactory(const std::string &, const std::string &, FactoryBase*);
    void installClass(const std::string &, const std::string&);
    FactoryBase * getFactory(const std::string &);
};

#endif // SERVER_ENTITY_FACTORY_H
