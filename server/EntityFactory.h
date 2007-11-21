// Cyphesis Online RPG Server and AI Engine
// Copyright (C) 2000,2001 Alistair Riddoch
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

// $Id: EntityFactory.h,v 1.53 2007-11-21 22:36:48 alriddoch Exp $

#ifndef SERVER_ENTITY_FACTORY_H
#define SERVER_ENTITY_FACTORY_H

#include <Atlas/Message/Element.h>
#include <Atlas/Objects/Root.h>
#include <Atlas/Objects/SmartPtr.h>
#include <Atlas/Objects/ObjectsFwd.h>

class Entity;
class Character;
class Task;
class FactoryBase;
class TaskFactory;
class ArithmeticFactory;
class BaseWorld;

template <class T>
class PersistantThingFactory;

typedef std::map<std::string, FactoryBase *> FactoryDict;
typedef std::map<std::string, TaskFactory *> TaskFactoryDict;
typedef std::multimap<std::string, TaskFactory *> TaskFactoryMultimap;
typedef std::map<std::string, TaskFactoryMultimap> TaskFactoryActivationDict;
typedef std::map<std::string, ArithmeticFactory *> StatisticsFactoryDict;


/// \brief Class to handle rules that cannot yet be installed, and the reason
class RuleWaiting {
  public:
    /// Name of the rule.
    std::string name;
    /// Complete description of the rule.
    Atlas::Objects::Root desc;
    /// Message giving a description of why this rule has not been installed.
    std::string reason;
};

typedef std::multimap<std::string, RuleWaiting> RuleWaitList;

/// \brief Class to handle the creation of all entities for the world.
///
/// Uses PersistantThingFactory to store information about entity types, and
/// create them. Handles connecting entities to their persistor as required.
class EntityFactory {
  protected:
    explicit EntityFactory(BaseWorld & w);
    ~EntityFactory();
    static EntityFactory * m_instance;

    FactoryDict m_factories;
    TaskFactoryDict m_taskFactories;
    TaskFactoryActivationDict m_taskActivations;
    StatisticsFactoryDict m_statisticsFactories;

    BaseWorld & m_world;
    RuleWaitList m_waitingRules;

    void getRulesFromFiles(std::map<std::string, Atlas::Objects::Root> &);
    void installRules();
    void installFactory(const std::string & parent,
                        const std::string & class_name,
                        FactoryBase * factory,
                        Atlas::Objects::Root classDesc = 0);
    int populateFactory(const std::string & class_name,
                        FactoryBase * factory,
                        const Atlas::Message::MapType & classDesc);
    FactoryBase * getNewFactory(const std::string & parent);
    bool isTask(const std::string & class_name);
    int installTaskClass(const std::string & class_name,
                         const std::string & parent,
                         const Atlas::Objects::Root & class_desc);
    int installEntityClass(const std::string & class_name,
                           const std::string & parent,
                           const Atlas::Objects::Root&);
    int installOpDefinition(const std::string & class_name,
                            const std::string & parent,
                            const Atlas::Objects::Root & class_desc);

    int modifyTaskClass(const std::string & class_name,
                        const Atlas::Objects::Root & class_desc);
    int modifyEntityClass(const std::string & class_name,
                          const Atlas::Objects::Root & class_desc);
    int modifyOpDefinition(const std::string & class_name,
                           const Atlas::Objects::Root & class_desc);

    void waitForRule(const std::string & class_name,
                     const Atlas::Objects::Root & class_desc,
                     const std::string & dependent,
                     const std::string & reason);
  public:
    static void init(BaseWorld & w) {
        m_instance = new EntityFactory(w);
        m_instance->installRules();
    }
    static EntityFactory * instance() {
        return m_instance;
    }
    static void del() {
        if (m_instance != 0) {
            delete m_instance;
            m_instance = 0;
        }
    }
    void initWorld();

    Entity * newEntity(const std::string &, long, const std::string &,
                       const Atlas::Objects::Entity::RootEntity &) const;
    void flushFactories();

    Task * newTask(const std::string &, Character &) const;
    Task * activateTask(const std::string &, const std::string &,
                        const std::string &, Character &) const;
    int addStatisticsScript(Character &) const;

    int installRule(const std::string &, const Atlas::Objects::Root &);
    int modifyRule(const std::string &, const Atlas::Objects::Root &);
};

#endif // SERVER_ENTITY_FACTORY_H
