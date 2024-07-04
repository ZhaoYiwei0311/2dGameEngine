#ifndef ECS_H
#define ECS_H

#include <bitset>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <set>

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
        int GetId() const;

        Entity& operator = (const Entity& other) = default;
        bool operator == (const Entity& other) const {
            return id == other.GetId();
        }

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
        virtual ~IPool() {}
};

// wrapper of vector containing entities
template <typename T>
class Pool: public IPool {
    private:
        std::vector<T> data;

    public:
        Pool(int size = 100) {
            data.resize(size);
        }

        virtual ~Pool() = default;

        bool isEmpty() const {
            return data.empty();
        }

        int GetSize() const {
            return data.size();
        }

        void Resize(int n) {
            data.resize(n);
        }

        void Clear() {
            data.clear();
        }

        void Add(T object) {
            data.push_back(object);
        }

        void Set(int index, T object) {
            data[index] = object;
        }

        T& Get(int index) {
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
        std::vector<IPool*> componentPools;

        // vector of component signature per entity
        // vector index = entity id
        std::vector<Signature> entityComponentSignatures;

        std::unordered_map<std::type_index, System*> systems;

        // set of enetities that are to be added or deleted
        std::set<Entity> entitiesToBeAdded;
        std::set<Entity> entitiesToBeAdded;

    public:
        Registry() = default;

        Entity CreateEntity();

        void Update();

        void AddEntityToSystem(Entity entity);

};

template <typename TComponent>
void System::RequireComponent() {
    const auto componentId = Component<TComponent>::GetId();
    componentSignature.set(componentId);
}



#endif