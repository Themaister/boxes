#include "global.hpp"
#include <memory>
#include <algorithm>

using namespace std;

ContextListener::ContextListener()
{
   ContextManager::get().register_listener(this);
}

ContextListener::~ContextListener()
{
   ContextManager::get().unregister_listener(this);
}

static unique_ptr<ContextManager> manager;

ContextManager& ContextManager::get()
{
   if (manager)
      return *manager;
   else
   {
      manager = unique_ptr<ContextManager>(new ContextManager);
      return *manager;
   }
}

void ContextManager::register_listener(ContextListener *listener)
{
   listeners.push_back(listener);
}

void ContextManager::unregister_listener(const ContextListener *listener)
{
   auto itr = find(begin(listeners), end(listeners), listener);
   if (itr != end(listeners))
      listeners.erase(itr);
}

void ContextManager::notify_reset()
{
   for (auto listener : listeners)
      listener->reset();
}

void ContextManager::notify_destroyed()
{
   for (auto listener : listeners)
      listener->destroyed();
}

