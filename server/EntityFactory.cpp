// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2000-2005 Alistair Riddoch

#include <Python.h>

#include "EntityFactory.h"

#include "PersistantThingFactory.h"
#include "ScriptFactory.h"
#include "Persistance.h"
#include "Persistor.h"
#include "Player.h"

#include "rulesets/Thing.h"
#include "rulesets/MindFactory.h"
#include "rulesets/Character.h"
#include "rulesets/Creator.h"
#include "rulesets/Plant.h"
#include "rulesets/Food.h"
#include "rulesets/Stackable.h"
#include "rulesets/Structure.h"
#include "rulesets/Line.h"
#include "rulesets/Area.h"
#include "rulesets/World.h"

#include "rulesets/Python_Script_Utils.h"

#include "common/id.h"
#include "common/log.h"
#include "common/debug.h"
#include "common/globals.h"
#include "common/const.h"
#include "common/inheritance.h"
#include "common/AtlasFileLoader.h"
#include "common/random.h"
#include "common/compose.hpp"

#include <Atlas/Message/Element.h>
#include <Atlas/Objects/Entity.h>
#include <Atlas/Objects/RootOperation.h>

using Atlas::Message::Element;
using Atlas::Message::MapType;
using Atlas::Message::ListType;
using Atlas::Objects::Entity::RootEntity;

static const bool debug_flag = false;

EntityFactory * EntityFactory::m_instance = NULL;

EntityFactory::EntityFactory(BaseWorld & w) : m_world(w),
             m_eft(new PersistantThingFactory<Entity>())
{
    installFactory("game_entity", "world", new ForbiddenThingFactory<World>());
    PersistantThingFactory<Thing> * tft = new PersistantThingFactory<Thing>();
    installFactory("game_entity", "thing", tft);
    installFactory("thing", "feature", tft->duplicateFactory());
    installFactory("feature", "line", new PersistantThingFactory<Line>());
    installFactory("feature", "area", new PersistantThingFactory<Area>());
    installFactory("thing", "character",
                   new PersistantThingFactory<Character>());
    installFactory("character", "creator",
                   new PersistantThingFactory<Creator>());
    installFactory("thing", "plant", new PersistantThingFactory<Plant>());
    installFactory("thing", "food", new PersistantThingFactory<Food>());
    installFactory("thing", "stackable",
                   new PersistantThingFactory<Stackable>());
    installFactory("thing", "structure",
                   new PersistantThingFactory<Structure>());
}

void EntityFactory::initWorld()
{
    FactoryDict::const_iterator I = m_factories.find("world");
    if (I == m_factories.end()) {
        log(CRITICAL, "No world factory");
        return;
    }
    ForbiddenThingFactory<World> * wft = dynamic_cast<ForbiddenThingFactory<World> *>(I->second);
    if (wft == 0) {
        log(CRITICAL, "Its not a world factory");
        return;
    }
    wft->m_p.persist((World&)m_world.m_gameWorld);
}

Entity * EntityFactory::newEntity(const std::string & id, long intId,
                                  const std::string & type,
                                  const RootEntity & attributes)
{
    debug(std::cout << "EntityFactor::newEntity()" << std::endl << std::flush;);
    Entity * thing = 0;
    FactoryDict::const_iterator I = m_factories.find(type);
    PersistorBase * pc = 0;
    if (I == m_factories.end()) {
        return 0;
    }
    FactoryBase * factory = I->second;
    if (consts::enable_persistence) {
        thing = factory->newPersistantThing(id, intId, &pc);
    } else {
        thing = factory->newThing(id, intId);
    }
    if (thing == 0) {
        return 0;
    }
    debug( std::cout << "[" << type << " " << thing->getName() << "]"
                     << std::endl << std::flush;);
    thing->setType(type);
    // Sort out python object
    if (factory->m_scriptFactory != 0) {
        debug(std::cout << "Class " << type << " has a python class"
                        << std::endl << std::flush;);
        factory->m_scriptFactory->addScript(thing);
    }

    // Read the defaults
    thing->merge(factory->m_attributes);
    // And then override with the values provided for this entity.
    thing->merge(attributes->asMessage());
    // Get location from entity, if it is present
    // The default attributes cannot contain info on location
    if (attributes->hasAttrFlag(Atlas::Objects::Entity::LOC_FLAG)) {
        const std::string & loc_id = attributes->getLoc();
        if (integerIdCheck(loc_id) == 0) {
            thing->m_location.m_loc = m_world.getEntity(loc_id);
        } else {
            log(ERROR, "Non int IG ID");
        }
    }
    if (thing->m_location.m_loc == 0) {
        // If no info was provided, put the entity in the game world
        thing->m_location.m_loc = &m_world.m_gameWorld;
    }
    thing->m_location.readFromEntity(attributes);
    if (!thing->m_location.pos().isValid()) {
        // If no position coords were provided, put it somewhere near origin
        thing->m_location.m_pos = Point3D(uniform(-8,8), uniform(-8,8), 0);
    }
    if (thing->m_location.velocity().isValid()) {
        if (attributes->hasAttrFlag(Atlas::Objects::Entity::VELOCITY_FLAG)) {
            log(ERROR, String::compose("EntityFactory::newEntity(%1, %2): Entity has velocity set from the attributes given by the creator", id, type).c_str());
        } else {
            log(ERROR, String::compose("EntityFactory::newEntity(%1, %2): Entity has velocity set from an unknown source", id, type).c_str());
        }
        thing->m_location.m_velocity.setValid(false);
    }
    if (pc != 0) {
        pc->persist();
        thing->clearUpdateFlags();
    }
    delete pc;
    return thing;
}

void EntityFactory::flushFactories()
{
    FactoryDict::const_iterator Iend = m_factories.end();
    for (FactoryDict::const_iterator I = m_factories.begin(); I != Iend; ++I) {
        delete I->second;
    }
    delete m_eft;
    m_eft = NULL;
    m_factories.clear();
}

void EntityFactory::populateFactory(const std::string & className,
                                    FactoryBase * factory,
                                    const MapType & classDesc)
{
    // Establish whether this rule has an associated script, and
    // if so, use it.
    MapType::const_iterator J = classDesc.find("script");
    MapType::const_iterator Jend = classDesc.end();
    if ((J != Jend) && (J->second.isMap())) {
        const MapType & script = J->second.asMap();
        J = script.find("name");
        if ((J != script.end()) && (J->second.isString())) {
            const std::string & scriptName = J->second.String();
            J = script.find("language");
            if ((J != script.end()) && (J->second.isString())) {
                const std::string & scriptLanguage = J->second.String();
                if (factory->m_scriptFactory != 0) {
                    if (factory->m_scriptFactory->package() != scriptName) {
                        delete factory->m_scriptFactory;
                        factory->m_scriptFactory = 0;
                    }
                }
                if (factory->m_scriptFactory == 0) {
                    if (scriptLanguage == "python") {
                        factory->m_scriptFactory = new PythonScriptFactory(scriptName, className);
                    } else {
                        log(ERROR, "Unknown script language.");
                    }
                }
            }
        }
    }

    // Establish whether this rule has an associated mind rule,
    // and handle it.
    J = classDesc.find("mind");
    if ((J != Jend) && (J->second.isMap())) {
        const MapType & script = J->second.asMap();
        J = script.find("name");
        if ((J != script.end()) && (J->second.isString())) {
            const std::string & mindType = J->second.String();
            // language is unused. might need it one day
            // J = script.find("language");
            // if ((J != script.end()) && (J->second.isString())) {
                // const std::string & mindLang = J->second.String();
            // }
            MindFactory::instance()->addMindType(className, mindType);
        }
    }

    // Store the default attribute for entities create by this rule.
    J = classDesc.find("attributes");
    if ((J != Jend) && (J->second.isMap())) {
        const MapType & attrs = J->second.asMap();
        MapType::const_iterator Kend = attrs.end();
        for (MapType::const_iterator K = attrs.begin(); K != Kend; ++K) {
            if (!K->second.isMap()) {
                log(ERROR, "Attribute description in rule is not a map.");
                continue;
            }
            const MapType & attr = K->second.asMap();
            MapType::const_iterator L = attr.find("default");
            if (L != attr.end()) {
                // Store this value in the defaults for this class
                factory->m_classAttributes[K->first] = L->second;
                // and merge it with the defaults inherited from the parent
                factory->m_attributes[K->first] = L->second;
            }
        }
    }

    // Check whether it should be available to players as a playable character.
    J = classDesc.find("playable");
    if ((J != Jend) && (J->second.isInt())) {
        Player::playableTypes.insert(className);
    }
}

int EntityFactory::installEntityClass(const std::string & className,
                                      const std::string & parent,
                                      const MapType & classDesc)
{
    // Get the new factory for this rule
    FactoryDict::const_iterator I = m_factories.find(parent);
    if (I == m_factories.end()) {
        debug(std::cout << "class \"" << className
                        << "\" has non existant parent \"" << parent
                        << "\". Waiting." << std::endl << std::flush;);
        m_waitingRules.insert(make_pair(parent, make_pair(className, classDesc)));
        return 1;
    }
    FactoryBase * parent_factory = I->second;
    FactoryBase * factory = parent_factory->duplicateFactory();
    if (factory == 0) {
        log(ERROR, String::compose("Attempt to install rule \"%1\" which has parent \"%2\" which cannot be instantiated", className, parent).c_str());
        return -1;
    }

    assert(factory->m_parent == parent_factory);

    // Copy the defaults from the parent. In populateFactory this may be
    // overriden with the defaults for this class.
    factory->m_attributes = parent_factory->m_attributes;

    populateFactory(className, factory, classDesc);

    debug(std::cout << "INSTALLING " << className << ":" << parent
                    << std::endl << std::flush;);

    // Install the factory in place.
    installFactory(parent, className, factory);

    // Add it as a child to its parent.
    parent_factory->m_children.insert(factory);

    return 0;
}

int EntityFactory::installOpDefinition(const std::string & opDefName,
                                       const std::string & parent,
                                       const MapType & opDefDesc)
{
    Inheritance & i = Inheritance::instance();

    if (!i.hasClass(parent)) {
        debug(std::cout << "op_definition \"" << opDefName
                        << "\" has non existant parent \"" << parent
                        << "\". Waiting." << std::endl << std::flush;);
        m_waitingRules.insert(make_pair(parent, make_pair(opDefName, opDefDesc)));
        return 1;
    }

    Atlas::Objects::Root r = atlasOpDefinition(opDefName, parent);

    if (i.addChild(r) != 0) {
        return -1;
    }

    int op_no = Atlas::Objects::Factories::instance()->addFactory(opDefName, &Atlas::Objects::generic_factory);
    i.opInstall(opDefName, op_no);

    return 0;
}

int EntityFactory::installRule(const std::string & className,
                               const MapType & classDesc)
{
    MapType::const_iterator J = classDesc.find("objtype");
    MapType::const_iterator Jend = classDesc.end();
    if (J == Jend || !J->second.isString()) {
        std::string msg = std::string("Rule \"") + className 
                          + "\" has no objtype. Skipping.";
        log(ERROR, msg.c_str());
        return -1;
    }
    const std::string & objtype = J->second.String();
    J = classDesc.find("parents");
    if (J == Jend) {
        std::string msg = std::string("Rule \"") + className 
                          + "\" has no parents. Skipping.";
        log(ERROR, msg.c_str());
        return -1;
    }
    if (!J->second.isList()) {
        std::string msg = std::string("Rule \"") + className 
                          + "\" has malformed parents. Skipping.";
        log(ERROR, msg.c_str());
        return -1;
    }
    const ListType & parents = J->second.asList();
    if (parents.empty()) {
        std::string msg = std::string("Rule \"") + className 
                          + "\" has empty parents. Skipping.";
        log(ERROR, msg.c_str());
        return -1;
    }
    const Element & p1 = parents.front();
    if (!p1.isString() || p1.String().empty()) {
        std::string msg = std::string("Rule \"") + className 
                          + "\" has malformed first parent. Skipping.";
        log(ERROR, msg.c_str());
        return -1;
    }
    const std::string & parent = p1.String();
    if (objtype == "class") {
        int ret = installEntityClass(className, parent, classDesc);
        if (ret != 0) {
            return ret;
        }
    } else if (objtype == "op_definition") {
        int ret = installOpDefinition(className, parent, classDesc);
        if (ret != 0) {
            return ret;
        }
    } else {
        std::string msg = std::string("Rule \"") + className 
                          + "\" has unknown objtype=\"" + objtype
                          + "\". Skipping.";
        log(ERROR, msg.c_str());
        return -1;
    }

    // Install any rules that were waiting for this rule before they
    // could be installed
    RuleWaitList::iterator I = m_waitingRules.lower_bound(className);
    for (; I != m_waitingRules.upper_bound(className); ++I) {
        const std::string & wClassName = I->second.first;
        const MapType & wClassDesc = I->second.second;
        debug(std::cout << "WAITING rule " << wClassName
                        << " now ready" << std::endl << std::flush;);
        installRule(wClassName, wClassDesc);
    }
    m_waitingRules.erase(className);
    return 0;
}

static void updateChildren(FactoryBase * factory)
{
    std::set<FactoryBase *>::const_iterator I = factory->m_children.begin();
    std::set<FactoryBase *>::const_iterator Iend = factory->m_children.end();
    for (; I != Iend; ++I) {
        FactoryBase * child_factory = *I;
        child_factory->m_attributes = factory->m_attributes;
        MapType::const_iterator J = child_factory->m_classAttributes.begin();
        MapType::const_iterator Jend = child_factory->m_classAttributes.end();
        for (; J != Jend; ++J) {
            child_factory->m_attributes[J->first] = J->second;
        }
        updateChildren(child_factory);
    }
}

int EntityFactory::modifyRule(const std::string & className,
                              const MapType & classDesc)
{
    FactoryDict::const_iterator I = m_factories.find(className);
    if (I == m_factories.end()) {
        log(ERROR, "Could not find factory for existing type.");
        return -1;
    }
    FactoryBase * factory = I->second;
    assert(factory != 0);
    
    ScriptFactory * script_factory = factory->m_scriptFactory;
    if (script_factory != 0) {
        script_factory->refreshClass();
    }

    // Copy the defaults from the parent. In populateFactory this may be
    // overriden with the defaults for this class.
    // FIXME
    // If the code crashes here because m_parent is NULL, it is because
    // the client has attempted to modify the factory for a core class.
    factory->m_attributes = factory->m_parent->m_attributes;

    populateFactory(className, factory, classDesc);

    updateChildren(factory);

    return 0;
}

void EntityFactory::getRulesFromFiles(MapType & rules)
{
    std::vector<std::string>::const_iterator I = rulesets.begin();
    std::vector<std::string>::const_iterator Iend = rulesets.end();
    for (; I != Iend; ++I) {
        std::string filename = etc_directory + "/cyphesis/" + *I + ".xml";
        AtlasFileLoader f(filename, rules);
        if (!f.isOpen()) {
            log(ERROR, "Unable to open rule file.");
        }
        f.read();
    }
}

void EntityFactory::installRules()
{
    MapType ruleTable;

    if (consts::enable_database) {
        Persistance * p = Persistance::instance();
        p->getRules(ruleTable);
    } else {
        getRulesFromFiles(ruleTable);
    }

    if (ruleTable.empty()) {
        log(ERROR, "Rule database table contains no rules.");
        if (consts::enable_database) {
            log(NOTICE, "Attempting to load temporary ruleset from files.");
            getRulesFromFiles(ruleTable);
        }
    }

    MapType::const_iterator Iend = ruleTable.end();
    for (MapType::const_iterator I = ruleTable.begin(); I != Iend; ++I) {
        const std::string & className = I->first;
        const MapType & classDesc = I->second.asMap();
        installRule(className, classDesc);
    }
    // Report on the non-cleared rules.
    // Perhaps we can keep them too?
    // m_waitingRules.clear();
    RuleWaitList::const_iterator J = m_waitingRules.begin();
    RuleWaitList::const_iterator Jend = m_waitingRules.end();
    for (; J != Jend; ++J) {
        const std::string & wParentName = J->first;
        log(ERROR, String::compose("Rule \"%1\" with parent \"%2\" is an orphan", J->second.first, wParentName).c_str());
    }
}

void EntityFactory::installFactory(const std::string & parent,
                                   const std::string & className,
                                   FactoryBase * factory)
{
    assert(factory != 0);

    m_factories[className] = factory;

    Inheritance & i = Inheritance::instance();

    i.addChild(atlasClass(className, parent));
}

FactoryBase * EntityFactory::getNewFactory(const std::string & parent)
{
    FactoryDict::const_iterator I = m_factories.find(parent);
    if (I == m_factories.end()) {
        return 0;
    }
    return I->second->duplicateFactory();
}
