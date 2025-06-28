#pragma once

#include "Components.hpp"
#include <string>
#include <tuple>

// Declare here so we can set as friend class in `Entity` while avoiding circular includes.
// Will actually be defined in `EntityManager.hpp`, but that includes this file.
class EntityManager;

using ComponentTuple = std::tuple<CTransform, CShape, CCollision, CInput, CScore, CLifespan, CSpecial>;

class Entity
{
    // Let's EntityManager access private attributes and methods of this class
    // So that only EntityManager can construct these
    friend class EntityManager;

    ComponentTuple      m_components;
    bool                m_active = true;
    std::string         m_tag = "default";
    size_t              m_id = 0;

    Entity(const size_t& id, const std::string& tag)
        : m_tag(tag), m_id(id) { }

public:
    bool isActive() const { return m_active; }
    
    void destroy() { m_active = false; }
    
    size_t id() const { return m_id; }
    
    const std::string& tag() const { return m_tag; }
    
    template <typename T>
    bool has() const { return get<T>().exists; }

    // Add component by reinitialising it with arguments, and setting `exists` attribute
    template <typename T, typename... TArgs>
    T& add(TArgs&&... mArgs)
    {
        auto& component = get<T>();
        component = T(std::forward<TArgs>(mArgs)...);
        component.exists = true;
        return component;
    }

    // Get correct component, they're all different types so we can do a generic function
    // here to get it from the tuple.
    template <typename T>
    T& get() { return std::get<T>(m_components); }

    template <typename T>
    const T& get() const { return std::get<T>(m_components); }

    // We can remove a component by just reinitialising it with the default constructor
    template <typename T>
    void remove() { get<T>() = T(); }
};