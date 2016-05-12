///
/// [EntityFu](https://github.com/NatWeiss/EntityFu)
/// A simple, fast entity component system written in C++.
/// Under the MIT license.
///

#include "EntityFu.hpp"
#include <algorithm>
using namespace std;

/// Turn this on to have a faster yet riskier ECS.
#define kTrustPointers 0

/// Log macro.
#ifndef Log
#ifndef NDEBUG
#include <stdio.h>
#define Log(...)             \
	{                        \
		printf(__VA_ARGS__); \
		printf("\n");        \
	}
#else
#define Log(...) \
	do           \
	{            \
	} while (0)
#endif
#endif

/// Assert macro.
#ifndef Assert
#ifndef NDEBUG
#define Assert(condition, format, ...) \
	{                                  \
		if (!condition)                \
			throw format;              \
	}
#else
#define Assert(...) \
	do              \
	{               \
	} while (0)
#endif
#endif

/// Turn this to 1 or 2 to debug the ECS.
/// 1 == log creation, 2 == log creation and deletion.
static int verbosity = 0;

/// Static pointers to the ECS data.
static bool *entities = nullptr;
static Entity::Component ***components = nullptr;
static vector<Eid> *componentEids = nullptr;

static void log(Cid cid)
{
	auto n = Entity::count(cid);
	auto &eids = Entity::getAll(cid);
	if (eids.size() > 0)
	{
		Log("Cid %u has %d entities ranging from %u to %u", cid, n, eids.front(), eids.back());
	}
}

void Entity::alloc()
{
	if (components != nullptr)
		return;
	if (verbosity > 0)
	{
		Log("Allocing entities");
	}

	// allocate entities
	entities = new bool[kMaxEntities];
	for (Eid eid = 0; eid < kMaxEntities; ++eid)
		entities[eid] = false;

	// allocate components
	auto max = Component::numCids;
	components = new Component **[max];
	componentEids = new vector<Eid>[Component::numCids];
	for (Cid cid = 0; cid < max; cid++)
	{
		// allocate component array
		components[cid] = new Component *[kMaxEntities];

		// zero component pointers
		for (Eid eid = 0; eid < kMaxEntities; eid++)
			components[cid][eid] = nullptr;
	}
}

void Entity::dealloc()
{
	if (verbosity > 0)
	{
		Log("Deallocing entities");
	}

	if (components != nullptr)
	{
		Entity::destroyAll();
		for (Cid cid = 0; cid < Component::numCids; cid++)
			if (components[cid] != nullptr)
				delete[] components[cid];
		delete[] components;
	}

	if (componentEids != nullptr)
		delete[] componentEids;

	if (entities != nullptr)
		delete[] entities;

	entities = nullptr;
	components = nullptr;
	componentEids = nullptr;
}

Eid Entity::create()
{
	// auto allocate
	Entity::alloc();

	Eid eid = 1;
	for (; eid < kMaxEntities && entities[eid]; ++eid)
	{
	}

	if (eid < 1 || eid >= kMaxEntities)
	{
		Assert(false, "Maximum number of entities reached!");
		eid = 0;
	}
	else
	{
		entities[eid] = true;
		if (verbosity > 0)
		{
			Log("Entity %u created", eid);
		}
	}

	return eid;
}

void Entity::destroyNow(Eid eid)
{
	if (eid == 0)
		return;
	if (verbosity > 0)
	{
		Log("Entity %u being destroyed", eid);
	}

	for (Cid cid = 0; cid < Component::numCids; cid++)
		Entity::removeComponent(cid, eid);
	entities[eid] = false;
}

void Entity::destroyAll()
{
	for (Eid eid = 1; eid < kMaxEntities; ++eid)
		if (entities[eid])
			Entity::destroyNow(eid);
}

void Entity::addComponent(Cid cid, Eid eid, Component *c)
{
	if (c == nullptr)
		return;
	if (eid >= kMaxEntities || !entities[eid] || cid >= Component::numCids)
	{
		Assert(false, "Invalid eid %u or cid %u", eid, cid);
		return;
	}
	if (verbosity > 0)
	{
		Log(" ");
		log(cid);
		Log("Adding component cid %u eid %u (%x)", cid, eid, (int)(long)c);
	}

	// if component already added, delete old one
	if (components[cid][eid] != nullptr)
		Entity::removeComponent(cid, eid);

	// pointers to components are stored in the map
	// (components must be allocated with new, not stack objects)
	components[cid][eid] = c;

	// store component eids
	componentEids[cid].push_back(eid);

	if (verbosity > 0)
		log(cid);
}

void Entity::removeComponent(Cid cid, Eid eid)
{
	if (eid >= kMaxEntities || !entities[eid] || cid >= Component::numCids)
	{
		Assert(false, "Invalid eid %u or cid %u", eid, cid);
		return;
	}

	// get pointer
	auto ptr = components[cid][eid];
	if (ptr == nullptr)
		return;

	if (verbosity > 1)
	{
		Log(" ");
		log(cid);
		Log("Removing component cid %u eid %u (%x)", cid, eid, (int)(long)ptr);
	}

	// pointers to components are deleted
	delete ptr;

	// erase the component pointer
	components[cid][eid] = nullptr;

	// update component eids
	auto &eids = componentEids[cid];
	auto it = find(eids.begin(), eids.end(), eid);
	if (it != eids.end())
		it = eids.erase(it);

	if (verbosity > 1)
		log(cid);
}

Entity::Component *Entity::getComponent(Cid cid, Eid eid)
{
#if (kTrustPointers == 0)
	if (eid < kMaxEntities && cid < Component::numCids)
	{
#endif
		return components[cid][eid];

#if (kTrustPointers == 0)
	}
	return nullptr;
#endif
}

const vector<Eid> &Entity::getAll(Cid cid)
{
	if (cid < Component::numCids)
		return componentEids[cid];
	static vector<Eid> blankEids;
	return blankEids;
}

unsigned Entity::count()
{
	int ret = 0;
	if (entities != nullptr)
	{
		for (Eid eid = 1; eid < kMaxEntities; ++eid)
			if (entities[eid])
				++ret;
	}
	return ret;
}

unsigned Entity::count(Cid cid)
{
	return (unsigned)Entity::getAll(cid).size();
}

bool Entity::exists(Eid eid)
{
	return entities != nullptr && entities[eid];
}

//
// Entity::Component
//

bool Entity::Component::full() const
{
	return !this->empty();
}
