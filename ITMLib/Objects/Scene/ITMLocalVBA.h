// Copyright 2014-2017 Oxford University Innovation Limited and the authors of InfiniTAM

#pragma once

#include "../../../ORUtils/MemoryBlock.h"
#include "../../../ORUtils/MemoryBlockPersister.h"

namespace ITMLib
{
	/** \brief
	Stores the actual voxel content that is referred to by a
	ITMLib::ITMHashTable.
	*/
	template<class TVoxel>
	class ITMLocalVBA
	{
	private:
		ORUtils::MemoryBlock<TVoxel> *voxelBlocks;
		ORUtils::MemoryBlock<int> *allocationList;

		MemoryDeviceType memoryType;

	public:
		inline TVoxel *GetVoxelBlocks(void) { return voxelBlocks->GetData(memoryType); }
		inline const TVoxel *GetVoxelBlocks(void) const { return voxelBlocks->GetData(memoryType); }

		TVoxel *array;
		size_t used;
		size_t size;

		void initArray(size_t initialSize)
		{
			array = (TVoxel *)malloc(initialSize * sizeof(TVoxel));
			used = 0;
			size = initialSize;
		}

		void insertArray(TVoxel element)
		{
			// a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
			// Therefore a->used can go up to a->size
			if (used == size)
			{
				size *= 2;
				array = (TVoxel *)realloc(array, size * sizeof(TVoxel));
			}
			array[used++] = element;
		}

		void freeArray()
		{
			// free(array);
				// array = NULL;
			// used = size = 0;
		}

		// TVoxel *GetOccupiedVoxelBlocks(void) {
		// 	int i;
		// 	freeArray();
		// 	initArray(1000);
		// 	TVoxel* t = GetVoxelBlocks();
		// 	for (i = 0; i < allocatedSize; i++ ) {
		// 		TVoxel vi = *(t + i);
		// 		if (vi.w_depth > 0) {
		// 			insertArray(vi);
		// 		}
		// 	}
		// 	return array;
		// }

		TVoxel *GetOccupiedVoxelBlocks2(int noTotalEntries, ITMLib::ITMVoxelBlockHash::IndexData * index) {
			freeArray();
			initArray(8000);

			// voxelIndex.GetEntries();
			for (int entryId = 0; entryId < noTotalEntries; entryId++)
			{
				const ITMHashEntry &currentHashEntry = index[entryId];

				if (currentHashEntry.ptr < 0)
					continue;

				Vector3i globalPos = currentHashEntry.pos.toInt() * SDF_BLOCK_SIZE;
				int vmIndex;

				for (int z = 0; z < SDF_BLOCK_SIZE; z++)
					for (int y = 0; y < SDF_BLOCK_SIZE; y++)
						for (int x = 0; x < SDF_BLOCK_SIZE; x++)
						{
							Vector3i pos = globalPos + Vector3i(x, y, z);
							TVoxel vi = readVoxel(GetVoxelBlocks(), index, pos, vmIndex);
							if (vi.w_depth > 0) {
								insertArray(vi);
							}
						}
			}
			return array;
		}

		int *GetAllocationList(void) { return allocationList->GetData(memoryType); }

#ifdef COMPILE_WITH_METAL
		const void* GetVoxelBlocks_MB() const { return voxelBlocks->GetMetalBuffer(); }
		const void* GetAllocationList_MB(void) const { return allocationList->GetMetalBuffer(); }
#endif
		int lastFreeBlockId;

		int allocatedSize;

		void SaveToDirectory(const std::string &outputDirectory) const
		{
			std::string VBFileName = outputDirectory + "voxel.dat";
			std::string ALFileName = outputDirectory + "alloc.dat";
			std::string AllocSizeFileName = outputDirectory + "vba.txt";

			ORUtils::MemoryBlockPersister::SaveMemoryBlock(VBFileName, *voxelBlocks, memoryType);
			ORUtils::MemoryBlockPersister::SaveMemoryBlock(ALFileName, *allocationList, memoryType);

			std::ofstream ofs(AllocSizeFileName.c_str());
			if (!ofs) throw std::runtime_error("Could not open " + AllocSizeFileName + " for writing");

			ofs << lastFreeBlockId << ' ' << allocatedSize;
		}

		void LoadFromDirectory(const std::string &inputDirectory)
		{
			std::string VBFileName = inputDirectory + "voxel.dat";
			std::string ALFileName = inputDirectory + "alloc.dat";
			std::string AllocSizeFileName = inputDirectory + "vba.txt";

			ORUtils::MemoryBlockPersister::LoadMemoryBlock(VBFileName, *voxelBlocks, memoryType);
			ORUtils::MemoryBlockPersister::LoadMemoryBlock(ALFileName, *allocationList, memoryType);

			std::ifstream ifs(AllocSizeFileName.c_str());
			if (!ifs) throw std::runtime_error("Could not open " + AllocSizeFileName + " for reading");

			ifs >> lastFreeBlockId >> allocatedSize;
		}

		ITMLocalVBA(MemoryDeviceType memoryType, int noBlocks, int blockSize)
		{
			this->memoryType = memoryType;

			allocatedSize = noBlocks * blockSize;

			voxelBlocks = new ORUtils::MemoryBlock<TVoxel>(allocatedSize, memoryType);
			allocationList = new ORUtils::MemoryBlock<int>(noBlocks, memoryType);
		}

		~ITMLocalVBA(void)
		{
			delete voxelBlocks;
			delete allocationList;
		}

		// Suppress the default copy constructor and assignment operator
		ITMLocalVBA(const ITMLocalVBA&) = delete;
		ITMLocalVBA& operator=(const ITMLocalVBA&) = delete;
	};
}
