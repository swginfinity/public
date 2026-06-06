/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef GALAXYLIST_H_
#define GALAXYLIST_H_

#include "server/db/ServerDatabase.h"
#include "conf/ConfigManager.h"
#include "Galaxy.h"

#ifdef WITH_SWGREALMS_API
#include "server/login/SWGRealmsAPI.h"
#endif

class GalaxyList {
	Vector<Galaxy> galaxies;
	Galaxy current;
	int curIdx = 0;

public:
#ifndef WITH_SWGREALMS_API
	GalaxyList(uint32 accountid) {
		StringBuffer query;
		query << "SELECT g.* FROM `galaxy` g"
			<< " LEFT OUTER JOIN `galaxy_access` ga ON ga.`galaxy_id` = g.`galaxy_id` AND (ga.`account_id` = 0 OR ga.`account_id` = " << accountid << ")"
			<< " WHERE (ga.`account_id` != 0 AND (ga.`expires` IS NULL OR ga.`expires` > NOW()))"
			<< " OR NOT EXISTS (SELECT * FROM `galaxy_access` gb WHERE gb.`galaxy_id` = g.`galaxy_id`)"
			<< " ORDER BY g.`galaxy_id`"
			;

		UniqueReference<ResultSet*> results(ServerDatabase::instance()->executeQuery(query));

		if (results == nullptr)
			return;

		while (results->next()) {
			galaxies.add(Galaxy(results));
		}

		curIdx = 0;
	}
#else // WITH_SWGREALMS_API
	// Implementation in GalaxyList.cpp to avoid circular include issues
	GalaxyList(uint32 accountid) {
		auto swgRealmsAPI = server::login::SWGRealmsAPI::instance();

		if (swgRealmsAPI != nullptr) {
			galaxies = swgRealmsAPI->getAuthorizedGalaxies(accountid);
		}
	}
#endif // WITH_SWGREALMS_API

	bool next() {
		if (curIdx < galaxies.size()) {
			current = galaxies.get(curIdx++);
			return true;
		}

		return false;
	}

	bool isAllowed(uint32 galaxyID) const {
		for (auto& galaxy : galaxies) {
			if (galaxy.getID() == galaxyID)
				return true;
		}

		return false;
	}

	uint32 getID() const {
		return current.getID();
	}

	const String& getName() const {
		return current.getName();
	}

	const String& getAddress() const {
		return current.getAddress();
	}

	uint16 getPort() const {
		return current.getPort();
	}

	uint16 getRandomPort() const {
		return current.getRandomPort();
	}

	uint16 getPingPort() const {
		return current.getPingPort();
	}

	uint32 getPopulation() const {
		return current.getPopulation();
	}

	String toString() const {
		return current.toString();
	}

	inline int size() const {
		return galaxies.size();
	}
};

#endif /*GALAXYLIST_H_*/
