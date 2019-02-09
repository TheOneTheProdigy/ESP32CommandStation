/**********************************************************************
DCC++ BASE STATION FOR ESP32

COPYRIGHT (c) 2017 Mike Dunston

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses
**********************************************************************/

#include "DCCppESP32.h"

/**********************************************************************

DCC++ESP32 BASE STATION can keep track of the direction of any turnout that is
controlled by a DCC stationary accessory decoder.  All turnouts, as well as any
other DCC accessories connected in this fashion, can always be operated using
the DCC BASE STATION Accessory command:

  <a ADDRESS SUBADDRESS ACTIVATE>

However, this general command simply sends the appropriate DCC instruction
packet to the main tracks to operate connected accessories. It does not store
or retain any information regarding the current status of that accessory.

To have this sketch store and retain the direction of DCC-connected turnouts, as
well as automatically invoke the required <a> command as needed, first
define/edit/delete such turnouts using the following variations of the "T"
command:
  <T ID ADDRESS SUBADDRESS>:   creates a new turnout ID, with specified ADDRESS
                               and SUBADDRESS. If turnout ID already exists, it
                               is updated with specificed ADDRESS and SUBADDRESS
      returns: <O> if successful and <X> if unsuccessful (e.g. out of memory)

  <T ID>:                      deletes definition of turnout ID
      returns: <O> if successful and <X> if unsuccessful (e.g. ID does not exist)

  <T>:                         lists all defined turnouts
      returns: <H ID ADDRESS SUBADDRESS THROW> for each defined turnout or <X>
               if no turnouts defined
where
  ID:         the numeric ID (0-32767) of the turnout to control
  ADDRESS:    the primary address of the decoder controlling this turnout (0-511)
  SUBADDRESS: the subaddress of the decoder controlling this turnout (0-3)

Once all turnouts have been properly defined, use the <E> command to store their
definitions to the ESP32. If you later make edits/additions/deletions to the
turnout definitions, you must invoke the <E> command if you want those new
definitions updated on the ESP32. You can also clear everything stored on the
ESP32 by invoking the <e> command.

To "throw" turnouts that have been defined use:
  <T ID THROW>:    sets turnout ID to either the "thrown" or "unthrown" position
        returns: <H ID THROW>, or <X> if turnout ID does not exist
where
  ID:     the numeric ID (0-32767) of the turnout to control
  THROW:  0 (unthrown) or 1 (thrown)

When controlled as such, the Arduino updates and stores the direction of each
Turnout on the ESP32 so that it is retained even without power. A list of the
current directions of each Turnout in the form <H ID THROW> is generated by this
sketch whenever the <s> status command is invoked. This provides an efficient
way of initializing the directions of any Turnouts being monitored or controlled
by a separate interface or GUI program.
**********************************************************************/

LinkedList<Turnout *> turnouts([](Turnout *turnout) {delete turnout; });

static constexpr const char *ORIENTATION_STRINGS[] = {
  "LEFT",
  "RIGHT",
  "WYE",
  "MULTI"
};

void TurnoutManager::init() {
  log_i("Initializing turnout list");
  JsonObject &root = configStore.load(TURNOUTS_JSON_FILE);
  JsonVariant count = root[JSON_COUNT_NODE];
  uint16_t turnoutCount = count.success() ? count.as<int>() : 0;
  log_i("Found %d turnouts", turnoutCount);
  InfoScreen::replaceLine(INFO_SCREEN_ROTATING_STATUS_LINE, F("Found %02d Turnouts"), turnoutCount);
  if(turnoutCount > 0) {
    for(auto turnout : root.get<JsonArray>(JSON_TURNOUTS_NODE)) {
      turnouts.add(new Turnout(turnout.as<JsonObject &>()));
    }
  }
}

void TurnoutManager::clear() {
  turnouts.free();
  store();
}

uint16_t TurnoutManager::store() {
  JsonObject &root = configStore.createRootNode();
  JsonArray &array = root.createNestedArray(JSON_TURNOUTS_NODE);
  uint16_t turnoutStoredCount = 0;
  for (const auto& turnout : turnouts) {
    turnout->toJson(array.createNestedObject());
    turnoutStoredCount++;
  }
  root[JSON_COUNT_NODE] = turnoutStoredCount;
  configStore.store(TURNOUTS_JSON_FILE, root);
  return turnoutStoredCount;
}

bool TurnoutManager::set(uint16_t turnoutID, bool thrown) {
  bool found = false;
  for (const auto& turnout : turnouts) {
    if(turnout->getID() == turnoutID) {
      turnout->set(thrown);
      found = true;
    }
  }
  if(!found) {
    log_w("Unable to locate turnout with ID %d", turnoutID);
  }
  return found;
}

bool TurnoutManager::toggle(uint16_t turnoutID) {
  bool found = false;
  for (const auto& turnout : turnouts) {
    if(turnout->getID() == turnoutID) {
      turnout->set(!turnout->isThrown());
      found = true;
    }
  }
  if(!found) {
    log_w("Unable to locate turnout with ID %d", turnoutID);
  }
  return found;
}

void TurnoutManager::getState(JsonArray & array) {
  for (const auto& turnout : turnouts) {
    JsonObject &json = array.createNestedObject();
    turnout->toJson(json, true);
  }
}

void TurnoutManager::showStatus() {
  for (const auto& turnout : turnouts) {
    turnout->showStatus();
  }
}

Turnout *TurnoutManager::createOrUpdate(const uint16_t id, const uint16_t address, const int8_t index, const TurnoutOrientation orientation) {
  for (const auto& turnout : turnouts) {
    if(turnout->getID() == id) {
      turnout->update(address, index, orientation);
      return turnout;
    }
  }
  turnouts.add(new Turnout(id, address, index, false, orientation));
  return getTurnoutByID(id);
}

bool TurnoutManager::remove(const uint16_t id) {
  Turnout *turnoutToRemoved = nullptr;
  for (const auto& turnout : turnouts) {
    if(turnout->getID() == id) {
      turnoutToRemoved = turnout;
    }
  }
  if(turnoutToRemoved != nullptr) {
    log_i("Removing Turnout(%d)", turnoutToRemoved->getID());
    turnouts.remove(turnoutToRemoved);
    return true;
  }
  return false;
}

Turnout *TurnoutManager::getTurnoutByIndex(const uint16_t index) {
  Turnout *retval = nullptr;
  uint16_t currentIndex = 0;
  for (const auto& turnout : turnouts) {
    if(currentIndex == index) {
      retval = turnout;
    }
    currentIndex++;
  }
  return retval;
}

Turnout *TurnoutManager::getTurnoutByID(const uint16_t id) {
  Turnout *retval = nullptr;
  for (const auto& turnout : turnouts) {
    if(turnout->getID() == id) {
      retval = turnout;
    }
  }
  return retval;
}

Turnout *TurnoutManager::getTurnoutByAddress(const uint16_t address) {
  Turnout *retval = nullptr;
  for (const auto& turnout : turnouts) {
    if(turnout->getAddress() == address) {
      retval = turnout;
    }
  }
  return retval;
}

uint16_t TurnoutManager::getTurnoutCount() {
  return turnouts.length();
}

void calculateTurnoutBoardAddressAndIndex(uint16_t *boardAddress, uint8_t *boardIndex, uint16_t address) {
  *boardAddress = (address + 3) / 4;
  *boardIndex = (address - (*boardAddress * 4)) + 3;
}

Turnout::Turnout(uint16_t turnoutID, uint16_t address, int8_t index,
  bool thrown, TurnoutOrientation orientation) : _turnoutID(turnoutID),
  _address(address), _index(index), _boardAddress(0), _thrown(thrown),
  _orientation(orientation) {
  if(index == -1) {
    // convert the provided decoder address to a board address and accessory index
    calculateTurnoutBoardAddressAndIndex(&_boardAddress, &_index, _address);
    log_i("Created Turnout(%d): DCC Address: %d, orientation: %d (%s), state: %d (%s)",
      _turnoutID, _address, _orientation, ORIENTATION_STRINGS[_orientation],
      _thrown, _thrown ? JSON_VALUE_THROWN.c_str() : JSON_VALUE_CLOSED.c_str());
  } else {
    log_i("Created Turnout(%d): Address: %d/%d, orientation: %d (%s), state: %d (%s)",
      _turnoutID, _address, _index, _orientation, ORIENTATION_STRINGS[_orientation],
      _thrown, _thrown ? JSON_VALUE_THROWN.c_str() : JSON_VALUE_CLOSED.c_str());
  }
}

Turnout::Turnout(JsonObject &json) {
  _turnoutID = json.get<int>(JSON_ID_NODE);
  _address = json.get<int>(JSON_ADDRESS_NODE);
  _index = json.get<int>(JSON_SUB_ADDRESS_NODE);
  _thrown = json.get<bool>(JSON_STATE_NODE);
  _orientation = (TurnoutOrientation)json.get<int>(JSON_ORIENTATION_NODE);
  _boardAddress = 0;
  if(json.get<int>(JSON_SUB_ADDRESS_NODE) == -1) {
    // convert the provided decoder address to a board address and accessory index
    calculateTurnoutBoardAddressAndIndex(&_boardAddress, &_index, _address);
    log_i("Loaded Turnout(%d): DCC Address: %d, orientation: %d (%s), state: %d (%s)",
      _turnoutID, _address, _orientation, ORIENTATION_STRINGS[_orientation],
      _thrown, _thrown ? JSON_VALUE_THROWN.c_str() : JSON_VALUE_CLOSED.c_str());
  } else {
    log_i("Loaded Turnout(%d): Address: %d/%d, orientation: %d (%s), state: %d (%s)",
      _turnoutID, _address, _index, _orientation, ORIENTATION_STRINGS[_orientation],
      _thrown, _thrown ? JSON_VALUE_THROWN.c_str() : JSON_VALUE_CLOSED.c_str());
  }
}

void Turnout::update(uint16_t address, int8_t index, TurnoutOrientation orientation) {
  _address = address;
  _index = index;
  _orientation = orientation;
  if(index == -1) {
    // convert the provided decoder address to a board address and accessory index
    calculateTurnoutBoardAddressAndIndex(&_boardAddress, &_index, _address);
    log_i("Turnout %d updated to address: %d, orientation: %d (%s)",
      _turnoutID, _address, _orientation, ORIENTATION_STRINGS[_orientation]);
  } else {
    log_i("Turnout %d updated to address: %d/%d, orientation: %d (%s)",
      _turnoutID, _address, _index, _orientation, ORIENTATION_STRINGS[_orientation]);
  }
}

void Turnout::toJson(JsonObject &json, bool readableStrings) {
  json[JSON_ID_NODE] = _turnoutID;
  json[JSON_ADDRESS_NODE] = _address;
  json[JSON_BOARD_ADDRESS_NODE] = _boardAddress;
  if(_boardAddress) {
    json[JSON_SUB_ADDRESS_NODE] = -1;
  } else {
    json[JSON_SUB_ADDRESS_NODE] = _index;
  }
  if(readableStrings) {
    if(_thrown) {
      json[JSON_STATE_NODE] = JSON_VALUE_THROWN;
    } else {
      json[JSON_STATE_NODE] = JSON_VALUE_CLOSED;
    }
  } else {
    json[JSON_STATE_NODE] = _thrown;
  }
  json[JSON_ORIENTATION_NODE] = (int)_orientation;
}

void Turnout::set(bool thrown, bool sendDCCPacket) {
  _thrown = thrown;
  if(sendDCCPacket) {
    std::vector<String> args;
    // if we are in DCC address mode use the calculated board address instead of address
    if(_boardAddress) {
      args.push_back(String(_boardAddress));
    } else {
      args.push_back(String(_address));
    }
    args.push_back(String(_index));
    args.push_back(String(_thrown));
    DCCPPProtocolHandler::getCommandHandler("a")->process(args);
  }
  wifiInterface.printf(F("<H %d %d>"), _turnoutID, _thrown);
  log_i("Turnout(%d) %s", _turnoutID, _thrown ? JSON_VALUE_THROWN.c_str() : JSON_VALUE_CLOSED.c_str());
}

void Turnout::showStatus() {
  wifiInterface.printf(F("<H %d %d %d %d>"), _turnoutID, _address, _index, _thrown);
}

void TurnoutCommandAdapter::process(const std::vector<String> arguments) {
  if(arguments.empty()) {
    // list all turnouts
    TurnoutManager::showStatus();
  } else {
    uint16_t turnoutID = arguments[0].toInt();
    if (arguments.size() == 1 && TurnoutManager::remove(turnoutID)) {
      // delete turnout
      wifiInterface.send(COMMAND_SUCCESSFUL_RESPONSE);
    } else if (arguments.size() == 2 && TurnoutManager::set(turnoutID, arguments[1].toInt() == 1)) {
      // throw turnout
    } else if (arguments.size() == 3) {
      // create/update turnout
      TurnoutManager::createOrUpdate(turnoutID, arguments[1].toInt(), arguments[2].toInt());
      wifiInterface.send(COMMAND_SUCCESSFUL_RESPONSE);
    } else {
      wifiInterface.send(COMMAND_FAILED_RESPONSE);
    }
  }
}

void AccessoryCommand::process(const std::vector<String> arguments) {
  std::vector<uint8_t> packetBuffer;
  uint16_t accessoryAddress = arguments[0].toInt();
  uint8_t accessoryIndex = arguments[1].toInt();
  bool activate = arguments[2].toInt() == 1;
  // first byte is of the form 10AAAAAA, where AAAAAA represent 6 least
  // signifcant bits of accessory address
  packetBuffer.push_back(0x80 + accessoryAddress % 64);
  // second byte is of the form 1AAACDDD, where C should be 1, and the least
  // significant D represent activate/deactivate
  packetBuffer.push_back(((((accessoryAddress / 64) % 8) << 4) +
    (accessoryIndex % 4 << 1) + activate) ^ 0xF8);
  dccSignal[DCC_SIGNAL_OPERATIONS].loadPacket(packetBuffer, 1);
}
