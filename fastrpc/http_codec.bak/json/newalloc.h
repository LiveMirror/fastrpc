class Elem
{
        public:
                Elem(){cout << "Elem()" << endl;}
                Elem(const Elem&){cout << "Elem(const Elem&)" << endl;}
                ~Elem(){cout << "~Elem()" << endl;}
                int m_i;
                float m_f;
};
template<typename T>
class MyAlloc
{
        public:
                typedef T* pointer;
                typedef const T* const_pointer;
                typedef T& reference;
                typedef const T& const_reference;
                template<typename U>
                struct rebind
                {
                        typedef MyAlloc<U> other;
                };
                pointer allocate(size_t num)
                {
                        cout << " allocate: " << num << endl;
                        return (pointer)::operator new(num*sizeof(T));
                }
                void deallocate(pointer p, size_t num)
                {
                        cout << "deallocate: " << typeid(p).name() << " " << num << endl;
                        ::operator delete ((void*)p);
                }
                MyAlloc()throw(){cout << "myalloc()" << endl;}
                template<typename U>
                MyAlloc(const MyAlloc<U>&) throw(){cout << "myalloc(const&" << typeid(U).name() << ")" << endl;}
};
template<typename T1, typename T2>
bool operator == (const MyAlloc<T1>& , const MyAlloc<T2>&)
{
        return true;
}