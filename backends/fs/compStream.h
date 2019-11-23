#pragma once

#include "backends/fs/abstract-fs.h"
#include "common/stream.h"

class ComparisonStream : public Common::SeekableReadStream {
public:
	ComparisonStream(Common::SeekableReadStream* test, Common::SeekableReadStream* verification);
	~ComparisonStream();

	virtual bool err() const override;
	virtual void clearErr() override;

	// Inherited via ReadStream
	virtual bool eos() const override;
	virtual uint32 read(void* dataPtr, uint32 dataSize) override;

	// Inherited via SeekableReadStream
	virtual int32 pos() const override;
	virtual int32 size() const override;
	virtual bool seek(int32 offset, int whence = SEEK_SET) override;
private:
	Common::SeekableReadStream *testStream;
	Common::SeekableReadStream *verificationStream;

	void checkSync() const;
};