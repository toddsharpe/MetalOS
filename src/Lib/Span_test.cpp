#include "Lib/Span.h"

void Span_test()
{
	//A whole array
	uint8_t data[4] = { 1, 2, 3, 4 };
	Span<uint8_t> all(data);
	Assert(all.Size == 4);
	Assert(all[3] == 4);

	//Writes go through to the array
	all[0] = 9;
	Assert(data[0] == 9);

	//Span<T> converts to Span<const T>
	const Span<const uint8_t> view = all;
	Assert(view.Data == data && view.Size == 4);

	//Equality compares contents, not addresses
	static constexpr uint8_t same[4] = { 9, 2, 3, 4 };
	static constexpr uint8_t other[4] = { 9, 2, 3, 5 };
	Assert(view == Span<const uint8_t>(same));
	Assert(view != Span<const uint8_t>(other));
	Assert(view != view.Slice(0, 3));

	//Slices, including empty ones at either end
	const Span<const uint8_t> middle = view.Slice(1, 2);
	Assert(middle.Size == 2 && middle[0] == 2 && middle[1] == 3);
	Assert(view.Slice(0, 0).Size == 0);
	Assert(view.Slice(4, 0).Size == 0);

	//Empty spans are equal and false
	const Span<const uint8_t> empty;
	Assert(!empty);
	Assert(empty == view.Slice(2, 0));
	Assert(all);

	//Range-for walks every element
	size_t sum = 0;
	for (const uint8_t b : view)
		sum += b;
	Assert(sum == 18);

	//Bytes of an object
	static constexpr uint32_t value = 0x04030201;
	const Span<const uint8_t> bytes = Bytes(&value, sizeof(value));
	Assert(bytes.Size == 4 && bytes[0] == 1 && bytes[3] == 4);
}
