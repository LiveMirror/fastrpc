/// Jack
#include <set>
#include "class_register.h"
#include "class_register_test_helper.h"
#include "scoped_ptr.h"
#include "gtest.h"


namespace base_namespace {

class SomeClass {
public:
    virtual ~SomeClass() {}
};

CLASS_REGISTER_DEFINE_REGISTRY(some_class_register, SomeClass);

#define REGISTER_SOME_CLASS(class_name) \
    CLASS_REGISTER_OBJECT_CREATOR( \
        base_namespace::some_class_register, \
        base_namespace::SomeClass, \
        #class_name, \
        class_name)

#define CREATE_SOME_CLASS(name_as_string) \
    CLASS_REGISTER_CREATE_OBJECT(base_namespace::some_class_register, name_as_string)

} // namespace base_namespace

namespace other_namespace {

class OtherClass : public ::base_namespace::SomeClass { };

REGISTER_SOME_CLASS(OtherClass);

} // namespace other_namespace

TEST(ClassRegister, CrossNamespace) {
    ipcs_common::scoped_ptr<base_namespace::SomeClass> p(CREATE_SOME_CLASS("OtherClass"));
    EXPECT_TRUE(p.get() != NULL);
}

TEST(ClassRegister, CreateMapper) {
    ipcs_common::scoped_ptr<Mapper> mapper;
    mapper.reset(CREATE_MAPPER(""));
    EXPECT_TRUE(mapper.get() == NULL);

    mapper.reset(CREATE_MAPPER("HelloMapper "));
    EXPECT_TRUE(mapper.get() == NULL);

    mapper.reset(CREATE_MAPPER("HelloWorldMapper"));
    EXPECT_TRUE(mapper.get() == NULL);

    mapper.reset(CREATE_MAPPER("HelloReducer"));
    EXPECT_TRUE(mapper.get() == NULL);

    mapper.reset(CREATE_MAPPER("WorldReducer"));
    EXPECT_TRUE(mapper.get() == NULL);

    mapper.reset(CREATE_MAPPER("SecondaryMapper"));
    EXPECT_TRUE(mapper.get() == NULL);

    mapper.reset(CREATE_MAPPER("HelloMapper"));
    ASSERT_TRUE(mapper.get() != NULL);
    EXPECT_EQ("HelloMapper", mapper->GetMapperName());

    mapper.reset(CREATE_MAPPER("WorldMapper"));
    ASSERT_TRUE(mapper.get() != NULL);
    EXPECT_EQ("WorldMapper", mapper->GetMapperName());
}

TEST(ClassRegister, CreateSecondaryMapper) {
    ipcs_common::scoped_ptr<Mapper> mapper;
    mapper.reset(CREATE_SECONDARY_MAPPER(""));
    EXPECT_TRUE(mapper.get() == NULL);

    mapper.reset(CREATE_SECONDARY_MAPPER("SecondaryMapper "));
    EXPECT_TRUE(mapper.get() == NULL);

    mapper.reset(CREATE_SECONDARY_MAPPER("HelloWorldMapper"));
    EXPECT_TRUE(mapper.get() == NULL);

    mapper.reset(CREATE_SECONDARY_MAPPER("HelloReducer"));
    EXPECT_TRUE(mapper.get() == NULL);

    mapper.reset(CREATE_SECONDARY_MAPPER("WorldReducer"));
    EXPECT_TRUE(mapper.get() == NULL);

    mapper.reset(CREATE_SECONDARY_MAPPER("HelloMapper"));
    EXPECT_TRUE(mapper.get() == NULL);

    mapper.reset(CREATE_SECONDARY_MAPPER("WorldMapper"));
    EXPECT_TRUE(mapper.get() == NULL);

    mapper.reset(CREATE_SECONDARY_MAPPER("SecondaryMapper"));
    ASSERT_TRUE(mapper.get() != NULL);
    EXPECT_EQ("SecondaryMapper", mapper->GetMapperName());
}

TEST(ClassRegister, CreateReducer) {
    ipcs_common::scoped_ptr<Reducer> reducer;
    reducer.reset(CREATE_REDUCER(""));
    EXPECT_TRUE(reducer.get() == NULL);

    reducer.reset(CREATE_REDUCER("HelloReducer "));
    EXPECT_TRUE(reducer.get() == NULL);

    reducer.reset(CREATE_REDUCER("HelloWorldReducer"));
    EXPECT_TRUE(reducer.get() == NULL);

    reducer.reset(CREATE_REDUCER("HelloMapper"));
    EXPECT_TRUE(reducer.get() == NULL);

    reducer.reset(CREATE_REDUCER("WorldMapper"));
    EXPECT_TRUE(reducer.get() == NULL);

    reducer.reset(CREATE_REDUCER("HelloReducer"));
    ASSERT_TRUE(reducer.get() != NULL);
    EXPECT_EQ("HelloReducer", reducer->GetReducerName());

    reducer.reset(CREATE_REDUCER("WorldReducer"));
    ASSERT_TRUE(reducer.get() != NULL);
    EXPECT_EQ("WorldReducer", reducer->GetReducerName());
}

TEST(ClassRegister, CreateFileImpl) {
    ipcs_common::scoped_ptr<FileImpl> file_impl;
    file_impl.reset(CREATE_FILE_IMPL("/mem"));
    ASSERT_TRUE(file_impl.get() != NULL);
    EXPECT_EQ("MemFileImpl", file_impl->GetFileImplName());

    file_impl.reset(CREATE_FILE_IMPL("/nfs"));
    ASSERT_TRUE(file_impl.get() != NULL);
    EXPECT_EQ("NetworkFileImpl", file_impl->GetFileImplName());

    file_impl.reset(CREATE_FILE_IMPL("/local"));
    ASSERT_TRUE(file_impl.get() != NULL);
    EXPECT_EQ("LocalFileImpl", file_impl->GetFileImplName());

    file_impl.reset(CREATE_FILE_IMPL("/"));
    EXPECT_TRUE(file_impl.get() == NULL);

    file_impl.reset(CREATE_FILE_IMPL(""));
    EXPECT_TRUE(file_impl.get() == NULL);

    file_impl.reset(CREATE_FILE_IMPL("/mem2"));
    EXPECT_TRUE(file_impl.get() == NULL);

    file_impl.reset(CREATE_FILE_IMPL("/mem/"));
    EXPECT_TRUE(file_impl.get() == NULL);

    file_impl.reset(CREATE_FILE_IMPL("/nfs2/"));
    EXPECT_TRUE(file_impl.get() == NULL);
}

TEST(ClassRegister, FileImplNames) {
    EXPECT_EQ(3u, FILE_IMPL_COUNT());

    std::set<std::string> file_impl_names;
    for (size_t i = 0; i < FILE_IMPL_COUNT(); ++i) {
        file_impl_names.insert(FILE_IMPL_NAME(i));
    }
    EXPECT_EQ(3u, file_impl_names.size());
    EXPECT_TRUE(file_impl_names.find("/mem") != file_impl_names.end());
    EXPECT_TRUE(file_impl_names.find("/nfs") != file_impl_names.end());
    EXPECT_TRUE(file_impl_names.find("/local") != file_impl_names.end());
}

TEST(ClassRegister, FileImplSingleton) {
    ipcs_common::scoped_ptr<FileImpl> file_impl;
    ASSERT_TRUE(GET_FILE_IMPL_SINGLETON("/mem") != NULL);
    EXPECT_EQ("MemFileImpl",
              GET_FILE_IMPL_SINGLETON("/mem")->GetFileImplName());
    // Test if it's a "real" singleton.
    EXPECT_EQ(GET_FILE_IMPL_SINGLETON("/mem"),
              GET_FILE_IMPL_SINGLETON("/mem"));
    file_impl.reset(CREATE_FILE_IMPL("/mem"));
    EXPECT_NE(GET_FILE_IMPL_SINGLETON("/mem"), file_impl.get());

    ASSERT_TRUE(GET_FILE_IMPL_SINGLETON("/nfs") != NULL);
    EXPECT_EQ("NetworkFileImpl",
              GET_FILE_IMPL_SINGLETON("/nfs")->GetFileImplName());
    EXPECT_EQ(GET_FILE_IMPL_SINGLETON("/nfs"),
              GET_FILE_IMPL_SINGLETON("/nfs"));
    file_impl.reset(CREATE_FILE_IMPL("/nfs"));
    EXPECT_NE(GET_FILE_IMPL_SINGLETON("/nfs"), file_impl.get());

    ASSERT_TRUE(GET_FILE_IMPL_SINGLETON("/local") != NULL);
    EXPECT_EQ("LocalFileImpl",
              GET_FILE_IMPL_SINGLETON("/local")->GetFileImplName());
    EXPECT_EQ(GET_FILE_IMPL_SINGLETON("/local"),
              GET_FILE_IMPL_SINGLETON("/local"));
    file_impl.reset(CREATE_FILE_IMPL("/local"));
    EXPECT_NE(GET_FILE_IMPL_SINGLETON("/local"), file_impl.get());

    EXPECT_TRUE(GET_FILE_IMPL_SINGLETON("/") == NULL);
    EXPECT_TRUE(GET_FILE_IMPL_SINGLETON("") == NULL);
    EXPECT_TRUE(GET_FILE_IMPL_SINGLETON("/mem/") == NULL);
    EXPECT_TRUE(GET_FILE_IMPL_SINGLETON("/mem2") == NULL);
}

// } // namespace common
