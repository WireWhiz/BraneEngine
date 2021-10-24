#include "chunk.h"

void operator>>(ChunkPool& pool, std::unique_ptr<Chunk>& dest)
{
	if (!pool._unused.empty())
	{
		dest = std::move(pool._unused[pool._unused.size() - 1]);
		pool._unused.resize(pool._unused.size() - 1);
	}
	else
		dest = std::make_unique<Chunk>();
}

void operator<<(ChunkPool& pool, std::unique_ptr<Chunk>& dest)
{
	pool._unused.push_back(std::move(dest));
}
