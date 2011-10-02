// Cyphesis Online RPG Server and AI Engine
// Copyright (C) 2011 Alistair Riddoch
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

#ifdef NDEBUG
#undef NDEBUG
#endif
#ifndef DEBUG
#define DEBUG
#endif

#include "server/EntityRuleHandler.h"

#include "server/EntityBuilder.h"

#include "common/TypeNode.h"

#include <Atlas/Objects/Anonymous.h>

#include <cstdlib>

using Atlas::Message::MapType;
using Atlas::Objects::Root;
using Atlas::Objects::Entity::Anonymous;

static TypeNode * stub_addChild_result = 0;
static bool stub_isTask_result = false;

int main()
{
    EntityBuilder::init();

    {
        RuleHandler * rh = new EntityRuleHandler(EntityBuilder::instance());
        delete rh;
    }

    // check() not a class
    {
        RuleHandler * rh = new EntityRuleHandler(EntityBuilder::instance());

        Anonymous description;
        description->setParents(std::list<std::string>(1, "foo"));
        int ret = rh->check(description);

        assert(ret == -1);

        delete rh;
    }

    // check() stub says it's a task
    {
        RuleHandler * rh = new EntityRuleHandler(EntityBuilder::instance());

        stub_isTask_result = true;

        Anonymous description;
        description->setObjtype("class");
        description->setParents(std::list<std::string>(1, "foo"));
        int ret = rh->check(description);

        assert(ret == -1);

        stub_isTask_result = false;

        delete rh;
    }

    // check() stub says it's not a task
    {
        RuleHandler * rh = new EntityRuleHandler(EntityBuilder::instance());

        Anonymous description;
        description->setObjtype("class");
        description->setParents(std::list<std::string>(1, "foo"));
        int ret = rh->check(description);

        assert(ret == 0);

        delete rh;
    }

    {
        RuleHandler * rh = new EntityRuleHandler(EntityBuilder::instance());

        Anonymous description;
        description->setId("class_name");
        std::string dependent, reason;

        int ret = rh->install("class_name", "parent_name",
                              description, dependent, reason);

        assert(ret == 1);
        assert(dependent == "parent_name");

        delete rh;
    }

    // Install a rule with addChild rigged to give a correct result
    {
        RuleHandler * rh = new EntityRuleHandler(EntityBuilder::instance());

        Anonymous description;
        description->setId("class_name");
        std::string dependent, reason;

        stub_addChild_result = (TypeNode *) malloc(sizeof(TypeNode));
        int ret = rh->install("class_name", "parent_name",
                              description, dependent, reason);

        assert(ret == 1);
        assert(dependent == "parent_name");

        free(stub_addChild_result);
        stub_addChild_result = 0;

        delete rh;
    }
    {
        RuleHandler * rh = new EntityRuleHandler(EntityBuilder::instance());

        Anonymous description;
        int ret = rh->update("", description);

        // FIXME Currently does nothing
        assert(ret == -1);

        delete rh;
    }

 
}

// stubs

#include "server/EntityFactory.h"
#include "server/Player.h"
#include "server/ScriptFactory.h"

#include "rulesets/MindFactory.h"

#include "common/Inheritance.h"
#include "common/log.h"

std::set<std::string> Player::playableTypes;

EntityBuilder * EntityBuilder::m_instance = NULL;

EntityBuilder::EntityBuilder()
{
}

bool EntityBuilder::isTask(const std::string & class_name)
{
    return stub_isTask_result;
}

EntityKit * EntityBuilder::getClassFactory(const std::string & class_name)
{
    return 0;
}

void EntityBuilder::installFactory(const std::string & class_name,
                                   const std::string & parent,
                                   EntityKit * factory)
{
}

ScriptKit::ScriptKit(const std::string & package,
                     const std::string & type) : m_package(package),
                                                 m_type(type)
{
}

ScriptKit::~ScriptKit()
{
}

PythonScriptFactory::PythonScriptFactory(const std::string & package,
                                         const std::string & type) :
                                         ScriptKit(package, type),
                                         m_module(0), m_class(0)
{
}

PythonScriptFactory::~PythonScriptFactory()
{
}

int PythonScriptFactory::addScript(Entity * entity)
{
    return 0;
}

int PythonScriptFactory::refreshClass()
{
    return 0;
}

void EntityKit::updateChildren()
{
}

void EntityKit::updateChildrenProperties()
{
}

void EntityKit::addProperties()
{
}

MindFactory * MindFactory::m_instance = NULL;

MindFactory::MindFactory()
{
}

Inheritance * Inheritance::m_instance = NULL;

Inheritance::Inheritance()
{
}

Inheritance & Inheritance::instance()
{
    if (m_instance == NULL) {
        m_instance = new Inheritance();
    }
    return *m_instance;
}

TypeNode * Inheritance::addChild(const Root & obj)
{
    return stub_addChild_result;
}

bool Inheritance::hasClass(const std::string & parent)
{
    return true;
}

Root atlasOpDefinition(const std::string & name, const std::string & parent)
{
    return Root();
}

void log(LogLevel lvl, const std::string & msg)
{
}

