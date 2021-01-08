#ifndef __NONE_HPP__
#define __NONE_HPP__

namespace leviathan
{
    enum class NoneType { null = 1, None = 2 };
    const NoneType null = null;
    const NoneType None = None;

    // we can use nullopt to replace these

} // namespace leviathan

#endif