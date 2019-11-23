#include "compStream.h"

#include <assert.h>

ComparisonStream::ComparisonStream(Common::SeekableReadStream* test, Common::SeekableReadStream* verification) :
	testStream(test), verificationStream(verification)
{
}

ComparisonStream::~ComparisonStream()
{
	delete testStream;
	delete verificationStream;
}

bool ComparisonStream::err() const
{
	auto test = testStream->err();
	auto ver = verificationStream->err();
	assert(test == ver);
	return ver;
}

void ComparisonStream::clearErr()
{
	testStream->clearErr();
	verificationStream->clearErr();
	checkSync();
}

bool ComparisonStream::eos() const {
	auto test = testStream->eos();
	auto ver = verificationStream->eos();
	assert(test == ver);
	return ver;
}

uint32 ComparisonStream::read(void* dataPtr, uint32 dataSize)
{
	auto test = testStream->read(dataPtr, dataSize);
	auto ver = verificationStream->read(dataPtr, dataSize);
	assert(test == ver);
	checkSync();
	return ver;
}

int32 ComparisonStream::pos() const
{
	auto test = testStream->pos();
	auto ver = verificationStream->pos();
	assert(test == ver);
	return ver;
}

int32 ComparisonStream::size() const
{
	auto test = testStream->size();
	auto ver = verificationStream->size();
	assert(test == ver);
	return ver;
}

bool ComparisonStream::seek(int32 offset, int whence)
{
	auto test = testStream->seek(offset, whence);
	auto ver = verificationStream->seek(offset, whence);
	assert(test == ver);
	checkSync();
	return ver;
}

void ComparisonStream::checkSync() const
{
	{
		auto test = testStream->size();
		auto ver = verificationStream->size();
		assert(test == ver);
	}
	{
		auto test = testStream->eos();
		auto ver = verificationStream->eos();
		assert(test == ver);
	}
	{
		auto test = testStream->pos();
		auto ver = verificationStream->pos();
		assert(test == ver);
	}
	{
		auto test = testStream->err();
		auto ver = verificationStream->err();
		assert(test == ver);
	}
}
