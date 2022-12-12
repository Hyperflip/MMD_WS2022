#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdio.h>

typedef uint8_t byte;

namespace SaveData {

	// @note - lukas.vogl - The data our "game" wants to store and read (will be altered, saved and loaded by using controller input)
	struct SaveGame
	{
		uint32_t score = 0u;
	};

	// @note - lukas.vogl - This is a simple representation of a SaveFile that can be saved and loaded
	struct SaveFile
	{
		byte* data = nullptr;
		size_t length = 0u;

		bool IsValid() const { return data != nullptr && length != 0u; }
	};

	class SaveSystem {
	public:
		/*
		* Initialize your save-data API. On consoles we maybe need to do additional
		* things in here as well.
		*/
		bool Initialize() { return true; }

		/*
		* Properly shutdown your save-data API.
		*/
		void Shutdown() {};

		/*
		* Store the provided data into a file on the current platform.
		*
		* - You receive a uint8_t buffer and a length, as well as the name of the save-data file
		* - Write a platform file and store the binary data in it to ready it back later
		* - Make sure to use a corruption-protection mechanism:
		*	- PS4: Use the backup and recovery API provided by the SDK
		*	- PC: Implement your own way of making sure that we only ever override the previous save-data when we could entirely write the new one
		* - Return true when saving worked, and false, if something didn't work (not enough space anymore, other error, ...) -> normally we would return an error reason enum but we stick with a binary output now
		*/
		bool Save(const SaveFile& save, const char* name) {
			const char* tempSaveDataName = "temp.dat";
			const char* backupSaveDataName = "backup.dat";

			std::ofstream output(tempSaveDataName, std::ios::binary);
			output.write((char*)save.data, save.length);
			output.close();

			if (!output.good()) {
				std::cout << "MMD: There was a problem while saving" << std::endl;
				return false;
			}

			std::ifstream temp(tempSaveDataName, std::ios::binary);
			std::ofstream file(name, std::ios::binary);

			file << temp.rdbuf();

			temp.close();
			file.close();

			remove(backupSaveDataName);
			int status = rename(tempSaveDataName, backupSaveDataName);

			if (status == 0) {
				std::cout << "MMD: Successfully created backup" << std::endl;
			}

			return true;
		}

		SaveFile* Load(const char* name) {
		Start:
			SaveGame saveGame;
			SaveFile* saveFile = new SaveFile();

			std::ifstream input(name, std::ios::binary);

			if (!input) {
				std::cout << "MMD: Could not open save file" << std::endl;
				std::cout << "MMD: Attempting to load backup" << std::endl;

				const char* backupSaveDataName = "backup.dat";
				std::ifstream backup(backupSaveDataName, std::ios::binary);

				if (backup.good()) {
					std::cout << "MMD: Restoring backup" << std::endl;
					std::ofstream file(name, std::ios::binary);
					file << backup.rdbuf();

					backup.close();
					file.close();

					goto Start;
				}
				else {
					std::cout << "MMD: No backup exists" << std::endl;
					backup.close();

					return saveFile;
				}
			}

			// TODO: figure out
			// reading data the size of savegame (32 bit) from savefile?
			input.read((char*)&saveGame, sizeof(SaveGame));
			input.close();

			if (!input.good()) {
				std::cout << "MMD: Error occured at reading time" << std::endl;
				return saveFile;
			}

			saveFile->data = reinterpret_cast<byte*>(&saveGame);
			saveFile->length = sizeof(saveGame);

			return saveFile;
		}
	};
}