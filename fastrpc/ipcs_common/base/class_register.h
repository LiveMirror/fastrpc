/// Jack
#ifndef IPCS_COMMON_BASE_CLASS_REGISTER_H
#define IPCS_COMMON_BASE_CLASS_REGISTER_H

// Defines several helper macros for registering class and its singleton by a
// string name and creating/retrieving them later per the registered name.
// The motivation is to help implement the factory class. C++ doesn't support
// reflection so we define several macros to do this.
//
// All macros defined here are NOT used by final user directly, and they are
// used to create register macros for a specific base class. Here is an example:
/*
   mapper.h (the interface definition):
   #include "class_register.h"
   class Mapper {
   };

   CLASS_REGISTER_DEFINE_REGISTRY(mapper_register, Mapper);

   #define REGISTER_MAPPER(mapper_name) \
       CLASS_REGISTER_OBJECT_CREATOR( \
           mapper_register, Mapper, #mapper_name, mapper_name) \

   #define CREATE_MAPPER(mapper_name_as_string) \
       CLASS_REGISTER_CREATE_OBJECT(mapper_register, mapper_name_as_string)

   hello_mapper.cc (an implementation of Mapper):
   #include "mapper.h"
   class HelloMapper : public Mapper {
   };
   REGISTER_MAPPER(HelloMapper);

   mapper_user.cc (the final user of all registered mappers):
   #include "mapper.h"
   Mapper* mapper = CREATE_MAPPER("HelloMapper");
*/
// Another usage is to register class by an arbitrary string instead of its
// class name, and register a singleton instance for each registered name.
// Here is an example:
/*
   file_impl.h (the interface definition):
   class FileImpl {
   };

   CLASS_REGISTER_DEFINE_REGISTRY(file_impl_register, FileImpl);

   #define REGISTER_DEFAULT_FILE_IMPL(path_prefix_as_string, file_impl_name) \
       CLASS_REGISTER_DEFAULT_OBJECT_CREATOR_WITH_SINGLETON( \
           file_impl_register, FileImpl, path_prefix_as_string, file_impl_name)

   #define REGISTER_FILE_IMPL(path_prefix_as_string, file_impl_name) \
       CLASS_REGISTER_OBJECT_CREATOR_WITH_SINGLETON( \
           file_impl_register, FileImpl, path_prefix_as_string, file_impl_name)

   #define CREATE_FILE_IMPL(path_prefix_as_string) \
       CLASS_REGISTER_CREATE_OBJECT(file_impl_register, path_prefix_as_string)

   #define GET_FILE_IMPL_SINGLETON(path_prefix_as_string) \
       CLASS_REGISTER_GET_SINGLETON(file_impl_register, path_prefix_as_string)

   #define FILE_IMPL_COUNT() \
       CLASS_REGISTER_CREATOR_COUNT(file_impl_register)

   #define FILE_IMPL_NAME(i) \
       CLASS_REGISTER_CREATOR_NAME(file_impl_register, i)

   local_file.cc (an implementation of FileImpl):
   #include "file.h"
   class LocalFileImpl : public FileImpl {
   };
   REGISTER_FILE_IMPL("/local", LocalFileImpl);

   file_user.cc (the final user of all registered file implementations):
   #include "file_impl.h"
   FileImpl* file_impl = CREATE_FILE_IMPL("/local");
   FileImpl* file_impl_singleton = GET_FILE_IMPL_SINGLETON("/local");
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <map>
#include <string>
#include <vector>

#include "preprocess.h"

// ClassRegistry manage (name -> creator/singleton) mapping.
// One base class may have multiple registry instance, distinguished by the
// register_name.
template <typename BaseClassName>
class ClassRegistry {
public:
    typedef BaseClassName* (*Creator)();
    typedef BaseClassName* (*Singleton)();

private:
    struct ClassInfo {
        Creator creator;
        Singleton singleton;
    };
    typedef std::map<std::string, ClassInfo> ClassMap;

public:
    ClassRegistry() {}
    ~ClassRegistry() {}

    void AddCreator(const std::string& entry_name,
                    Creator creator,
                    Singleton singleton = NULL) {
        typename ClassMap::iterator it = m_creator_map.find(entry_name);
        if (it != m_creator_map.end()) {
            fprintf(stderr,
                    "ClassRegister: class %s already registered. "
                    "One possibility: Duplicated class definition "
                    "(maybe the same source file) is being linked into "
                    "more than one shared libraries or both shared "
                    "library and executable\n",
                    entry_name.c_str());
            abort();
        }
        ClassInfo& info = m_creator_map[entry_name];
        info.creator = creator;
        info.singleton = singleton;
        m_creator_names.push_back(entry_name);
    }

    size_t CreatorCount() const {
        return m_creator_names.size();
    }

    const std::string& CreatorName(size_t i) const {
        assert(i < m_creator_names.size());
        return m_creator_names[i];
    }

    BaseClassName* CreateObject(const std::string& entry_name) const {
        typename ClassMap::const_iterator it =
            m_creator_map.find(entry_name);
        if (it == m_creator_map.end()) {
            return NULL;
        }
        return (*(it->second.creator))();
    }

    BaseClassName* GetSingleton(const std::string& entry_name) const {
        typename ClassMap::const_iterator it =
            m_creator_map.find(entry_name);
        if (it == m_creator_map.end() || it->second.singleton == NULL) {
            return NULL;
        }
        return (*(it->second.singleton))();
    }

private:
    std::vector<std::string> m_creator_names;
    ClassMap m_creator_map;
};

// Get the registry singleton instance for a given register_name
template <typename RegistryTag>
ClassRegistry<typename RegistryTag::BaseClass>& GetRegistry() {
    static ClassRegistry<typename RegistryTag::BaseClass> registry;
    return registry;
}

// CLASS_REGISTER_DEFINE_REGISTRY Make a unique type for a given register_name.
// This class is the base of the generated unique type
template <typename BaseClassName>
struct ClassRegistryTagBase {
    typedef BaseClassName BaseClass;
};

// All class can share the same creator as a function template
template <typename BaseClassName, typename SubClassName>
BaseClassName* ClassRegistry_NewObject() {
    return new SubClassName();
}

// Different RegistryTag generate different instance
template <typename SubClassName, typename RegistryTag>
typename RegistryTag::BaseClass* ClassRegistry_GetSingleton() {
    static SubClassName singleton;
    return &singleton;
}

// Used to register a given class into given registry
template <typename RegistryTag>
class ClassRegisterer {
    typedef typename RegistryTag::BaseClass BaseClassName;
public:
    ClassRegisterer(
        const std::string& entry_name,
        typename ClassRegistry<BaseClassName>::Creator creator,
        typename ClassRegistry<BaseClassName>::Singleton singleton = NULL) {
        GetRegistry<RegistryTag>().AddCreator(entry_name,
                                              creator,
                                              singleton);
    }
    ~ClassRegisterer() {}
};

// Define a registry for a base class.
//
// The first parameter, register_name, should be unique globally.
// Mulitiple registry can be defined for one base class with different
// register_name.
//
// This macro should be used in the same namespace as base_class_name.
#define CLASS_REGISTER_DEFINE_REGISTRY(register_name, base_class_name) \
    struct register_name##RegistryTag: \
        public ::ClassRegistryTagBase<base_class_name> {};

// User could select one of following two versions of
// CLASS_REGISTER_OBJECT_CREATOR, with or without singleton, but couldn't use
// both. So if the user decides to use the singleton version, all
// implementations must have public default constructor.
//
// These macros should be used in the same namespace as class_name, and
// class_name should not be namespace prefixed.
//
// But namespace prefix is required for register_name and base_class_name if
// they are defined in different namespace, for example, ::common::File
//
#define CLASS_REGISTER_OBJECT_CREATOR(register_name, \
                                      base_class_name, \
                                      entry_name_as_string, \
                                      class_name) \
    static ClassRegisterer<register_name##RegistryTag> \
        PP_JOIN(g_object_creator_register_##class_name, __LINE__)( \
            entry_name_as_string, \
            &ClassRegistry_NewObject<base_class_name, class_name>)

#define CLASS_REGISTER_OBJECT_CREATOR_WITH_SINGLETON(register_name, \
                                                     base_class_name, \
                                                     entry_name_as_string, \
                                                     class_name) \
    static ClassRegisterer<register_name##RegistryTag> \
        PP_JOIN(g_object_creator_register_##class_name, __LINE__)( \
            entry_name_as_string, \
            &ClassRegistry_NewObject<base_class_name, class_name>, \
            &ClassRegistry_GetSingleton<class_name, register_name##RegistryTag>)

// Create object from registry by name.
// Namespace prefix is required for register_name if it is defined in different
// namespace
#define CLASS_REGISTER_CREATE_OBJECT(register_name, entry_name_as_string) \
    GetRegistry<register_name##RegistryTag>().CreateObject(entry_name_as_string)

// Get object singleton from registry by name.
#define CLASS_REGISTER_GET_SINGLETON(register_name, entry_name_as_string) \
    GetRegistry<register_name##RegistryTag>().GetSingleton(entry_name_as_string)

// Obtain the number of classes registered to the registry.
#define CLASS_REGISTER_CREATOR_COUNT(register_name) \
    GetRegistry<register_name##RegistryTag>().CreatorCount()

// Obtain class name by index for the registry.
#define CLASS_REGISTER_CREATOR_NAME(register_name, i) \
    GetRegistry<register_name##RegistryTag>().CreatorName(i)

#endif // IPCS_COMMON_BASE_CLASS_REGISTER_H
