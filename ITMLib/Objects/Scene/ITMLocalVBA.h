// Copyright 2014-2017 Oxford University Innovation Limited and the authors of InfiniTAM

#pragma once

#include "../../../ORUtils/MemoryBlock.h"
#include "../../../ORUtils/MemoryBlockPersister.h"
#include "../../../ORUtils/DynamicArray.h"

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

		DynamicArray<Vector3i> positions;
		DynamicArray<TVoxel> voxels;
		
	 	void refreshOccupiedVoxelBlocks(int noTotalEntries, ITMLib::ITMVoxelBlockHash::IndexData *index)
		{
			TVoxel *voxelBlocks = GetVoxelBlocks();
			if (memoryType == MEMORYDEVICE_CUDA)
			{
				TVoxel *localVBA_cpu = (TVoxel *)malloc(sizeof(TVoxel) * allocatedSize);
				cudaMemcpy(localVBA_cpu, voxelBlocks, sizeof(TVoxel) * allocatedSize, cudaMemcpyDeviceToHost);
				voxelBlocks = localVBA_cpu;

				// TODO need to check memorytypeof Index, not LocalVBA!
				ITMLib::ITMVoxelBlockHash::IndexData *index_cpu = (ITMHashEntry *)malloc(sizeof(ITMHashEntry) * (0x100000 + 0x20000));
				cudaMemcpy(index_cpu, index, sizeof(ITMHashEntry) * (0x100000 + 0x20000), cudaMemcpyDeviceToHost);
				index = index_cpu;
			}

			positions.initArray(8000);
			voxels.initArray(8000);

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
							TVoxel vi = readVoxel(voxelBlocks, index, pos, vmIndex);
							if (vi.w_depth > 0 
							// && (vi.clr.x > 0 || vi.clr.y > 0 || vi.clr.z > 0) 
							// && x%8==y%8==z%8==0
							)
							{
								float ftsdf = TVoxel::fTSDF(vi.sdf);
								if (abs(ftsdf) >= 0.95) {
									// vi.ftsdf = ftsdf;
									positions.insertArray(pos);
									voxels.insertArray(vi);
								}
							}
						}
			}

			if (memoryType == MEMORYDEVICE_CUDA)
				{
					free(voxelBlocks);
					free(index);
				}
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
