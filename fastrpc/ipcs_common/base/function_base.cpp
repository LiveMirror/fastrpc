/// Jack
/// description: function_base implementation

#include "function_base.h"
#include <algorithm>

namespace common
{

namespace internal
{

FunctionBase::FunctionBase(
    InvokeFuncStorage polymorphic_invoke,
    scoped_refptr<InvokerStorageBase>* invoker_storage
):
    m_polymorphic_invoke(polymorphic_invoke)
{
    if (invoker_storage)
    {
        m_invoker_storage.swap(*invoker_storage);
    }
}

FunctionBase::~FunctionBase()
{
}

bool FunctionBase::IsNull() const
{
    return m_invoker_storage.get() == NULL;
}

void FunctionBase::Clear()
{
    m_invoker_storage = NULL;
    m_polymorphic_invoke = NULL;
}

bool FunctionBase::IsEqualTo(const FunctionBase& other) const
{
    return m_invoker_storage.get() == other.m_invoker_storage.get() &&
           m_polymorphic_invoke == other.m_polymorphic_invoke;
}

void FunctionBase::DoSwap(FunctionBase* other)
{
    using std::swap;
    swap(m_invoker_storage, other->m_invoker_storage);
    swap(m_polymorphic_invoke, other->m_polymorphic_invoke);
}

}  // namespace internal

}  // namespace common
