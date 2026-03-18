#pragma once
#include "IObserver.h"
#include <vector>
#include <algorithm>

namespace dae
{
    class Subject
    {
    public:
        void AddObserver(IObserver* observer)
        {
            m_observers.push_back(observer);
        }

        void RemoveObserver(IObserver* observer)
        {
            m_observers.erase(
                std::remove(m_observers.begin(), m_observers.end(), observer),
                m_observers.end()
            );
        }

        void NotifyObservers(GameEvent event, int value = 0)
        {
            for (auto* observer : m_observers)
                observer->OnNotify(event, value);
        }

    private:
        std::vector<IObserver*> m_observers;
    };
}