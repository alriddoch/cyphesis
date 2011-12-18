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

#include <Python.h>

#include "rulesets/PythonClass.h"

#include <cassert>

class TestPythonClass : public PythonClass {
  public:
    TestPythonClass(const std::string & p, const std::string & t) :
          PythonClass(p, t, &PyBaseObject_Type) { }

    virtual ~TestPythonClass() { }

    virtual int check() const { return 0; }

    int test_getClass(PyObject * o) { return getClass(o); }
    int tet_load() { return load(); }
    int test_refresh() { return refresh(); }

    const std::string & access_package() { return m_package; }
    const std::string & access_type() { return m_type; }

    struct _object * access_module() { return m_module; }
    struct _object * access_class() { return m_class; }
};

int main()
{
    {
        const char * package = "acfd44fd-dccb-4a63-98c3-6facd580ca5f";
        const char * type = "3265e96a-28a0-417c-ad30-2970c1777c50";

        TestPythonClass * pc = new TestPythonClass(package, type);

        assert(pc != 0);

        assert(pc->access_package() == package);
        assert(pc->access_type() == type);

        assert(pc->access_module() == 0);
        assert(pc->access_class() == 0);

        delete pc;
    }
    return 0;
}

// stubs

#include "common/log.h"

#include "rulesets/Python_Script_Utils.h"

void log(LogLevel lvl, const std::string & msg)
{
}

struct _object * Get_PyClass(struct _object * module,
                             const std::string & package,
                             const std::string & type)
{
    return 0;
}

struct _object * Get_PyModule(const std::string & package)
{
    return 0;
}