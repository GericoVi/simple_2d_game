#pragma once

#include "Entity.hpp"
#include <map>
#include <string>

// Convenience
using EntityVec = std::vector<std::shared_ptr<Entity>>;

class EntityManager
{
    EntityVec   m_entities;
    EntityVec   m_entitiesToAdd;
    size_t      m_totalEntities = 0;

    // For getting all entities of a certain tag
    std::map<std::string, EntityVec> m_entityMap;

    void removeDeadEntities(EntityVec& vec)
    {
        // TODO: remove all dead entities from the input vector,
        //       called by the update function
    }

public:
    EntityManager() = default;

    void update()
    {
        // TODO: add entities from m_entitiesToAdd to the proper locations
        //       - add them to the vector of all entities
        //       - add them to the vector inside the map of the correct tag

        // remove dead entities from the vector of all entities
        removeDeadEntities(m_entities);

        // also remove from each vector in the entity map
        for (auto& [tag, entityVec] : m_entityMap)
        {
            removeDeadEntities(entityVec);
        }
    }

    // Returns the pointer so we can then add components after we create a new entity
    std::shared_ptr<Entity> addEntity(const std::string& tag)
    {
        // Make entity shared ptr, we can give it the latest id and then increment that
        auto entity = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag));

        m_entitiesToAdd.push_back(entity);

        // If tag is not in the map, make a new one. Then append regardless
        if ( m_entityMap.find(tag) == m_entityMap.end() ) { m_entityMap[tag] = EntityVec(); }
        m_entityMap[tag].push_back(entity);

        return entity;
    }

    const EntityVec& getEntities() { return m_entities; }

    const EntityVec& getEntities(const std::string& tag)
    {
        // If tag doesn't exist in the map, just return empty entity vector
        // But just returning a newly formed entity vector doesn't really work since it's
        // a temporary variable.
        // Let's just add it to the map and return that, it will be empty.
        // Maybe better is to have some error handling??
        if ( m_entityMap.find(tag) == m_entityMap.end() ) { m_entityMap[tag] = EntityVec(); }
        return m_entityMap[tag];
    }

    const std::map<std::string, EntityVec>& getEntityMap() { return m_entityMap; }
};