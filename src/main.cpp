///
/// [EntityFu](https://github.com/NatWeiss/EntityFu)
/// A simple, fast entity component system written in C++.
/// Under the MIT license.
///

#include "EntityFu.hpp"
#include <iostream>
#include <unistd.h>
using namespace std;

/// An example component.
struct HealthComponent : Entity::Component
{
	int hp, maxHP;

	HealthComponent(int _hp, int _maxHP)
	{
		hp = _hp;
		maxHP = _maxHP;
	}

	HealthComponent() : HealthComponent(0, 0) {}

	virtual bool empty() const { return maxHP == 0; }
	virtual bool is_enabled() const { return maxHP == 0; }

	static Cid cid;
};

/// Assign unique class IDs from 0 to `numClassIds` - 1
static Cid _id = 0;
Cid HealthComponent::cid = _id++;
Cid Entity::Component::numCids = _id; // also critical to assign `numClassIds`

/// An example `Ent`.
struct System::Ent
{
	Eid id;
	HealthComponent &health;
	/// Add more components your systems will use frequently

	Ent(Eid _id) : health(Entity::get<HealthComponent>(_id)),
				   id(_id)
	{
	}
};

/// An example system.
struct HealthSystem : System
{
	virtual void tick(double fixedDelta)
	{
		// create a copy of the vector<Eid> for all health components so we
		// don't cause an assertion failure: "vector iterator not incrementable"
		// for more speed you can instead loop over a reference as long as you schedule
		// Entity::destroyNow for after the loop
		auto all = Entity::getAll<HealthComponent>();

		// for this example, just decrement all health components each tick
		for (auto eid : all)
		{
			Ent e(eid);

			// this is overly pragmatic, but you get the drift of how to check if a component is valid
			if (e.health.empty())
				continue;

			// decrement
			e.health.hp--;
			if (e.health.hp < 0)
				e.health.hp = 0;
			cout << "Entity " << (int)e.id << " has " << e.health.hp << "/" << e.health.maxHP << " hit points." << endl;

			// destroy entity if zero health
			if (e.health.hp <= 0)
				Entity::destroyNow(eid);
		}
	}
};

int main(int argc, const char *argv[])
{
	std::vector<System *> m_systems;
	m_systems.push_back(new HealthSystem());

	// create some entities
	Entity::create(new HealthComponent(100, 100));
	Entity::create(new HealthComponent(7, 7));

	// Simulate Game loop
	while (Entity::count())
	{
		// Simulate update() loop
		for (auto &system : m_systems)
		{
			system->tick(0.1);
			usleep(1000 * 100);
		}
	}

	Entity::dealloc();
	cout << "Goodbye, World!\n";
	return 0;
}
