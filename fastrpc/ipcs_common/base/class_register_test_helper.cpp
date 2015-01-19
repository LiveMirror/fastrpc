/// Jack
#include "class_register_test_helper.h"

class HelloMapper : public Mapper {
    virtual std::string GetMapperName() const {
        return "HelloMapper";
    }
};
REGISTER_MAPPER(HelloMapper);

class WorldMapper : public Mapper {
    virtual std::string GetMapperName() const {
        return "WorldMapper";
    }
};
REGISTER_MAPPER(WorldMapper);

class SecondaryMapper : public Mapper {
    virtual std::string GetMapperName() const {
        return "SecondaryMapper";
    }
};
REGISTER_SECONDARY_MAPPER(SecondaryMapper);

class HelloReducer : public Reducer {
    virtual std::string GetReducerName() const {
        return "HelloReducer";
    }
};
REGISTER_REDUCER(HelloReducer);

class WorldReducer : public Reducer {
    virtual std::string GetReducerName() const {
        return "WorldReducer";
    }
};
REGISTER_REDUCER(WorldReducer);

class LocalFileImpl : public FileImpl {
    virtual std::string GetFileImplName() const {
        return "LocalFileImpl";
    }
};
REGISTER_FILE_IMPL("/local", LocalFileImpl);

class MemFileImpl : public FileImpl {
    virtual std::string GetFileImplName() const {
        return "MemFileImpl";
    }
};
REGISTER_FILE_IMPL("/mem", MemFileImpl);

class NetworkFileImpl : public FileImpl {
    virtual std::string GetFileImplName() const {
        return "NetworkFileImpl";
    }
};
REGISTER_FILE_IMPL("/nfs", NetworkFileImpl);
