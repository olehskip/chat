#include "packets_manager.h"

PacketsManager::PacketsManager() noexcept
{

}

void PacketsManager::splitPackets(std::string packetsString) noexcept
{
	if(packetsString.empty()) 
		return;

	if(incompletePacket) {
		// if there is a incomplete packet, then the programm should look for the end of the current incomplete packet
		const size_t lengthToBecomeReady = incompletePacket->expectedLength - incompletePacket->data.size();

		if(lengthToBecomeReady > packetsString.size()) {
			// then there should be another message with the end of the packet
			incompletePacket = std::make_unique<IncompletePacket>(*incompletePacket.get() + IncompletePacket(packetsString, 0));
			return;
		}
		else {
			// then the incompletePacket is complete
			incompletePacket = std::make_unique<IncompletePacket>(*incompletePacket.get() + 
				IncompletePacket(packetsString.substr(0, lengthToBecomeReady), 0));
			packetsString.erase(packetsString.begin(), packetsString.begin() + lengthToBecomeReady);
			addReadyPacket(incompletePacket->data);
			incompletePacket.reset();
			splitPackets(packetsString);
			return;
		}

		// if the incompletePacket is complete, it is ready to be processed
		if(incompletePacket->isComplete()) {
			addReadyPacket(incompletePacket->data);
			incompletePacket.reset();
			splitPackets(packetsString);
			return;
		}
	}
	else {
		size_t lengthStartPos = 0, lengthEndPos = 0;
		try {
			std::tie(lengthStartPos, lengthEndPos) = findLengthSignPos(packetsString);
		}
		catch(const std::exception &) {
			// if this message doesn't contain the length, than the program can't read it, because it doesn't know how much to read
			return;
		}
		const size_t packetLength = std::stoi(packetsString.substr(lengthStartPos, lengthEndPos - lengthStartPos + 1));

		if(packetLength > packetsString.size()) {
			// then the program should look for the ending in the next message, so the packet is incomplete
			incompletePacket = std::make_unique<IncompletePacket>(std::string(packetsString.begin() + lengthEndPos + 1, packetsString.end()), 
				packetLength);
			return;
		}
		else {
			// then the packet is complete and the program can process it
			addReadyPacket(packetsString.substr(lengthEndPos + 1, lengthEndPos - lengthStartPos + 1 + packetLength));
			packetsString.erase(packetsString.begin() + lengthStartPos, packetsString.begin() + lengthEndPos - lengthStartPos + 1 + packetLength);
			splitPackets(packetsString);
			return;
		}
	}
}

std::vector<std::shared_ptr<const nlohmann::json>> PacketsManager::getPacketsJson() noexcept
{
	std::vector<std::shared_ptr<const nlohmann::json>> output;
	const std::size_t readyPacketsSize = readyPackets.size();
	output.reserve(readyPacketsSize);
	for(std::size_t i = 0; i < readyPacketsSize; ++i) {
		output.push_back(readyPackets[i]->getJsonTree());
	}
	return output;
}

void PacketsManager::clearReadyPackets() noexcept
{
	readyPackets.clear();
}

std::pair<size_t, size_t> PacketsManager::findLengthSignPos(std::string str)
{
	size_t lengthStartPos = 0, lengthEndPos = 0;
	bool lengthStartPosPresence = false;

	const std::size_t strSize = str.size();
	for(size_t i = 0; i < strSize; ++i) {
		if(isdigit(str[i])) {
			if(!lengthStartPosPresence) {
				lengthStartPos = i;
				lengthStartPosPresence = true;
			}
			// the length can be in the end
			else if(i == strSize - 1) {
				lengthEndPos = i;
				break;
			}
		}
		else if(lengthStartPosPresence) {
			lengthEndPos = i - 1;
			break;
		}
	}
	if(!lengthStartPosPresence)
		throw std::invalid_argument("There is no packet length");

	return {lengthStartPos, lengthEndPos};
};

void PacketsManager::addReadyPacket(std::string str) noexcept
{
	std::unique_ptr<Packet> packet = std::make_unique<Packet>(str);
	if(packet->getIsJsonValid())
		readyPackets.push_back(std::move(packet));	
}
