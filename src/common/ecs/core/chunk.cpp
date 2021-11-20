#include "chunk.h"

void operator>>(ChunkPool& pool, std::unique_ptr<Chunk>& dest)
{
	std::scoped_lock lock(pool._m);
	if (!pool._unused.empty())
	{
		dest = std::move(pool._unused[pool._unused.size() - 1]);
		pool._unused.erase(pool._unused.end() - 1);
	}
	else
		dest = std::make_unique<Chunk>();
}

void operator<<(ChunkPool& pool, std::unique_ptr<Chunk>& src)
{
	std::scoped_lock lock(pool._m);
	src->setArchetype(nullptr);
	pool._unused.emplace_back(std::move(src));
	//pool._unused[pool._unused.size() - 1] = std::move(src);
}
