/// Jack
#include "ref_counted.h"

#include <assert.h>
#include "atomic.h"

namespace common
{
using namespace ipcs_common;

bool IRefCounted::IsUnique() const
{
    return AtomicGet(&m_ref_count) == 1;
}

int IRefCounted::GetRefCount() const
{
    return AtomicGet(&m_ref_count);
}

IRefCounted::IRefCounted() : m_ref_count(0)
{
}

IRefCounted::IRefCounted(int base_ref_count) : m_ref_count(base_ref_count)
{
}

IRefCounted::~IRefCounted()
{
}

int IRefCounted::AddRef() const
{
    return AtomicIncrement(&m_ref_count);
}

bool IRefCounted::Release() const
{
    int count = AtomicDecrement(&m_ref_count);
    assert(count >= 0);
    return count == 0;
}

}  // namespace common
