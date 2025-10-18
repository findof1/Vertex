#pragma once

#include "types.hpp"
#include <set>
#include <unordered_map>
#include <memory>
#include <cassert>

class System {
public:
    std::set<Entity> mEntities;
    Signature mSignature;
};

class SystemManager {
public:
    template<typename T>
    std::shared_ptr<T> RegisterSystem() {
        const char* typeName = typeid(T).name();

        assert(mSystems.find(typeName) == mSystems.end() && "Registering system more than once.");

        auto system = std::make_shared<T>();
        mSystems.insert({typeName, system});
        return system;
    }

    template<typename T>
    void SetSignature(Signature signature) {
        const char* typeName = typeid(T).name();

        assert(mSystems.find(typeName) != mSystems.end() && "System used before registered.");

        // Set the signature for this system
        mSystems[typeName]->mEntities.clear();
        mSystems[typeName]->mSignature = signature;
    }

    void EntityDestroyed(Entity entity) {
        for (auto const& pair : mSystems) {
            auto const& system = pair.second;

            system->mEntities.erase(entity);
        }
    }

    void EntitySignatureChanged(Entity entity, Signature entitySignature) {
        for (auto const& pair : mSystems) {
            auto const& type = pair.first;
            auto const& system = pair.second;
            auto const& systemSignature = system->mSignature;

            if ((entitySignature & systemSignature) == systemSignature) {
                system->mEntities.insert(entity);
            }
            else {
                system->mEntities.erase(entity);
            }
        }
    }

private:
    std::unordered_map<const char*, Signature> mSignatures{};

    std::unordered_map<const char*, std::shared_ptr<System>> mSystems{};
};