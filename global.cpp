#include "global.hpp"
#include <memory>
#include <algorithm>

using namespace std;
using namespace Template;

ContextListener::ContextListener()
{
   ContextManager::get().register_listener(this);
}

ContextListener::~ContextListener()
{
   ContextManager::get().unregister_listener(this);
}

void ContextListener::register_dependency(ContextListener *listener)
{
   if (!listener)
      return;

   ContextManager::get().register_dependency(this, listener);
}

void ContextListener::unregister_dependency(ContextListener *listener)
{
   if (!listener)
      return;

   ContextManager::get().unregister_dependency(this, listener);
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
   ListenerState state{};
   state.listener = listener;
   state.id = context_id++;
   if (alive)
      state.reset_chain();
   listeners.push_back(std::move(state));
}

void ContextManager::register_dependency(ContextListener *master, ContextListener *slave)
{
   if (!master || !slave)
      return;

   auto& itr_master = find_or_throw(listeners, *master);
   auto& itr_slave  = find_or_throw(listeners, *slave);
   itr_master.dependencies.push_back(&itr_slave);
   itr_slave.dependers.push_back(&itr_master);

   if (alive)
   {
      itr_master.reset_chain();
      itr_slave.reset_chain();
   }
}

void ContextManager::unregister_dependency(ContextListener *master_ptr, ContextListener *slave_ptr)
{
   if (!master_ptr || !slave_ptr)
      return;

   auto& itr_master = find_or_throw(listeners, *master_ptr);
   auto& itr_slave  = find_or_throw(listeners, *slave_ptr);
   erase_all(itr_master.dependencies, &itr_slave);
   erase_all(itr_slave.dependers, &itr_master);
}

void ContextManager::unregister_listener(const ContextListener *listener)
{
   auto& itr = find_or_throw(listeners, *listener);
   for (auto& parent_listener : itr.dependers)
      erase_all(parent_listener->dependencies, &itr);
   for (auto& child_listener : itr.dependencies)
      erase_all(child_listener->dependers, &itr);

   if (itr.signaled)
   {
      itr.signaled = false;
      itr.listener->destroyed();
   }

   erase_all(listeners, itr);
}

void ContextManager::notify_reset()
{
   alive = true;
   for (auto& state : listeners)
      state.reset_chain();
}

void ContextManager::notify_destroyed()
{
   for (auto& state : listeners)
      state.destroy_chain();
   alive = false;
}

void ContextManager::ListenerState::reset_chain()
{
   if (!signaled)
   {
      signaled = true;
      for (auto state : dependencies)
         state->reset_chain();
      listener->reset();
   }
}

void ContextManager::ListenerState::destroy_chain()
{
   if (signaled)
   {
      signaled = false;
      for (auto state : dependers)
         state->destroy_chain();
      listener->destroyed();
   }
}

