#ifndef SESSION_H
#define SESSION_H

#include <string>
#include <vector>
#include <functional>
#include <exception>
#include <tuple>
#include <memory>
#include <iostream>

#include <nlohmann/json.hpp>

// Some packets may be too long, so they can be split to several different messages
class IncompletePacket
{
public:
	IncompletePacket(std::string data_, size_t expectedLength_) noexcept;

	IncompletePacket operator+(const IncompletePacket &incompletePacket) noexcept;

	bool isComplete() const noexcept;

	const std::string data;
	const size_t expectedLength;
};

/*
 * Data is transmitted as:
 * data_length{json}
 * because several different packets can be delivered in a row
 */

class Packet
{
public:
	Packet(std::string data_) noexcept;
	Packet(const IncompletePacket &packet) noexcept;

	const std::string &getData() const noexcept;
	const std::shared_ptr<const nlohmann::json> getJsonTree() const noexcept;
	bool getIsJsonValid() const noexcept;

private:
	bool convertDataToJson() noexcept;

	std::string data;
	std::shared_ptr<nlohmann::json> json;

	bool isAnswered = false;
	bool isJsonValid = false;
};

#endif // SESSION_H
