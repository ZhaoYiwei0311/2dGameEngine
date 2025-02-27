#ifndef ECS_H
#define ECS_H

#include "../Logger/Logger.h"

#include <bitset>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <set>
#include <memory>
#include <deque>

const unsigned int MAX_COMPONENTS = 32;


// Signature
// a bitset to keep track of which components an entity has,
// and also helps keep the track of which entities a system is interested in
typedef std::bitset<MAX_COMPONENTS> Signature;

struct IComponent {
    protected:
        static int nextId;
};

// used to assign a unique id to a component type
template <typename T>
class Component: public IComponent {
    public:
        static int GetId() {
            static auto id = nextId++;
            return id;
    }

};

class Entity {
    private:
        int id;

    public:
        Entity(int id): id(id) {};
        Entity(const Entity& entity) = default;
        void Kill();
        int GetId() const;

        // manage entity tags and groups
        void Tag(const std::string &tag);
        bool HasTag(const std::string& tag) const;
        void Group(const std::string& group);
        bool BelongsToGroup(const std::string& group) const;

        Entity& operator =(const Entity& other) = default;
        bool operator ==(const Entity& other) const { return id == other.id; }
        bool operator !=(const Entity& other) const { return id != other.id; }
        bool operator >(const Entity& other) const { return id > other.id; }
        bool operator <(const Entity& other) const { return id < other.id; }

        template <typename TComponent, typename ...TArgs> void AddComponent(TArgs&& ...args);
        template <typename TComponent> void RemoveComponent();
        template <typename TComponent> bool HasComponent() const;
        template <typename TComponent> TComponent& GetComponent() const;

        // hold a pointer to the entity's owner registry
        class Registry* registry;


};

// the system processes entities with signature
class System {
    private:
        Signature componentSignature;
        std::vector<Entity> entities;
    
    public:
        System() = default;
        ~System() = default;

        void AddEntityToSystem(Entity entity);
        void RemoveEntityFromSystem(Entity entity);
        std::vector<Entity> GetSystemEntities() const;
        const Signature& GetComponentSignature() const;

        // defines the component type that entities must have to be considered by the system
        template <typename TComponent> void RequireComponent();

};

class IPool {
    public:
        virtual ~IPool() = default;
        virtual void RemoveEntityFromPool(int entityId) = 0;
};

// wrapper of vector containing entities
template <typename T>
class Pool: public IPool {
    private:
    // keep track of vector if objects and current number of elements
        std::vector<T> data;
        int size;

        // helper maps
        std::unordered_map<int, int> entityIdToIndex;
        std::unordered_map<int, int> indexToEntityId;

    public:
        Pool(int capacity = 100) {
            size = 0;
            data.resize(capacity);
        }

        virtual ~Pool() = default;

        bool IsEmpty() const {
            return size == 0;
        }

        int GetSize() const {
            return size;
        }

        void Resize(int n) {
            data.resize(n);
        }

        void Clear() {
            data.clear();
            size = 0;
        }

        void Add(T object) {
            data.push_back(object);
        }

        void Set(int entityId, T object) {
            if (entityIdToIndex.find(entityId) != entityIdToIndex.end()) {
                // if the element already exists, simply replace the component object
                int index = entityIdToIndex[entityId];
                data[index] = object;
            } else {
                // when adding a new object, keep track of entity ids and vector index 
                int index = size;
                entityIdToIndex.emplace(entityId, index);
                indexToEntityId.emplace(index, entityId);
                if (index >= data.capacity()) {
                    // if necessary, resize current capacity
                    data.resize(size * 2);
                }
                data[index] = object;
                size++;
            }
        }

        void Remove(int entityId) {
            // copy the last element to the deleted position to array packed
            int indexOfRemoved = entityIdToIndex[entityId];
            int indexOfLast = size - 1;
            data[indexOfRemoved] = data[indexOfLast];

            // update the index-entity map to point to the correct elements
            int entityIdOfLastElement = indexToEntityId[indexOfLast];
            entityIdToIndex[entityIdOfLastElement] = indexOfRemoved;
            indexToEntityId[indexOfRemoved] = entityIdOfLastElement;

            entityIdToIndex.erase(entityId);
            indexToEntityId.erase(indexOfLast);

            size--;
        }

        void RemoveEntityFromPool(int entityId) override {
            if (entityIdToIndex.find(entityId) != entityIdToIndex.end()) {
                Remove(entityId);
            }
        }

        T& Get(int entityId) {
            int index = entityIdToIndex[entityId];
            return static_cast<T&>(data[index]);
        }

        T& operator [](unsigned int index) {
            return data[index];
        }
};

// the registry manages the creationg and destruction of entites, add systems and components
class Registry {
    private:
        int numEntities = 0;

        // vector of component pools, each pool contains all the data for a certain component type
        // vector index = component type id
        // pool index = entity id
        std::vector<std::shared_ptr<IPool>> componentPools;

        // vector of component signature per entity
        // vector index = entity id
        std::vector<Signature> entityComponentSignatures;

        std::unordered_map<std::type_index, std::shared_ptr<System>> systems;

        // set of enetities that are to be added or deleted
        std::set<Entity> entitiesToBeAdded;
        std::set<Entity> entitiesToBeKilled;

        // entity tags (one tag name per entity)
        std::unordered_map<std::string, Entity> entitiyPerTag;
        std::unordered_map<int, std::string> tagPerEntity;

        // entity groups (a set of entities per group name)
        std::unordered_map<std::string, std::set<Entity>> entitiesPerGroup;
        std::unordered_map<int, std::string> groupPerEntity;

        // list of free entity ids that were previously removed
        std::deque<int> freeIds;

    public:
        Registry() {
            Logger::Log("Registry constructor called");
        };

        ~Registry() {
            Logger::Log("Registry destructor called");
        }

        // process entities that are waiting to be added or killed
        void Update();

        // entity management
        Entity CreateEntity();
        void KillEntity(Entity entity);

        // tag management
        void TagEntity(Entity entity, const std::string& tag);
        bool EntityHasTag(Entity entity, const std::string& tag) const;
        Entity GetEntityByTag(const std::string& tag) const;
        void RemoveEntityTag(Entity entity);

        // group management
        void GroupEntity(Entity entity, const std::string& group);
        bool EntityBelongsToGroup(Entity entity, const std::string& group) const;
        std::vector<Entity> GetEntitesByGroup(const std::string& group) const;
        void RemoveEntityGroup(Entity entity);

        // component management
        template <typename TComponent, typename ...TArgs> void AddComponent(Entity entity, TArgs&& ...args);
        template <typename TComponent> void RemoveComponent(Entity entity);
        template <typename TComponent> bool HasComponent(Entity entity) const;
        template <typename TComponent> TComponent& GetComponent(Entity entity) const;

        // system management
        template <typename TSystem, typename ...TArgs> void AddSystem(TArgs&& ...args);
        template <typename TSystem> void RemoveSystem();
        template <typename TSystem> bool HasSystem() const;
        template <typename TSystem> TSystem& GetSystem() const;

        // check the component signature of an entity and add the entity to systems that are interested in it
        void AddEntityToSystems(Entity entity);
        void RemoveEntityFromSystems(Entity entity);

};

template <typename TComponent>
void System::RequireComponent() {
    const auto componentId = Component<TComponent>::GetId();
    componentSignature.set(componentId);
}

template <typename TSystem, typename ...TArgs>
void Registry::AddSystem(TArgs&& ...args) {
    std::shared_ptr<TSystem> newSystem = std::make_shared<TSystem>(std::forward<TArgs>(args)...);
    systems.insert(std::make_pair(std::type_index(typeid(TSystem)), newSystem));
}

template <typename TSystem>
void Registry::RemoveSystem() {
    auto system = systems.find(std::type_index(typeid(TSystem)));
    systems.erase(system);
}

template <typename TSystem>
bool Registry::HasSystem() const {
    return systems.find(std::type_index(typeid(TSystem))) != systems.end();
}

template <typename TSystem>
TSystem& Registry::GetSystem() const {
    auto system = systems.find(std::type_index(typeid(TSystem)));
    return *(std::static_pointer_cast<TSystem>(system->second));
}

template <typename TComponent, typename ...TArgs>
void Registry::AddComponent(Entity entity, TArgs&& ...args) {
    const auto componentId = Component<TComponent>::GetId();
    const auto entityId = entity.GetId();

    if (componentId >= static_cast<int>(componentPools.size())) {
        componentPools.resize(componentId + 1, nullptr);
    }

    if (!componentPools[componentId]) {
        std::shared_ptr<Pool<TComponent>> newComponentPool = std::make_shared<Pool<TComponent>>();
        componentPools[componentId] = newComponentPool;
    }

    std::shared_ptr<Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);

    TComponent newComponent(std::forward<TArgs>(args)...);

    componentPool->Set(entityId, newComponent);

    entityComponentSignatures[entityId].set(componentId);

    Logger::Log("Component id = " + std::to_string(componentId) + " was added to entity id " + std::to_string(entityId));
}

template <typename TComponent>
void Registry::RemoveComponent(Entity entity) {
    const auto componentId = Component<TComponent>::GetId();
    const auto entityId = entity.GetId();

    // remove the component from the component list for that entity
    std::shared_ptr<Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);
    componentPool->Remove(entityId);

    entityComponentSignatures[entityId].set(componentId, false);

    Logger::Log("Component id = " + std::to_string(componentId) + " was removed from entity id " + std::to_string(entityId));
}

template <typename TComponent>
bool Registry::HasComponent(Entity entity) const {
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();
	return entityComponentSignatures[entityId].test(componentId);
}

template <typename TComponent>
TComponent& Registry::GetComponent(Entity entity) const {
    const auto componentId = Component<TComponent>::GetId();
    const auto entityId = entity.GetId();
    auto componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);
    return componentPool->Get(entityId);
}

template <typename TComponent, typename ...TArgs>
void Entity::AddComponent(TArgs&& ...args) {
    registry->AddComponent<TComponent>(*this, std::forward<TArgs>(args)...);

}

template <typename TComponent>
void Entity::RemoveComponent() {
    registry->RemoveComponent<TComponent>(*this);
}

template <typename TComponent>
bool Entity::HasComponent() const {
    return registry->HasComponent<TComponent>(*this);
}

template <typename TComponent>
TComponent& Entity::GetComponent() const {
    return registry->GetComponent<TComponent>(*this);
}



#endif