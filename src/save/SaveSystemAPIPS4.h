#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>

#include <save_data.h>
#include <sceerror.h>
#include <user_service.h>

typedef uint8_t byte;

#define PRINT					printf
#define EPRINT					printf("Error : %s at %d\n  ", __FILE__, __LINE__ ); \
								printf

namespace SaveData
{
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

	class SaveSystem
	{
	public:
		SaveSystem()
			:m_UserId(SCE_USER_SERVICE_USER_ID_INVALID)
		{
			int ret = SCE_OK;

			ret = sceUserServiceInitialize(NULL);
			if (ret < SCE_OK) {
				if (ret == SCE_USER_SERVICE_ERROR_ALREADY_INITIALIZED) {
					std::cout << "User service already initialized" << std::endl;
				}
				else {
					std::cout << "Failed to initialize user service" << std::endl;
					return;
				}
			}
			else {
				std::cout << "Successfully initialize user service" << std::endl;
			}

			ret = sceSaveDataInitialize3(NULL);
			if (ret < SCE_OK) {
				std::cout << "Failed to initialize save data system" << std::endl;
				return;
			}
			std::cout << "Initialize save data system" << std::endl;

			ret = sceUserServiceGetInitialUser(&m_UserId);
			if (ret < SCE_OK) {
				std::cout << "Failed to obtain user id" << std::endl;
				return;
			}
			std::cout << "Obtained user id" << std::endl;

			memset(&m_DirName, 0x00, sizeof(m_DirName));
			strlcpy(m_DirName.data, "SAVEDATA00", sizeof(m_DirName.data));
		}
		/*
		* Initialize your save-data API. On consoles we maybe need to do additional
		* things in here as well.
		*/
		bool Initialize() {
			int ret = SCE_OK;

			SceSaveDataMount2 mount2;
			setupSceSaveDataMount2(m_UserId,
				SCE_SAVE_DATA_MOUNT_MODE_CREATE | SCE_SAVE_DATA_MOUNT_MODE_RDWR,
				&m_DirName,
				&mount2);
			SceSaveDataMountResult mountResult;
			memset(&mountResult, 0x00, sizeof(mountResult));
			ret = sceSaveDataMount2(&mount2, &mountResult);
			if (ret < SCE_OK) {
				if (ret == SCE_SAVE_DATA_ERROR_BUSY || ret == SCE_SAVE_DATA_ERROR_EXISTS) {
					std::cout << "Save data already exists" << std::endl;
					return true;
				}
				else {
					std::cout << "Failed to mount save data" << std::endl;
					return false;
				}
			}
			std::cout << "Successfully mounted save data" << std::endl;

			SceSaveDataMountPoint* mountPoint = &mountResult.mountPoint;

			ret = sceSaveDataUmount(mountPoint);
			if (ret < SCE_OK) {
				std::cout << "Failed to unmount save data" << std::endl;
				return false;
			}
			std::cout << "Successfully unmounted save data" << std::endl;

			return true;
		}

		/*
		* Properly shutdown your save-data API.
		*/
		void Shutdown() {
			int ret = SCE_OK;

			ret = sceSaveDataTerminate();
			if (ret < SCE_OK) {
				std::cout << "Failed to terminate save data" << std::endl;
			}
		};

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
			int32_t ret = SCE_OK;
			bool hasSaved = true;

			// Mount
			SceSaveDataMount2 mount2;
			setupSceSaveDataMount2(m_UserId,
				SCE_SAVE_DATA_MOUNT_MODE_RDWR,
				&m_DirName,
				&mount2);
			SceSaveDataMountResult mountResult;
			memset(&mountResult, 0x00, sizeof(mountResult));
			ret = sceSaveDataMount2(&mount2, &mountResult);
			if (ret < SCE_OK)
			{
				std::cout << "Failed to mount save data" << std::endl;
				hasSaved = false;
				return hasSaved;
			}

			SceSaveDataMountPoint* mountPoint = &mountResult.mountPoint;

			// Write
			char path[64];
			snprintf(path, sizeof(path), "%s/%s", mountPoint->data, name);

			std::ofstream output(path, std::ios::binary);
			output.write((char*)save.data, save.length);
			output.close();

			if (!output.good()) {
				std::cout << "There was a problem while saving" << std::endl;
				hasSaved = false;
			}

			// Unmount + Backup
			ret = sceSaveDataUmountWithBackup(mountPoint);
			if (ret < SCE_OK)
			{
				EPRINT("sceSaveDataUmount : 0x%08x\n", ret);
			}

			return hasSaved;
		}

		/*
		* Load the save that was previously stored under the provided name
		*
		* - Return the buffer and length of the binary data we loaded (the "game" needs to parse it later)
		* - If we cannot load a save-game, check if there's a backup of it and restore it and return the backupped save-data instead
		* - If no save-data or backup exists, return an invalid SaveFile (a defaulted one)
		*/
		SaveFile* Load(const char* name) {
		Start:
			int32_t ret = SCE_OK;

			SaveGame saveGame;
			SaveFile* file = new SaveFile();

			// Mount
			SceSaveDataMount2 mount2;
			setupSceSaveDataMount2(m_UserId,
				SCE_SAVE_DATA_MOUNT_MODE_RDONLY,
				&m_DirName,
				&mount2);
			SceSaveDataMountResult mountResult;
			memset(&mountResult, 0x00, sizeof(mountResult));
			ret = sceSaveDataMount2(&mount2, &mountResult);
			if (ret < SCE_OK)
			{
				std::cout << "Failed to mount save data" << std::endl;

				// Attempt to restore backup
				if (ret == SCE_SAVE_DATA_ERROR_BROKEN) {
					std::cout << "Save data is corrupted" << std::endl;
					std::cout << "Attempting to load backup" << std::endl;

					SceSaveDataCheckBackupData check;
					memset(&check, 0x00, sizeof(SceSaveDataCheckBackupData));
					check.userId = m_UserId;
					check.dirName = &m_DirName;
					ret = sceSaveDataCheckBackupData(&check);

					if (ret < SCE_OK) {
						std::cout << "No backup exists" << std::endl;
						return file;
					}
					else {
						SceSaveDataRestoreBackupData restore;
						memset(&restore, 0x00, sizeof(SceSaveDataRestoreBackupData));
						restore.userId = m_UserId;
						restore.dirName = &m_DirName;
						ret = sceSaveDataRestoreBackupData(&restore);
						std::cout << "Restoring backup" << std::endl;
						goto Start;
					}
				}
				else {
					return file;
				}
			}

			SceSaveDataMountPoint* mountPoint = &mountResult.mountPoint;

			// Read
			char path[64];
			snprintf(path, sizeof(path), "%s/%s", mountPoint->data, name);

			std::ifstream input(path, std::ios::binary);

			if (!input) {
				std::cout << "Could not open save file" << std::endl;
				goto End;
			}

			input.read((char*)&saveGame, sizeof(SaveGame));
			input.close();

			if (!input.good()) {
				std::cout << "Error occured at reading time" << std::endl;
				goto End;
			}

			file->data = reinterpret_cast<byte*>(&saveGame);
			file->length = sizeof(saveGame);

			// Unmount
		End:
			ret = sceSaveDataUmount(mountPoint);
			if (ret < SCE_OK)
			{
				EPRINT("sceSaveDataUmount : 0x%08x\n", ret);
			}

			return file;
		}


		// @note - lukas.vogl - You can alter the API and introduce new methods when you need them (Update, ...). 
		// You are free to choose if saving data is sync or async - both is supported on all platforms and it's up 
		// to you to decide what and why you see one more fitting than the other

	private:
		SceUserServiceUserId m_UserId;
		SceSaveDataDirName m_DirName;

		int clean(const SceUserServiceUserId userId, const char* dirNameTemplate, const size_t num)
		{
			int ret = SCE_OK;

			SceSaveDataDelete del;
			SceSaveDataDirName dirName;
			for (int32_t i = 0; i < num; i++)
			{
				memset(&dirName, 0x00, sizeof(dirName));
				snprintf(dirName.data, sizeof(dirName.data), dirNameTemplate, i);

				memset(&del, 0x00, sizeof(SceSaveDataDelete));
				del.userId = userId;
				del.titleId = NULL;
				del.dirName = &dirName;
				ret = sceSaveDataDelete(&del);
				printf("* sceSaveDataDelete : 0x%08x(%08x, %s)\n", ret, userId, dirName.data);
			}

			return SCE_OK;
		}

		void setupSceSaveDataMount2(const SceUserServiceUserId userId, const SceSaveDataMountMode mode,
			const SceSaveDataDirName* dirName, SceSaveDataMount2* mount)
		{
			memset(mount, 0x00, sizeof(SceSaveDataMount2));
			mount->userId = userId;
			mount->dirName = dirName;
			mount->blocks = SCE_SAVE_DATA_BLOCKS_MIN2;
			mount->mountMode = mode;
		}
	};
}