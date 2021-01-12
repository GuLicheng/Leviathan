#pragma

namespace leviathan::math
{

template <typename InIt, typename OutIt, typename T, typename F>
InIt split(InIt it, InIt end_it, OutIt out_it, T split_val, F bin_func) 
{
	while (it != end_it) 
    {
		auto slice_end(find(it, end_it, split_val));
		*out_it++ = bin_func(it, slice_end);
		if (slice_end == end_it) 
        {
			return end_it;
		}
		it = next(slice_end);
	}
	return it;
}

}  // namespace math