/*
			Copyright <SWGEmu>
	See file COPYING for copying conditions.*/

#ifndef GALAXY_H_
#define GALAXY_H_

#include "engine/engine.h"

// #define USE_RANDOM_EXTRA_PORTS
#ifdef USE_RANDOM_EXTRA_PORTS
#include "conf/ConfigManager.h"
#endif // USE_RANDOM_EXTRA_PORTS

class Galaxy {
	uint32 id = 0;
	String name;
	String address;
	uint32 port = 0;
	uint32 pingPort = 0;
	uint32 population = 0;
#ifdef USE_RANDOM_EXTRA_PORTS
	Vector<uint32> extraPorts;
#endif // USE_RANDOM_EXTRA_PORTS
public:
	Galaxy() = default;

	Galaxy(ResultSet *result) {
		id = result->getUnsignedInt(0);
		name = result->getString(1);
		address = result->getString(2);
		port = result->getUnsignedInt(3);
		pingPort = result->getUnsignedInt(4);
		population = result->getUnsignedInt(5);
#ifdef USE_RANDOM_EXTRA_PORTS
		extraPorts.add(port);

		try {
			String extraPortStrings = result->getString(6);

			if (!extraPortStrings.isEmpty()) {
				StringTokenizer tokenizer(extraPortStrings);
				tokenizer.setDelimiter(",");

				while (tokenizer.hasMoreTokens() && extraPorts.size() < 256) {
					try {
						uint32 newPort = tokenizer.getIntToken();

						if (newPort != 0)
							extraPorts.add(newPort);
					} catch (Exception e) {
						// Do nothing
					}
				}
			}
		} catch (Exception e) {
			// Do Nothing
		}
#endif // USE_RANDOM_EXTRA_PORTS
	}

	Galaxy(uint32 id) {
		this->id = id;
	}

	void setID(uint32 id) {
		this->id = id;
	}

	void setName(const String& name) {
		this->name = name;
	}

	void setAddress(const String& address) {
		this->address = address;
	}

	void setPort(uint32 port) {
		this->port = port;
#ifdef USE_RANDOM_EXTRA_PORTS
		extraPorts.add(port);
#endif // USE_RANDOM_EXTRA_PORTS
	}

	void setPingPort(uint32 pingPort) {
		this->pingPort = pingPort;
	}

	void setPopulation(uint32 population) {
		this->population = population;
	}

	uint32 getID() const {
		return id;
	}

	const String& getName() const {
		return name;
	}

	const String& getAddress() const {
		return address;
	}

	uint32 getPort() const {
		return port;
	}

	uint32 getPingPort() const {
		return pingPort;
	}

	uint32 getPopulation() const {
		return population;
	}

#ifdef USE_RANDOM_EXTRA_PORTS
	void addPort(uint32 port) {
		extraPorts.add(port);
	}
#endif // USE_RANDOM_EXTRA_PORTS

	uint16 getRandomPort() const {
#ifdef USE_RANDOM_EXTRA_PORTS
		const static auto type = ConfigManager::instance()->getInt("Core3.ZonePortsBalancer", 1);

		if (type == 1) {
			static AtomicInteger roundRobin;

			return (uint16)extraPorts.get(roundRobin.increment() % extraPorts.size());
		} else {
			return (uint16)extraPorts.get(System::random(extraPorts.size() - 1));
		}
#else // USE_RANDOM_EXTRA_PORTS
		return port;
#endif // USE_RANDOM_EXTRA_PORTS
	}

	bool toBinaryStream(ObjectOutputStream* stream) {
		return false;
	}

	bool parseFromBinaryStream(ObjectInputStream* stream) {
		return false;
	}

	String toString() const {
		StringBuffer buf;

		buf << "Galaxy("
			<< "id: " << id
			<< ", name: " << name
			<< ", address: " << address
			<< ", port: " << port
			<< ", pingPort: " << pingPort
			<< ", population: " << population
		;
#ifdef USE_RANDOM_EXTRA_PORTS

		buf << ", extraPorts:";

		for (auto port : extraPorts)
			buf << " " << port;
#endif
		buf << ")";

		return buf.toString();
	}

	String toStringData() const {
		return toString();
	}
};

#endif // GALAXY_H_
