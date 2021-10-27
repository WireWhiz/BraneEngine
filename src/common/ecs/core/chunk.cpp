#include "chunk.h"

void operator>>(ChunkPool& pool, std::unique_ptr<Chunk>& dest)
{
	std::scoped_lock lock(pool._m);
	if (pool._unused.size() > 0)
	{
		dest = std::move(pool._unused[pool._unused.size() - 1]);
		pool._unused.erase(pool._unused.end() - 1);
	}
	else
		dest = std::make_unique<Chunk>();
}

void operator<<(ChunkPool& pool, std::unique_ptr<Chunk>& dest)
{
	std::scoped_lock lock(pool._m);
	pool._unused.emplace_back();
	pool._unused[pool._unused.size() - 1] = std::move(dest);
}

ChunkPool::~ChunkPool()
{
	//printf("chunk pool destroyed\n");
}
